#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "common.h"
#include <string>
#include <map>
#include <random>
#include "adbprocess.h"
#include <ctime>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QSize>
#include "demuxer.h"
#include "decoder.h"
#include "./ui/videoform.h"
#include "recorder.h"
#include "tcpsocket.h"
#include "controller.h"
#include "./ui/previewwidget.h"
class TcpServer :public QTcpServer
{
    Q_OBJECT
public:
    TcpServer(QObject *parent = nullptr);
    ~TcpServer();
protected:
    virtual void incomingConnection(qintptr handle);
};
class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(const DeviceParams &params,const DeviceInfo &info,QObject *parent = nullptr);
    void usbConnect();

    std::string &getSerial();
    VideoForm *getVideoForm();
    void setPreviewWidget(PreviewWidget *widget);
    DeviceParams *getDeviceParams();
    DeviceInfo *getDeviceInfo();

private:
    uint32_t generateId();
    bool parserDeviceInfo();
    void process();
    void close();
    void adbStepProcess(AdbState state);

signals:
    void start(bool state);
    void newFrame(AVFrame *frame);

private:
    DeviceParams deviceParams;
    DeviceInfo deviceInfo;
    AdbProcess adbProcess;
    AdbStep adbStep;

    VideoForm *videoForm;
    PreviewWidget *previewWidget;

    TcpServer tcpServer;
    TcpSocket *videoSocket;
    TcpSocket *audioSocket;
    TcpSocket *controlSocket;

    DemuxerThread *videoDemuxer;
    DemuxerThread *audioDemuxer;
    Decoder *videoDecoder;
    Decoder *audioDecoder;
    RecorderThread *recorder;
    ControllerThread *controller;
};
class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();
    static DeviceManager *instance();

    Device *getDevice(const std::string &serial);
    bool addDevice(Device *device);
    void deleteDevice(Device *device);
    void deleteDevice(const std::string &serial);
    void deleteAll();
private:
    static DeviceManager *_instance;
    std::map<std::string,Device*> deviceMap;
    typedef std::map<std::string,Device*>::iterator DeviceIt;
};

#endif // DEVICE_H
