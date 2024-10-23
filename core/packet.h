#ifndef PACKET_H
#define PACKET_H

#define PACKET_SOURCE_MAX_SINKS 2

extern "C"{
#include <libavcodec/avcodec.h>
}
struct packet_sink {
    const struct packet_sink_ops *ops;
};

struct packet_sink_ops {
    /* The codec context is valid until the sink is closed */
    bool (*open)(struct packet_sink *sink, AVCodecContext *ctx);
    void (*close)(struct packet_sink *sink);
    bool (*push)(struct packet_sink *sink, const AVPacket *packet);

    /*/
     * Called when the input stream has been disabled at runtime.
     *
     * If it is called, then open(), close() and push() will never be called.
     *
     * It is useful to notify the recorder that the requested audio stream has
     * finally been disabled because the device could not capture it.
     */
    void (*disable)(struct packet_sink *sink);
};

class packet_source
{
public:
    packet_source();
    ~packet_source();

    void add_sink(struct packet_sink *sink);
    bool sinks_open(AVCodecContext *ctx);
    void sinks_close();
    bool sinks_push(const AVPacket *packet);
    void sinks_disable();
    void sinks_close_firsts(unsigned count);
private:
    struct packet_sink *sinks[PACKET_SOURCE_MAX_SINKS];
    unsigned sink_count;

};
struct packet_merger {
    uint8_t *config;
    size_t config_size;
};
#endif // PACKET_H
