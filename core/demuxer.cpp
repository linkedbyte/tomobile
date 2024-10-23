#include "demuxer.h"
#include "binary.h"
extern "C"{
#include <stdint.h>
#include <unistd.h>
#include <libavformat/version.h>
}
#include <QDebug>

// In ffmpeg/doc/APIchanges:
// 2018-02-06 - 0694d87024 - lavf 58.9.100 - avformat.h
//   Deprecate use of av_register_input_format(), av_register_output_format(),
//   av_register_all(), av_iformat_next(), av_oformat_next().
//   Add av_DemuxerThread_iterate(), and av_muxer_iterate().
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 9, 100)
# define MDC_LAVF_HAS_NEW_MUXER_ITERATOR_API
#else
# define MDC_LAVF_REQUIRES_REGISTER_ALL
#endif

// Not documented in ffmpeg/doc/APIchanges, but AV_CODEC_ID_AV1 has been added
// by FFmpeg commit d42809f9835a4e9e5c7c63210abb09ad0ef19cfb (included in tag
// n3.3).
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 89, 100)
# define MDC_LAVC_HAS_AV1
#endif

// In ffmpeg/doc/APIchanges:
// 2018-01-28 - ea3672b7d6 - lavf 58.7.100 - avformat.h
//   Deprecate AVFormatContext filename field which had limited length, use the
//   new dynamically allocated url field instead.
//
// 2018-01-28 - ea3672b7d6 - lavf 58.7.100 - avformat.h
//   Add url field to AVFormatContext and add ff_format_set_url helper function.
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 7, 100)
# define MDC_LAVF_HAS_AVFORMATCONTEXT_URL
#endif

// Not documented in ffmpeg/doc/APIchanges, but the channel_layout API
// has been replaced by chlayout in FFmpeg commit
// f423497b455da06c1337846902c770028760e094.
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 23, 100)
# define MDC_LAVU_HAS_CHLAYOUT
#endif

#define PACKET_HEADER_SIZE 12
#define PACKET_FLAG_CONFIG    (UINT64_C(1) << 63)
#define PACKET_FLAG_KEY_FRAME (UINT64_C(1) << 62)
#define PACKET_PTS_MASK (PACKET_FLAG_KEY_FRAME - 1)

#define CODEC_ID_H264 UINT32_C(0x68323634) // "h264" in ASCII
#define CODEC_ID_H265 UINT32_C(0x68323635) // "h265" in ASCII
#define CODEC_ID_AV1 UINT32_C(0x00617631) // "av1" in ASCII
#define CODEC_ID_OPUS UINT32_C(0x6f707573) // "opus" in ASCII
#define CODEC_ID_AAC UINT32_C(0x00616163) // "aac in ASCII"
#define CODEC_ID_RAW UINT32_C(0x00726177) // "raw" in ASCII

