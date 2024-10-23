#ifndef RECORDER_H
#define RECORDER_H
#include <stdint.h>
#include "packet_sink.h"
#include <string>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <queue>
#include "common.h"

struct RecorderStream {
    int index;
    int64_t last_pts;
};
class RecoderVideoSink;
class RecoderAudioSink;
class RecorderThread :public QThread
{
    friend class RecoderVideoSink;
    friend class RecoderAudioSink;
    Q_OBJECT
public:
    RecorderThread(bool audio,bool video,
             const std::string &filename,QObject *parent = nullptr);
    ~RecorderThread();
    void ended();
    void stop();
    virtual void run();
    RecoderVideoSink *getVideoSink();
    RecoderAudioSink *getAudioSink();
private:
    RecordFormat getRecordFormat();
    bool openOutputFile();
    bool processHeader();
    bool processPackets();
    void closeOutputFile();
    inline bool writeVideo(AVPacket *packet);
    inline bool writeAudio(AVPacket *packet);
    bool writeStream(RecorderStream *st, AVPacket *packet);
    inline bool hasEmptyQueues();
private:
    bool audio;
    bool video;
    std::string filename;
    AVFormatContext *ctx;

    QMutex mutex;
    QWaitCondition cond;
    bool stopped;
    std::queue<AVPacket*> videoQueue;
    std::queue<AVPacket*> audioQueue;
    RecordFormat recordFormat;
    RecorderStream videoStream;
    RecorderStream audioStream;
    RecoderVideoSink *videoSink;
    RecoderAudioSink *audioSink;
    bool videoInit;
    bool audioInit;

};

class RecoderVideoSink:public PacketSink{
public:
    RecoderVideoSink(RecorderThread *handler);
    ~RecoderVideoSink();

    virtual bool open(AVCodecContext *ctx);
    virtual void close();
    virtual bool push(AVPacket *packet);
    //virtual void disable();
    //void set_recorder_handle(recorder *rec);
private:
    RecorderThread *recorder;

};
class RecoderAudioSink:public PacketSink{
public:
    RecoderAudioSink(RecorderThread *handler);
    ~RecoderAudioSink();

    virtual bool open(AVCodecContext *ctx);
    virtual void close();
    virtual bool push(AVPacket *packet);
    //virtual void disable();
    //void set_recorder_handle(recorder *rec);
private:
    RecorderThread *recorder;
};
#endif // RECORDER_H
