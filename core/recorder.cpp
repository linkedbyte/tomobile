#include "recorder.h"
#include "common.h"
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}
#include <algorithm>
#include <QDebug>
#include "utils.h"

static const AVRational TIME_BASE = {1, 1000000}; // timestamps in us

RecoderVideoSink::RecoderVideoSink(RecorderThread *handler)
{
    this->recorder = handler;
    this->recorder->videoInit = false;

}

RecoderVideoSink::~RecoderVideoSink()
{

}

bool RecoderVideoSink::open(AVCodecContext *ctx)
{
    this->recorder->mutex.lock();
    AVStream *stream = avformat_new_stream(this->recorder->ctx,ctx->codec);
    if (!stream) {
        this->recorder->mutex.unlock();
        return false;
    }

    int r = avcodec_parameters_from_context(stream->codecpar, ctx);
    if (r < 0) {
        this->recorder->mutex.unlock();
        return false;
    }

    this->recorder->videoStream.index = stream->index;
    this->recorder->cond.notify_all();
    this->recorder->videoInit = true;
    this->recorder->mutex.unlock();

    return true;
}

void RecoderVideoSink::close()
{
    this->recorder->mutex.lock();
    this->recorder->stopped = true;
    this->recorder->cond.notify_all();
    this->recorder->mutex.unlock();
}
static AVPacket *recorder_packet_ref(const AVPacket *packet)
{
    AVPacket *p = av_packet_alloc();
    if (!p) {
        //LOG_OOM();
        return NULL;
    }

    if (av_packet_ref(p, packet)) {
        av_packet_free(&p);
        return NULL;
    }

    return p;
}
bool RecoderVideoSink::push(AVPacket *packet)
{
    if(!this->recorder->videoInit)
        return false;
    this->recorder->mutex.lock();

    if (this->recorder->stopped) {
        // reject any new packet
        this->recorder->mutex.unlock();
        return false;
    }

    AVPacket *rec = recorder_packet_ref(packet);
    if (!rec) {
        //LOG_OOM();
        this->recorder->mutex.unlock();
        return false;
    }

    rec->stream_index = this->recorder->videoStream.index;

    this->recorder->videoQueue.push(rec);
    this->recorder->cond.notify_all();
    this->recorder->mutex.unlock();
    return true;
}

RecoderAudioSink::RecoderAudioSink(RecorderThread *handler)
{
    this->recorder = handler;
    this->recorder->audioInit = true;
}

RecoderAudioSink::~RecoderAudioSink()
{

}

bool RecoderAudioSink::open(AVCodecContext *ctx)
{
    this->recorder->mutex.lock();

    AVStream *stream = avformat_new_stream(this->recorder->ctx, ctx->codec);
    if (!stream) {
        this->recorder->mutex.unlock();
        return false;
    }

    int r = avcodec_parameters_from_context(stream->codecpar, ctx);
    if (r < 0) {
        this->recorder->mutex.unlock();
        return false;
    }

    this->recorder->audioStream.index = stream->index;
    this->recorder->cond.notify_all();
    this->recorder->videoInit = true;
    this->recorder->mutex.unlock();

    return true;
}

void RecoderAudioSink::close()
{
    this->recorder->mutex.lock();
    recorder->stopped = true;
    recorder->cond.notify_all();
    recorder->mutex.unlock();
}

bool RecoderAudioSink::push(AVPacket *packet)
{
    if(!this->recorder->audioInit)
        return false;
    this->recorder->mutex.lock();

    if (this->recorder->stopped) {
        // reject any new packet
        this->recorder->mutex.unlock();
        return false;
    }

    AVPacket *rec = recorder_packet_ref(packet);
    if (!rec) {
        //LOG_OOM();
        this->recorder->mutex.unlock();
        return false;
    }

    rec->stream_index = this->recorder->audioStream.index;

    this->recorder->audioQueue.push(rec);
    this->recorder->cond.notify_all();
    this->recorder->mutex.unlock();

    return true;
}

RecorderThread::RecorderThread(bool audio, bool video,
                   const std::string &filename,QObject *parent)
    : QThread{parent}
{
    this->audio = audio;
    this->video = video;
    this->filename = filename;
    stopped = false;
    videoInit = false;
    audioInit = false;
    videoStream = {
        .index = -1,
        .last_pts = AV_NOPTS_VALUE
    };
    audioStream = {
        .index = -1,
        .last_pts = AV_NOPTS_VALUE
    };
    recordFormat = this->getRecordFormat();
    if(this->video)
        videoSink = new RecoderVideoSink(this);
    else
        videoSink = NULL;
    if(this->audio)
        audioSink = new RecoderAudioSink(this);
    else
        audioSink = NULL;
}

