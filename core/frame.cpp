#include "frame.h"
#include <assert.h>
frame_source::frame_source()
{
    sink_count = 0;
}

frame_source::~frame_source()
{

}

void frame_source::add_sink(frame_sink *sink)
{
    assert(sink_count < FRAME_SOURCE_MAX_SINKS);
    assert(sink);
    assert(sink->ops);
    sinks[sink_count++] = sink;
}

bool frame_source::sinks_open(const AVCodecContext *ctx)
{
    assert(sink_count);
    for(unsigned i = 0; i < sink_count; ++i) {
        struct frame_sink *sink = sinks[i];
        if (!sink->ops->open(sink, ctx)) {
            sinks_close_firsts(i);
            return false;
        }
    }
    return true;
}
void frame_source::sinks_close_firsts(unsigned count)
{
    while(count) {
        struct frame_sink *sink = sinks[--count];
        sink->ops->close(sink);
    }
}
void frame_source::sinks_close()
{
    assert(sink_count);
    sinks_close_firsts(sink_count);
}

bool frame_source::sinks_push(const AVFrame *frame)
{
    assert(sink_count);
    for (unsigned i = 0; i < sink_count; ++i)
    {
        struct frame_sink *sink = sinks[i];
        if (!sink->ops->push(sink, frame)) {
            return false;
        }
    }

    return true;
}
