#include "packet.h"
#include <assert.h>

void packet_source::add_sink(struct packet_sink *sink)
{

    this->sinks[this->sink_count++] = sink;
}

void packet_source::sinks_close_firsts(unsigned count)
{
    while (count) {
        struct packet_sink *sink = this->sinks[--count];
        sink->ops->close(sink);
    }
}

bool packet_source::sinks_open(AVCodecContext *ctx) {
    assert(this->sink_count);
    for (unsigned i = 0; i < this->sink_count; ++i) {
        struct packet_sink *sink = this->sinks[i];
        if (!sink->ops->open(sink, ctx)) {
            sinks_close_firsts(i);
            return false;
        }
    }
    return true;
}

void packet_source::sinks_close()
{
    assert(this->sink_count);
    sinks_close_firsts(this->sink_count);
}

bool packet_source::sinks_push(const AVPacket *packet)
{
    assert(this->sink_count);
    for (unsigned i = 0; i < this->sink_count; ++i) {
        struct packet_sink *sink = this->sinks[i];
        if (!sink->ops->push(sink, packet)) {
            return false;
        }
    }
    return true;
}
void packet_source::sinks_disable()
{
    assert(this->sink_count);
    for (unsigned i = 0; i < this->sink_count; ++i) {
        struct packet_sink *sink = this->sinks[i];
        if (sink->ops->disable) {
            sink->ops->disable(sink);
        }
    }
}
packet_source::packet_source()
{
    this->sink_count = 0;

}

packet_source::~packet_source()
{

}

