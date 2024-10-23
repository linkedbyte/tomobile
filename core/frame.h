#ifndef FRAME_H
#define FRAME_H
extern "C"{
#include <libavcodec/avcodec.h>
}
#define FRAME_SOURCE_MAX_SINKS 2

struct frame_sink {
    const struct frame_sink_ops *ops;
};

struct frame_sink_ops {
    /* The codec context is valid until the sink is closed */
    bool (*open)(struct frame_sink *sink, const AVCodecContext *ctx);
    void (*close)(struct frame_sink *sink);
    bool (*push)(struct frame_sink *sink, const AVFrame *frame);
};

class frame_source {
public:
    frame_source();
    ~frame_source();
    void add_sink(struct frame_sink *sink);
    bool sinks_open(const AVCodecContext *ctx);
    void sinks_close();
    bool sinks_push(const AVFrame *frame);
    void sinks_close_firsts(unsigned count);

private:
    struct frame_sink *sinks[FRAME_SOURCE_MAX_SINKS];
    unsigned sink_count;
};

#endif // FRAME_H
