#include "decoder.h"
#include <QDebug>
Decoder::Decoder(const std::string &name)
{
    this->name = name;
}

Decoder::~Decoder()
{

}

void Decoder::addSink(PacketSink *sink)
{
    QMutexLocker mutexLocker(&mutex);
    this->sinks.push_back(sink);
}

bool Decoder::open(AVCodecContext *ctx)
{
    assert(sinks.size());
    for (int i = 0; i < sinks.size(); ++i) {
        PacketSink *sink = this->sinks[i];
        if (!sink->open(ctx)) {
            sink->close();
            return false;
        }
    }
    return true;
}
void Decoder:: close()
{
    assert(sinks.size());
    for (unsigned i = 0; i < sinks.size(); ++i) {
        PacketSink *sink = this->sinks[i];
        sink->close();
    }
}

bool Decoder::push(AVPacket *packet)
{
    for (unsigned i = 0; i < sinks.size(); ++i) {
        PacketSink *sink = this->sinks[i];
        sink->push(packet);
    }
    return true;
}

void Decoder::disable()
{
    assert(sinks.size());
    for (unsigned i = 0; i < sinks.size(); ++i) {
        //PacketSink *sink = this->sinks[i];
        //sink->disable();
    }
}
