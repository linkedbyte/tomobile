#include "utils.h"
#include <string>
#include <string.h>
#include <fstream>
#include <QDebug>
Utils::Utils()
{

}
const char *Utils::getFormatName(enum RecordFormat format)
{
    switch (format) {
    case RECORD_FORMAT_MP4:
    case RECORD_FORMAT_M4A:
    case RECORD_FORMAT_AAC:
        return "mp4";
    case RECORD_FORMAT_MKV:
    case RECORD_FORMAT_MKA:
        return "matroska";
    case RECORD_FORMAT_OPUS:
        return "opus";
    default:
        return NULL;
    }
}
bool Utils::strListContains(const char *list, char sep, const char *s) {
    char *p;
    do {
        p = strchr(list, sep);

        size_t token_len = p ? (size_t) (p - list) : strlen(list);
        if (!strncmp(list, s, token_len)) {
            return true;
        }

        if (p) {
            list = p + 1;
        }
    } while (p);
    return false;
}
bool Utils::parseVideoCodec(const char *optarg, enum VideoAudioCodec *codec) {
    if (!strcmp(optarg, "h264")) {
        *codec = CODEC_H264;
        return true;
    }
    if (!strcmp(optarg, "h265")) {
        *codec = CODEC_H265;
        return true;
    }
    if (!strcmp(optarg, "av1")) {
        *codec = CODEC_AV1;
        return true;
    }
    return false;
}
std::string Utils::getVideoAudioCodec(enum VideoAudioCodec codec) {
    std::string ret = "unknow";
    switch(codec)
    {
    case CODEC_H264:
        ret = "h264";
        break;
    case CODEC_H265:
        ret = "h265";
        break;
    case CODEC_AV1:
        ret = "av1";
        break;
    case CODEC_OPUS:
        ret = "opus";
        break;
    case CODEC_AAC:
        ret = "acc";
        break;
    case CODEC_RAW:
        ret = "raw";
        break;
    default:
        break;
    }
    return ret;
}
std::string Utils::getVideoSource(enum VideoSource source)
{
    std::string ret = "unknow";
    switch(source)
    {
    case VIDEO_SOURCE_DISPLAY:
        ret = "display";
        break;
    case VIDEO_SOURCE_CAMERA:
        ret = "camera";
        break;
    default:
        break;
    }
    return ret;
}
std::string Utils::getAudioSource(enum AudioSource source)
{
    std::string ret = "unknow";
    switch(source)
    {
    case AUDIO_SOURCE_MIC:
        ret = "mic";
        break;
    case AUDIO_SOURCE_OUTPUT:
        ret = "output";
        break;
    default:
        break;
    }
    return ret;
}