#define LOG_OOM qDebug()<<"OOM:"<<__FILE__<<":"<<__LINE__<<__func__;
enum AVCodecID demuxer_to_avcodec_id(uint32_t codec_id) {
    switch (codec_id) {
    case CODEC_ID_H264:
        return AV_CODEC_ID_H264;
    case CODEC_ID_H265:
        return AV_CODEC_ID_HEVC;
    case CODEC_ID_AV1:
#ifdef MDC_LAVC_HAS_AV1
        return AV_CODEC_ID_AV1;
#else
        qDebug()<<"AV1 not supported by this FFmpeg version";
        return AV_CODEC_ID_NONE;
#endif
    case CODEC_ID_OPUS:
        return AV_CODEC_ID_OPUS;
    case CODEC_ID_AAC:
        return AV_CODEC_ID_AAC;
    case CODEC_ID_RAW:
        return AV_CODEC_ID_PCM_S16LE;
    default:
        qDebug()<<"Unknown codec id :"<< codec_id;
        return AV_CODEC_ID_NONE;
    }
}
bool DemuxerThread::recvPacket(AVPacket *packet)
{
    // The video and audio streams contain a sequence of raw packets (as
    // provided by MediaCodec), each prefixed with a "meta" header.
    //
    // The "meta" header length is 12 bytes:
    // [. . . . . . . .|. . . .]. . . . . . . . . . . . . . . ...
    //  <-------------> <-----> <-----------------------------...
    //        PTS        packet        raw packet
    //                    size
    //
    // It is followed by <packet_size> bytes containing the packet/frame.
    //
    // The most significant bits of the PTS are used for packet flags:
    //
    //  byte 7   byte 6   byte 5   byte 4   byte 3   byte 2   byte 1   byte 0
    // CK...... ........ ........ ........ ........ ........ ........ ........
    // ^^<------------------------------------------------------------------->
    // ||                                PTS
    // | `- key frame
    //  `-- config packet

    uint8_t header[PACKET_HEADER_SIZE];

    ssize_t r = this->socket->recv((char*)header,PACKET_HEADER_SIZE);
    if (r < PACKET_HEADER_SIZE) {
        return false;
    }

    uint64_t pts_flags = read64be(header);
    uint32_t len = read32be(&header[8]);
    assert(len);

    if (av_new_packet(packet, len)) {
        LOG_OOM;
        return false;
    }
    r = this->socket->recv((char*)packet->data,len);

    if (r < 0 || ((uint32_t) r) < len) {
        av_packet_unref(packet);
        return false;
    }

    if (pts_flags & PACKET_FLAG_CONFIG) {
        packet->pts = AV_NOPTS_VALUE;
    } else {
        packet->pts = pts_flags & PACKET_PTS_MASK;
    }

    if (pts_flags & PACKET_FLAG_KEY_FRAME) {
        packet->flags |= AV_PKT_FLAG_KEY;
    }

    packet->dts = packet->pts;
    return true;
}

bool DemuxerThread::recvVideoSize(uint32_t *width,uint32_t *height)
{
    uint8_t data[8];
    ssize_t r = this->socket->recv((char*)data,8);//net_recv_all(DemuxerThread->socket, data, 8);
    if (r < 8) {
        return false;
    }

    *width = read32be(data);
    *height = read32be(data + 4);
    return true;
}

DemuxerThread::DemuxerThread(const std::string &name,
                 TcpSocket *socket,Decoder *decoder,QObject *parent)
    : QThread{parent}
{
    this->name = name;
    this->socket = socket;
    this->socket->moveToThread(this);
    this->decoder = decoder;
}

DemuxerThread::~DemuxerThread()
{

}

