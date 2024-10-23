#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <QMutex>
class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();

    bool init();
    void destory();
    bool push(AVFrame *frame,bool *skipped);
    void consume(AVFrame *dst);
private:
    AVFrame *pendingFrame;
    AVFrame *tmpFrame; // To preserve the pending frame on error
    QMutex  mutex;

    bool pendingFrameConsumed;
};
#endif // FRAME_BUFFER_H
