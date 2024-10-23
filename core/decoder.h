#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include "packet_sink.h"
#include <string>
#include <vector>
#include <QMutex>
#define MAX_PACKET_SINKS 2

class Decoder
{
public:
    explicit Decoder(const std::string &name);
    ~Decoder();
    void addSink(PacketSink *sink);
    bool open(AVCodecContext *ctx);
    void close();
    bool push(AVPacket *packet);
    void disable();

private:
    std::vector<PacketSink*> sinks;
    AVCodecContext *ctx;
    AVFrame *frame;
    std::string name;
    QMutex mutex;
};

#endif // DECODER_H