static int frameToImage(AVFrame* frame, enum AVCodecID codecID, uint8_t* outbuf, size_t outbufSize)
{
    int ret = 0;
    AVPacket pkt;
    AVCodecContext* ctx = NULL;
    AVFrame* rgbFrame = NULL;
    uint8_t* buffer = NULL;
    struct SwsContext* swsContext = NULL;
    av_init_packet(&pkt);
    const AVCodec* codec = avcodec_find_encoder(codecID);
    if (!codec)
    {
        printf("avcodec_send_frame error %d", codecID);
        goto end;
    }
    if (!codec->pix_fmts)
    {
        printf("unsupport pix format with codec %s", codec->name);
        goto end;
    }
    ctx = avcodec_alloc_context3(codec);
    ctx->bit_rate = 3000000;
    ctx->width = frame->width;
    ctx->height = frame->height;
    ctx->time_base.num = 1;
    ctx->time_base.den = 25;
    ctx->gop_size = 10;
    ctx->max_b_frames = 0;
    ctx->thread_count = 1;
    ctx->pix_fmt = *codec->pix_fmts;
    ret = avcodec_open2(ctx, codec, NULL);
    if (ret < 0)
    {
        printf("avcodec_open2 error %d", ret);
        goto end;
    }
    if (frame->format != ctx->pix_fmt)
    {
        rgbFrame = av_frame_alloc();
        if (rgbFrame == NULL)
        {
            printf("av_frame_alloc  fail");
            goto end;
        }
        swsContext = sws_getContext(frame->width, frame->height, (enum AVPixelFormat)frame->format, frame->width, frame->height, ctx->pix_fmt, 1, NULL, NULL, NULL);
        if (!swsContext)
        {
            printf("sws_getContext  fail");
            goto end;
        }
        int bufferSize = av_image_get_buffer_size(ctx->pix_fmt, frame->width, frame->height, 1) * 2;
        buffer = (unsigned char*)av_malloc(bufferSize);
        if (buffer == NULL)
        {
            printf("buffer alloc fail:%d", bufferSize);
            goto end;
        }
        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, ctx->pix_fmt, frame->width, frame->height, 1);
        if ((ret = sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height, rgbFrame->data, rgbFrame->linesize)) < 0)
        {
            printf("sws_scale error %d", ret);
        }
        rgbFrame->format = ctx->pix_fmt;
        rgbFrame->width = ctx->width;
        rgbFrame->height = ctx->height;
        ret = avcodec_send_frame(ctx, rgbFrame);
    }
    else
    {
        ret = avcodec_send_frame(ctx, frame);
    }
    if (ret < 0)
    {
        printf("avcodec_send_frame error %d", ret);
        goto end;
    }
    ret = avcodec_receive_packet(ctx, &pkt);
    if (ret < 0)
    {
        printf("avcodec_receive_packet error %d", ret);
        goto end;
    }
    if (pkt.size > 0 && pkt.size <= outbufSize)
        memcpy(outbuf, pkt.data, pkt.size);
    ret = pkt.size;
end:
    if (swsContext)
    {
        sws_freeContext(swsContext);
    }
    if (rgbFrame)
    {
        av_frame_unref(rgbFrame);
        av_frame_free(&rgbFrame);
    }
    if (buffer)
    {
        av_free(buffer);
    }
    av_packet_unref(&pkt);
    if (ctx)
    {
        avcodec_close(ctx);
        avcodec_free_context(&ctx);
    }
    return ret;
}


bool Utils::screenShot(const std::string &fileName,AVFrame *frame)
{
    int bufSize = av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame->width, frame->height, 64);
    uint8_t* buf = (uint8_t*)av_malloc(bufSize);
    int picSize = frameToImage(frame, AV_CODEC_ID_MJPEG, buf, bufSize);
    std::ofstream outfile(fileName, std::ios::binary);

    if (!outfile) {
        qDebug()<<"open file failed!";
        return false;
    }
    outfile.write(reinterpret_cast<char*>(&buf[0]),bufSize);
    outfile.close();
    av_free(buf);
    return true;
 }
bool Utils::parseAudioCodec(const char *optarg, enum VideoAudioCodec *codec) {
    if (!strcmp(optarg, "opus")) {
        *codec = CODEC_OPUS;
        return true;
    }
    if (!strcmp(optarg, "aac")) {
        *codec = CODEC_AAC;
        return true;
    }
    if (!strcmp(optarg, "raw")) {
        *codec = CODEC_RAW;
        return true;
    }
    return false;
}

bool Utils::parseVideoSource(const char *optarg, enum VideoSource *source) {
    if (!strcmp(optarg, "display")) {
        *source = VIDEO_SOURCE_DISPLAY;
        return true;
    }

    if (!strcmp(optarg, "camera")) {
        *source = VIDEO_SOURCE_CAMERA;
        return true;
    }
    return false;
}
bool Utils::parseAudioSource(const char *optarg, enum AudioSource *source) {
    if (!strcmp(optarg, "mic")) {
        *source = AUDIO_SOURCE_MIC;
        return true;
    }

    if (!strcmp(optarg, "output")) {
        *source = AUDIO_SOURCE_OUTPUT;
        return true;
    }
    return false;
}