void DemuxerThread::run()
{
    qInfo()<<"DemuxerThread '"<<this->name.c_str()<<"': starting thread id is :"<<QThread::currentThreadId();
    enum DemuxerStatus status = DemuxerStatus::DEMUXER_STATUS_ERROR;

    uint32_t raw_codec_id;
    uint8_t data[4];

    enum AVCodecID codec_id = AV_CODEC_ID_NONE;
    const AVCodec *codec = NULL;
    AVCodecContext *codec_ctx = NULL;
    bool must_merge_config_packet = false;
    struct PacketMerger merger;
    AVPacket *packet = NULL;

    int ret = this->socket->recv((char*)data, 4);
    if (ret < 4) {
        goto end;
    }

    raw_codec_id = read32be(data);
    if(raw_codec_id == 0) {
        qDebug()<<"DemuxerThread '"<<this->name.c_str()<<"': stream explicitly disabled by the device";
        status = DemuxerStatus::DEMUXER_STATUS_DISABLED;
        goto end;
    }
    if(raw_codec_id == 1) {
        qDebug()<<"DemuxerThread '"<<this->name.c_str()<<"': stream configuration error on the device";
        goto end;
    }
    codec_id = demuxer_to_avcodec_id(raw_codec_id);
    if (codec_id == AV_CODEC_ID_NONE) {
        qDebug()<<"DemuxerThread '"<<this->name.c_str()<<"': stream disabled due to unsupported codec";
        goto end;
    }

    codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        qDebug()<<"DemuxerThread '"<<this->name.c_str()<<"': stream disabled due to missing decoder";
        goto end;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        qDebug()<<"OOM:"<<__FILE__<<":"<<__LINE__<<__func__;
        goto end;
    }

    codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
    codec_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;

    if (codec->type == AVMEDIA_TYPE_VIDEO) {
        uint32_t width;
        uint32_t height;
        bool ok = recvVideoSize(&width, &height);
        if(!ok)
        {
            qDebug()<<"recv video size failed!";
            avcodec_free_context(&codec_ctx);
            goto end;
        }
        codec_ctx->width = width;
        codec_ctx->height = height;
        codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    }
    else
    {
        // Hardcoded audio properties
#ifdef MDC_LAVU_HAS_CHLAYOUT
        codec_ctx->ch_layout = (AVChannelLayout) AV_CHANNEL_LAYOUT_STEREO;
#else
        codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
        codec_ctx->channels = 2;
#endif
        codec_ctx->sample_rate = 48000;
    }

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        qDebug()<<"DemuxerThread '"<<this->name.c_str()<<"': could not open codec";
        avcodec_free_context(&codec_ctx);
    }

    if (!this->decoder->open(codec_ctx))
    {
       avcodec_free_context(&codec_ctx);
    }

    // Config packets must be merged with the next non-config packet only for
    // video streams
    must_merge_config_packet = codec->type == AVMEDIA_TYPE_VIDEO;

    if (must_merge_config_packet) {
       merger.config = NULL;
    }

    packet = av_packet_alloc();
    if (!packet) {
        qDebug()<<"OOM:"<<__FILE__<<":"<<__LINE__<<__func__;

        goto end;
    }

    for (;;) {
        if(isInterruptionRequested())
            break;
        bool ok = this->recvPacket(packet);
        if (!ok) {
            // end of stream
            status = DemuxerStatus::DEMUXER_STATUS_EOS;
            break;
        }

        if (must_merge_config_packet) {
            // Prepend any config packet to the next media packet
            ok = packetMergerMerge(&merger, packet);
            if (!ok) {
                av_packet_unref(packet);
                qDebug()<<"Prepend any config packet to the next media packet";
                break;
            }
        }

        ok = this->decoder->push(packet);
        av_packet_unref(packet);
        if (!ok) {
            // The sink already logged its concrete error
            qDebug()<<"The sink already logged its concrete error";
            break;
        }
    }

    qDebug()<<"DemuxerThread '"<<this->name.c_str()<<"': end of frames";

    if (must_merge_config_packet) {
        free(merger.config);
    }

    av_packet_free(&packet);

finally_close_sinks:
    this->decoder->close();
finally_free_context:
    avcodec_free_context(&codec_ctx);
end:
    emit onEnded(status);
}

bool DemuxerThread::packetMergerMerge(struct PacketMerger *merger, AVPacket *packet)
{
    bool is_config = packet->pts == AV_NOPTS_VALUE;

    if(is_config)
    {
        free(merger->config);

        merger->config = (uint8_t*)malloc(packet->size);
        if (!merger->config) {
            qDebug()<<"OOM:"<<__FILE__<<":"<<__LINE__<<__func__;
            return false;
        }

        memcpy(merger->config, packet->data, packet->size);
        merger->configSize = packet->size;
    }
    else if(merger->config)
    {
        size_t config_size = merger->configSize;
        size_t media_size = packet->size;

        if (av_grow_packet(packet, config_size)) {
            qDebug()<<"OOM:"<<__FILE__<<":"<<__LINE__<<__func__;
            return false;
        }

        memmove(packet->data + config_size, packet->data, media_size);
        memcpy(packet->data, merger->config, config_size);

        free(merger->config);
        merger->config = NULL;
        // merger->size is meaningless when merger->config is NULL
    }
    else
    {
        //
    }

    return true;
}

