#ifndef PACKET_SINK_H
#define PACKET_SINK_H
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <QObject>
#include "frame_buffer.h"
#include <functional>
struct PacketMerger {
    uint8_t *config;
    size_t configSize;
};

class FrameSink:public QObject{
    Q_OBJECT
public:
    explicit FrameSink(QObject *parent = nullptr);
    virtual bool open(AVCodecContext *ctx) = 0;
    virtual void close() = 0;
    virtual bool push(AVFrame *frame) = 0;
};
class ScreenSink:public FrameSink
{
    Q_OBJECT
public:
    explicit ScreenSink(QObject *parent = nullptr);
    ~ScreenSink();
    virtual bool open(AVCodecContext *ctx);
    virtual void close();
    virtual bool push(AVFrame *frame);
    FrameBuffer *getFrameBuffer();
signals:
    void screenInitSize(int w,int h);
    void newFrame();
private:
    FrameBuffer *frameBuffer;
};
typedef ScreenSink PreviewSink;
class PacketSink
{
public:
    PacketSink();
    ~PacketSink();
    virtual bool open(AVCodecContext *ctx) = 0;
    virtual void close() = 0;
    virtual bool push(AVPacket *packet) = 0;
    //virtual void disable() = 0;
};
class DecoderSink: public PacketSink{
public:
    DecoderSink();
    ~DecoderSink();
    void addFrameSink(FrameSink *sink);
    virtual bool open(AVCodecContext *ctx);
    virtual void close();
    virtual bool push(AVPacket *packet);
    //virtual void disable();
private:
    AVCodecContext *ctx;
    AVFrame *frame;
    std::vector<FrameSink*> sinks;

};
#endif // PACKET_SINK_H