RecorderThread::~RecorderThread()
{

}

void RecorderThread::ended()
{
    this->mutex.lock();
    this->stopped = true;
    this->mutex.unlock();
}

const AVOutputFormat *find_muxer(const char *name)
{
#ifdef LAVF_HAS_NEW_MUXER_ITERATOR_API
    void *opaque = NULL;
#endif
    const AVOutputFormat *oformat = NULL;
    do {
#ifdef LAVF_HAS_NEW_MUXER_ITERATOR_API
        oformat = av_muxer_iterate(&opaque);
#else
        oformat = av_oformat_next(oformat);
#endif \
    // until null or containing the requested name
    } while (oformat && !Utils::strListContains(oformat->name, ',', name));
    return oformat;
}
bool RecorderThread::openOutputFile()
{
    const char *format_name = Utils::getFormatName(this->recordFormat);
    assert(format_name);
    const AVOutputFormat *format = find_muxer(format_name);
    if (!format) {
        //LOGE("Could not find muxer");
        return false;
    }

    this->ctx = avformat_alloc_context();
    if (!this->ctx) {
        //LOG_OOM();
        return false;
    }

    int ret = avio_open(&this->ctx->pb, this->filename.c_str(),
                        AVIO_FLAG_WRITE);
    if (ret < 0) {
        //LOGE("Failed to open output file: %s", recorder->filename);
        avformat_free_context(this->ctx);
        return false;
    }

    // contrary to the deprecated API (av_oformat_next()), av_muxer_iterate()
    // returns (on purpose) a pointer-to-const, but AVFormatContext.oformat
    // still expects a pointer-to-non-const (it has not be updated accordingly)
    // <https://github.com/FFmpeg/FFmpeg/commit/0694d8702421e7aff1340038559c438b61bb30dd>
    this->ctx->oformat = (AVOutputFormat *) format;

    av_dict_set(&this->ctx->metadata, "comment",
                "Recorded by ToMobile " TOMOBILE_VERSION, 0);

    //LOGI("Recording started to %s file: %s", format_name, recorder->filename);
    return true;
}
bool set_extradata(AVStream *ostream, const AVPacket *packet)
{
    uint8_t *extradata = (uint8_t*)av_malloc(packet->size * sizeof(uint8_t));
    if (!extradata) {
        //LOG_OOM();
        return false;
    }

    // copy the first packet to the extra data
    memcpy(extradata, packet->data, packet->size);

    ostream->codecpar->extradata = extradata;
    ostream->codecpar->extradata_size = packet->size;
    return true;
}
bool RecorderThread::processHeader()
{
    this->mutex.lock();

    while (!this->stopped && this->videoQueue.empty())
    {
        this->cond.wait(&this->mutex);
    }

    if (this->video && this->videoQueue.empty())
    {
        assert(this->stopped);
        // If the recorder is stopped, don't process anything if there are not
        // at least video packets
        this->mutex.unlock();
        return false;
    }

    AVPacket *video_pkt = NULL;
    if (!this->videoQueue.empty()) {
        assert(this->video);
        video_pkt = this->videoQueue.back();
        this->videoQueue.pop();
    }

    AVPacket *audio_pkt = NULL;
    if (!this->audioQueue.empty()) {
        assert(this->audio);
        audio_pkt = this->audioQueue.back();
        this->audioQueue.pop();
    }

    this->mutex.unlock();

    int ret = false;
    bool ok = false;

    if (video_pkt) {
        if (video_pkt->pts != AV_NOPTS_VALUE) {
            //LOGE("The first video packet is not a config packet");
            goto end;
        }

        assert(this->videoStream.index >= 0);
        AVStream *video_stream =
            this->ctx->streams[this->videoStream.index];
        ok = set_extradata(video_stream, video_pkt);
        if (!ok) {
            goto end;
        }
    }

    if (audio_pkt) {
        if (audio_pkt->pts != AV_NOPTS_VALUE) {
            //LOGE("The first audio packet is not a config packet");
            goto end;
        }

        assert(this->audioStream.index >= 0);
        AVStream *audio_stream =
            this->ctx->streams[this->audioStream.index];
        ok = set_extradata(audio_stream, audio_pkt);
        if (!ok) {
            goto end;
        }
    }

    ok = avformat_write_header(this->ctx, NULL) >= 0;
    if (!ok) {
        //LOGE("Failed to write header to %s", recorder->filename);
        goto end;
    }

    ret = true;

end:
    if (video_pkt) {
        av_packet_free(&video_pkt);
    }
    if (audio_pkt) {
        av_packet_free(&audio_pkt);
    }

    return ret;
}
inline bool RecorderThread::writeVideo(AVPacket *packet)
{
    return writeStream(&this->videoStream, packet);
}
inline bool RecorderThread::writeAudio(AVPacket *packet)
{
    return writeStream(&this->audioStream, packet);
}
inline void rescale_packet(AVStream *stream, AVPacket *packet) {
    av_packet_rescale_ts(packet, TIME_BASE, stream->time_base);
}
bool RecorderThread::writeStream(RecorderStream *st, AVPacket *packet) {
    AVStream *stream = this->ctx->streams[st->index];
    rescale_packet(stream, packet);
    if (st->last_pts != AV_NOPTS_VALUE && packet->pts <= st->last_pts) {
        //LOGW("Fixing PTS non monotonically increasing in stream %d "
        //     "(%" PRIi64 " >= %" PRIi64 ")",
        //     st->index, st->last_pts, packet->pts);
        packet->pts = ++st->last_pts;
        packet->dts = packet->pts;
    } else {
        st->last_pts = packet->pts;
    }
    return av_interleaved_write_frame(this->ctx, packet) >= 0;
}
bool RecorderThread::processPackets()
{
    int64_t pts_origin = AV_NOPTS_VALUE;

    bool header_written = processHeader();
    if (!header_written) {
        return false;
    }

    AVPacket *video_pkt = NULL;
    AVPacket *audio_pkt = NULL;

    // We can write a video packet only once we received the next one so that
    // we can set its duration (next_pts - current_pts)
    AVPacket *video_pkt_previous = NULL;

    bool error = false;
    AVPacket *last = NULL;
    int ret = -1;
    for (;;) {
        if(this->isInterruptionRequested())
            break;
        this->mutex.lock();

        while (!this->stopped) {
            if (this->video && !video_pkt &&
                !this->videoQueue.empty()) {
                // A new packet may be assigned to video_pkt and be processed
                break;
            }
            if (this->audio && !audio_pkt
                && !this->audioQueue.empty()) {
                // A new packet may be assigned to audio_pkt and be processed
                break;
            }
            this->cond.wait(&this->mutex);
        }

        // If stopped is set, continue to process the remaining events (to
        // finish the recording) before actually stopping.

        // If there is no video, then the video_queue will remain empty forever
        // and video_pkt will always be NULL.
        assert(this->video || (!video_pkt
                                 && this->videoQueue.empty()));

        // If there is no audio, then the audio_queue will remain empty forever
        // and audio_pkt will always be NULL.
        assert(this->audio || (!audio_pkt
                                 && this->audioQueue.empty()));

        if (!video_pkt && !this->videoQueue.empty()) {
            video_pkt = this->videoQueue.back();
            this->videoQueue.pop();
        }

        if (!audio_pkt && !this->audioQueue.empty()) {
            audio_pkt = this->audioQueue.back();
            this->audioQueue.pop();
        }

        if (this->stopped && !video_pkt && !audio_pkt) {
            this->mutex.unlock();
            break;
        }

        assert(video_pkt || audio_pkt); // at least one

        this->mutex.unlock();

        // Ignore further config packets (e.g. on device orientation
        // change). The next non-config packet will have the config packet
        // data prepended.
        if (video_pkt && video_pkt->pts == AV_NOPTS_VALUE) {
            av_packet_free(&video_pkt);
            video_pkt = NULL;
        }

        if (audio_pkt && audio_pkt->pts == AV_NOPTS_VALUE) {
            av_packet_free(&audio_pkt);
            audio_pkt = NULL;
        }
        bool ok = false;


        if (pts_origin == AV_NOPTS_VALUE) {
            if (!this->audio) {
                assert(video_pkt);
                pts_origin = video_pkt->pts;
            } else if (!this->video) {
                assert(audio_pkt);
                pts_origin = audio_pkt->pts;
            } else if (video_pkt && audio_pkt) {
                pts_origin = std::min(video_pkt->pts, audio_pkt->pts);
            } else if (this->stopped) {
                if (video_pkt) {
                    // The recorder is stopped without audio, record the video
                    // packets
                    pts_origin = video_pkt->pts;
                } else {
                    // Fail if there is no video
                    error = true;
                    goto end;
                }
            } else {
                // We need both video and audio packets to initialize pts_origin
                continue;
            }
        }

        assert(pts_origin != AV_NOPTS_VALUE);

        if (video_pkt) {
            video_pkt->pts -= pts_origin;
            video_pkt->dts = video_pkt->pts;

            if (video_pkt_previous) {
                // we now know the duration of the previous packet
                video_pkt_previous->duration = video_pkt->pts
                                               - video_pkt_previous->pts;

                ok = writeVideo(video_pkt_previous);
                av_packet_free(&video_pkt_previous);
                if (!ok) {
                    //LOGE("Could not record video packet");
                    error = true;
                    goto end;
                }
            }

            video_pkt_previous = video_pkt;
            video_pkt = NULL;
        }

        if (audio_pkt) {
            audio_pkt->pts -= pts_origin;
            audio_pkt->dts = audio_pkt->pts;

            ok = writeAudio(audio_pkt);
            if (!ok) {
                //LOGE("Could not record audio packet");
                error = true;
                goto end;
            }

            av_packet_free(&audio_pkt);
            audio_pkt = NULL;
        }
    }

    // Write the last video packet
    last = video_pkt_previous;
    if (last) {
        // assign an arbitrary duration to the last packet
        last->duration = 100000;
        bool ok = writeVideo(last);
        if (!ok) {
            // failing to write the last frame is not very serious, no
            // future frame may depend on it, so the resulting file
            // will still be valid
            //LOGW("Could not record last packet");
        }
        av_packet_free(&last);
    }

    ret = av_write_trailer(this->ctx);
    if (ret < 0) {
        //LOGE("Failed to write trailer to %s", recorder->filename);
        error = false;
    }

end:
    if (video_pkt) {
        av_packet_free(&video_pkt);
    }
    if (audio_pkt) {
        av_packet_free(&audio_pkt);
    }

    return !error;
}
void RecorderThread::closeOutputFile()
{
    avio_close(this->ctx->pb);
    avformat_free_context(this->ctx);
}
void RecorderThread::run()
{
    qDebug()<<"recorder start ,thread is:"<<QThread::currentThreadId();
    bool ok = openOutputFile();
    if (ok) {
        ok = processPackets();
        if(!ok)
        {
            qDebug()<<"process packets failed!";
        }
        closeOutputFile();
        this->mutex.lock();
        this->stopped = true;
        this->mutex.unlock();
    }
    else
    {
        qDebug()<<"open output file failed!";
    }
}

