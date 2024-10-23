#include "packet_sink.h"
#include <QDebug>
PacketSink::PacketSink()
{

}

PacketSink::~PacketSink()
{

}
DecoderSink::DecoderSink()
{

}

DecoderSink::~DecoderSink()
{

}
void DecoderSink::addFrameSink(FrameSink *sink)
{
    sinks.push_back(sink);
}

bool DecoderSink::open(AVCodecContext *ctx)
{
    this->ctx = ctx;
    this->frame = av_frame_alloc();
    if (!this->frame) {
        //LOG_OOM();
        return false;
    }
    for(int i = 0;i<sinks.size();++i)
    {
        FrameSink *tmp = sinks[i];
        bool ok = tmp->open(ctx);
        if(!ok)
        {
            av_frame_free(&frame);
            return false;
        }

    }
    return true;
}

void DecoderSink::close()
{
    for(int i = 0;i<sinks.size();++i)
    {
        FrameSink *tmp = sinks[i];
        tmp->close();
    }
    av_frame_free(&this->frame);
}

bool DecoderSink::push(AVPacket *packet)
{
    bool is_config = packet->pts == AV_NOPTS_VALUE;
    if (is_config) {
        // nothing to do
        //qDebug()<<"nothing to do";
        return true;
    }

    int ret = avcodec_send_packet(this->ctx, packet);
    if (ret < 0 && ret != AVERROR(EAGAIN)) {
        qDebug()<<" could not send video packet:"<<ret;
        return false;
    }

    for (;;) {
        if(!frame)
            break;
        ret = avcodec_receive_frame(this->ctx,frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            //qDebug()<<"AVERROR(EAGAIN) || ret == AVERROR_EOF";
            break;
        }

        if (ret) {
            qDebug()<<"could not receive frame:"<<ret;
            return false;
        }
        // a frame was received
        for(int i = 0;i<sinks.size();++i)
        {
            FrameSink *tmp = sinks[i];
            bool ok = tmp->push(frame);

            if (!ok) {
                av_frame_unref(frame);
                // Error already logged
                return false;
            }

        }
        av_frame_unref(frame);
    }

    return true;
}

ScreenSink::ScreenSink(QObject *parent)
    : FrameSink(parent)
{
    frameBuffer = new FrameBuffer();
}

ScreenSink::~ScreenSink()
{
}

bool ScreenSink::open(AVCodecContext *ctx)
{
    assert(ctx->pix_fmt == AV_PIX_FMT_YUV420P);

    assert(ctx->width > 0 && ctx->width <= 0xFFFF);
    assert(ctx->height > 0 && ctx->height <= 0xFFFF);

    //emit screenInitSize(ctx->width,ctx->height);
    return true;
}

void ScreenSink::close()
{

}

bool ScreenSink::push(AVFrame *frame)
{
    bool previous_skipped;
    bool ok = this->frameBuffer->push(frame, &previous_skipped);
    if (!ok) {
        return false;
    }

    if (previous_skipped) {
        //sc_fps_counter_add_skipped_frame(&screen->fps_counter);
        // The SC_EVENT_NEW_FRAME triggered for the previous frame will consume
        // this new frame instead
        //qDebug()<<"this new frame instead";
    } else {
        emit newFrame();
    }

    return true;
}

FrameBuffer *ScreenSink::getFrameBuffer()
{
    return this->frameBuffer;
}


FrameSink::FrameSink(QObject *parent):QObject{parent}
{

}
