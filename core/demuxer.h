#ifndef DEMUXER_H
#define DEMUXER_H

#include <QObject>
#include <string>
#include <QTcpSocket>
#include <QThread>
#include "decoder.h"
#include "tcpsocket.h"
extern "C"{
#include <libavcodec/avcodec.h>
}
enum DemuxerStatus {
    DEMUXER_STATUS_EOS,
    DEMUXER_STATUS_DISABLED,
    DEMUXER_STATUS_ERROR,
};

class DemuxerThread : public QThread
{
    Q_OBJECT
public:
    explicit DemuxerThread(const std::string &name,
                     TcpSocket *socket,Decoder *decoder,QObject *parent = nullptr);
    ~DemuxerThread();
protected:
    virtual void run();
private:
    bool recvVideoSize(uint32_t *width,uint32_t *height);
    bool recvPacket(AVPacket *packet);
    bool packetMergerMerge(struct PacketMerger *merger, AVPacket *packet);

signals:
    void onEnded(DemuxerStatus status);

private:
    std::string name;
    TcpSocket *socket;
    Decoder *decoder;


};

#endif // DEMUXER_H