RecoderVideoSink *RecorderThread::getVideoSink()
{
    return this->videoSink;
}

RecoderAudioSink *RecorderThread::getAudioSink()
{
    return this->audioSink;
}

inline bool RecorderThread::hasEmptyQueues()
{
    if (this->video && this->videoQueue.empty())
    {
        // The video queue is empty
        return true;
    }

    if (this->audio && this->audioQueue.empty())
    {
        // The audio queue is empty (when audio is enabled)
        return true;
    }

    // No queue is empty
    return false;
}

RecordFormat RecorderThread::getRecordFormat()
{
    if(this->filename.empty())
        return RECORD_FORMAT_NONE;
    if(this->filename.find("mp4"))
        return RECORD_FORMAT_MP4;
    if(this->filename.find("mkv"))
    {
        return RECORD_FORMAT_MKV;
    }
    if(this->filename.find("m4a"))
    {
        return RECORD_FORMAT_M4A;
    }
    if(this->filename.find("mka"))
    {
        return RECORD_FORMAT_MKA;
    }
    if(this->filename.find("opus"))
    {
        return RECORD_FORMAT_OPUS;
    }
    if(this->filename.find("aac")){
        return RECORD_FORMAT_AAC;
    }
    return RECORD_FORMAT_NONE;
}
void RecorderThread::stop()
{
    this->mutex.lock();
    this->stopped = true;
    cond.notify_all();
    this->mutex.unlock();
}
