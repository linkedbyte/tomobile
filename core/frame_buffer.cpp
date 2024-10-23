#include "frame_buffer.h"

FrameBuffer::FrameBuffer()
{
    this->init();
}

FrameBuffer::~FrameBuffer()
{
    this->destory();
}

bool FrameBuffer::init()
{
    this->pendingFrame = av_frame_alloc();
    if (!this->pendingFrame) {
        //LOG_OOM();
        return false;
    }

    this->tmpFrame = av_frame_alloc();
    if (!this->tmpFrame) {
        //LOG_OOM();
        av_frame_free(&this->pendingFrame);
        return false;
    }

    // there is initially no frame, so consider it has already been consumed
    this->pendingFrameConsumed = true;

    return true;
}

void FrameBuffer::destory()
{
    av_frame_free(&this->pendingFrame);
    av_frame_free(&this->tmpFrame);
}
static inline void
swap_frames(AVFrame **lhs, AVFrame **rhs) {
    AVFrame *tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}
bool FrameBuffer::push(AVFrame *frame, bool *skipped)
{
    int r = av_frame_ref(this->tmpFrame, frame);
    if (r) {
        //LOGE("Could not ref frame: %d", r);
        return false;
    }

    mutex.lock();

    // Now that av_frame_ref() succeeded, we can replace the previous
    // pending_frame
    swap_frames(&this->pendingFrame, &this->tmpFrame);
    av_frame_unref(this->tmpFrame);

    if (skipped) {
        *skipped = !this->pendingFrameConsumed;
    }
    this->pendingFrameConsumed = false;

    mutex.unlock();

    return true;
}

void FrameBuffer::consume(AVFrame *dst)
{
    mutex.lock();

    this->pendingFrameConsumed = true;

    av_frame_move_ref(dst, this->pendingFrame);
    // av_frame_move_ref() resets its source frame, so no need to call
    // av_frame_unref()

    mutex.unlock();
}
