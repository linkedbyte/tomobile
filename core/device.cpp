#include "device.h"
#include <iostream>
#include <string>
#include <sstream>
#include "recorder.h"
#include <QDebug>
#include <QMessageBox>

Device::Device(const struct DeviceParams &params,const DeviceInfo &info,QObject *parent)
    : QObject{parent}
{
    videoSocket = NULL;
    audioSocket = NULL;
    controlSocket = NULL;

    videoForm = NULL;
    previewWidget = NULL;

    this->videoDecoder = NULL;
    this->audioDecoder = NULL;
    this->controller = NULL;

    connect(videoForm,&VideoForm::signal_close,this,[=](){
        this->close();
    });
    deviceInfo = info;

    deviceParams = params;
    deviceParams.id = this->generateId();
    char pid[10] = {0};
    sprintf(pid,"%08x",deviceParams.id);
    deviceParams.socketName = SOCKET_NAME_PREFIX + std::string((char*)pid);

    adbStep = AdbStep::NONE;

    connect(&adbProcess,&AdbProcess::adbState,this,&Device::adbStepProcess);
    connect(&tcpServer, &QTcpServer::newConnection, this,[=](){
        QTcpSocket *temp = tcpServer.nextPendingConnection();
        if(!videoSocket && deviceParams.video)
        {
            videoSocket = (TcpSocket*)temp;
            if(!videoSocket->isValid() || !this->parserDeviceInfo())
            {
                qDebug()<<"video socket failed!";
                disconnect(&tcpServer, &QTcpServer::newConnection, this,0);
                emit start(false);
            }

        }
        else if(!audioSocket && deviceParams.audio)
        {
        audioSocket = (TcpSocket*)temp;
        if(!audioSocket->isValid())
            {
                qDebug()<<"audio socket failed!";
                emit start(false);
            }
        }
        else if(!controlSocket && deviceParams.control)
        {
            controlSocket = (TcpSocket*)temp;
            if(!controlSocket->isValid())
            {
                qDebug()<<"controller socket failed!";
                emit start(false);
            }
        }
        else
        {
            qDebug()<<"unknow socket!";
        }
        if(videoSocket && controlSocket)
        {
            emit start(true);
            AdbProcess temp;
            temp.reverseRemove(deviceParams.serial,deviceParams.socketName);
        }

    });

    connect(this,&Device::start,this,[=](bool state){
        disconnect(this,&Device::start, nullptr, nullptr);
        if(state)
        {
            qDebug()<<"device start state:"<<state;
            process();
        }
        else
        {
            qDebug()<<"device start failed!";
            QMessageBox msgBox;
            msgBox.setText("设备启动失败!");
            //msgBox.setInformativeText("Do you want to save your changes?");
            //msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            int ret = msgBox.exec();
        }
    });

}
void Device::adbStepProcess(AdbState state)
{
    qDebug()<<"adb worker state:"<<state<<";step:"<<adbStep;
    if(state == AdbState::SUCCESS_EXEC)
    {
        switch(adbStep)
        {
        case AdbStep::START:
            adbProcess.push(this->deviceParams.serial);
            adbStep = AdbStep::PUSH;
            break;
        case AdbStep::PUSH:
            adbProcess.reverse(this->deviceParams.serial,this->deviceParams.socketName,deviceParams.port);
            adbStep = AdbStep::REVERSE;
            break;
        case AdbStep::REVERSE:
        {
            int tcp_count = 0;
            if(deviceParams.video){
                tcp_count++;
            }
            if(deviceParams.audio)
                tcp_count++;
            if(deviceParams.control)
                tcp_count++;

            tcpServer.setMaxPendingConnections(tcp_count);
            if(!tcpServer.listen(QHostAddress::LocalHost, deviceParams.port))
            {
                qCritical() << QString("Could not listen on port %1").arg(deviceParams.port).toStdString().c_str();
                adbStep = AdbStep::NONE;
                AdbProcess temp;
                temp.reverseRemove(deviceParams.serial,deviceParams.socketName);
                emit start(false);
            }
            else
            {
                adbStep = AdbStep::CONNECT;
                adbProcess.executeDeviceService(this->deviceParams);
            }
        }
        break;
        case AdbStep::CONNECT:
        {
            //adb_process temp;
            //temp.reverse_remove(m_params.serial,m_params.socket_name);
        }
        break;
        default:
            adbStep = AdbStep::NONE;
            break;
        }
    }
    else
    {
        emit start(false);
    }
}
void Device::usbConnect()
{
    adbProcess.startServer();
    adbStep = AdbStep::START;
}

std::string &Device::getSerial()
{
    return deviceParams.serial;
}

VideoForm *Device::getVideoForm()
{
    return this->videoForm;
}

void Device::setPreviewWidget(PreviewWidget *widget)
{
    if(!this->previewWidget)
        this->previewWidget = widget;
}

DeviceParams *Device::getDeviceParams()
{
    return &this->deviceParams;
}

DeviceInfo *Device::getDeviceInfo()
{
    return &this->deviceInfo;
}

uint32_t Device::generateId()
{
    std::default_random_engine re;
    re.seed(time(0));
    return re() & 0x7FFFFFFF;
}
bool Device::parserDeviceInfo()
{
    unsigned char buf[TOMOBILE_NAME_FIELD_LENGTH];
    if(videoSocket->bytesAvailable() <= sizeof(buf))
    {
        videoSocket->waitForReadyRead();
    }

    qint64 len = videoSocket->read((char *)buf, sizeof(buf));
    if(len < TOMOBILE_NAME_FIELD_LENGTH) {
        qInfo("Could not retrieve device information");
        return false;
    }
    buf[TOMOBILE_NAME_FIELD_LENGTH - 1] = '\0'; // in case the client sends garbage
    // strcpy is safe here, since name contains at least DEVICE_NAME_FIELD_LENGTH bytes
    // and strlen(buf) < DEVICE_NAME_FIELD_LENGTH
    deviceParams.deviceName = (char *)buf;
    /*unsigned char buf1[4] = {0};
    if(p_video_socket->bytesAvailable() <= sizeof(buf1))
    {
        p_video_socket->waitForReadyRead(300);
    }

    len = p_video_socket->read((char *)buf1, sizeof(buf1));
    m_device_size.setWidth((buf1[0] << 8) | buf1[1]);
    m_device_size.setHeight((buf1[2] << 8) | buf1[3]);*/
    return true;
}
void Device::process()
{
    if(deviceParams.record)
    {
        recorder = new RecorderThread(true,true,"1.mp4");
        recorder->start();
    }

    if(videoSocket)
    {
        videoSocket->setParent(0);
        videoDecoder = new Decoder("video_decoder");
        DecoderSink *videoSink = new DecoderSink();
        videoDecoder->addSink(videoSink);
        if(deviceParams.record)
        {
            videoDecoder->addSink(recorder->getVideoSink());
        }
        this->videoForm = new VideoForm();
        this->videoForm->setTitle(deviceParams.deviceName);
        if(videoForm)
        {
            ScreenSink *screenSink = new ScreenSink();
            videoForm->setFrameBuffer(screenSink->getFrameBuffer());
            connect(screenSink,&ScreenSink::newFrame,this,[=](){
                videoForm->updateFrame();
            });

            videoSink->addFrameSink(screenSink);
        }
        //preview sink

        if(previewWidget)
        {
            PreviewSink *previewSink = new PreviewSink();
            previewWidget->setFrameBuffer(previewSink->getFrameBuffer());

            connect(previewSink,&ScreenSink::newFrame,this,[=](){
                previewWidget->updateFrame();
            });

            videoSink->addFrameSink(previewSink);
        }

        videoDemuxer = new DemuxerThread("video",videoSocket,videoDecoder);
        videoDemuxer->start();
    }
    /*if(p_audio_socket)
    {
        p_audio_decoder = new decoder("audio_decoder");
        decoder_sink *p_audio_sink = new decoder_sink();
        p_audio_decoder->add_sink(p_audio_sink);

        p_audio_decoder->add_sink(p_recorder->get_audio_sink());

        tcpsocket *audio_socket = this->p_audio_socket;
        this->p_audio_socket = nullptr;
        audio_socket->setParent(0);
        p_audio_demuxer = new demuxer("audio",audio_socket,p_audio_decoder);
        p_audio_demuxer->start();

    }*/
    if(deviceParams.control && controlSocket)
    {
        controlSocket->setParent(0);
        controller = new ControllerThread(controlSocket);
        this->videoForm->setController(controller);
        controller->start();
    }
}
void Device::close()
{
    this->videoDemuxer->requestInterruption();
    this->videoDecoder->close();

    this->recorder->stop();
    this->recorder->requestInterruption();
}

TcpServer::TcpServer(QObject *parent):QTcpServer(parent)
{

}

TcpServer::~TcpServer()
{

}

void TcpServer::incomingConnection(qintptr handle)
{
    TcpSocket *tmp = new TcpSocket();
    tmp->setSocketDescriptor(handle);
    addPendingConnection(tmp);
}

DeviceManager::DeviceManager()
{

}

DeviceManager::~DeviceManager()
{

}
DeviceManager *DeviceManager::_instance = NULL;
DeviceManager *DeviceManager::instance()
{
    static QMutex mutex;
    if (!_instance) {
        QMutexLocker locker(&mutex);
        if (!_instance) {
            _instance = new DeviceManager();
        }
    }
    return _instance;
}

Device *DeviceManager::getDevice(const std::string &serial)
{
    for(DeviceIt it = deviceMap.begin();it!=deviceMap.end();++it)
    {
        if(it->first == serial)
        {
            return it->second;
        }
    }
    return NULL;
}

bool DeviceManager::addDevice(Device *device)
{
    if(deviceMap.find(device->getSerial()) == deviceMap.end())
    {
        deviceMap.insert(std::make_pair(device->getSerial(),device));
        return true;
    }
    return false;
}

void DeviceManager::deleteDevice(Device *device)
{
    this->deleteDevice(device->getSerial());
}

void DeviceManager::deleteDevice(const std::string &serial)
{
    for(DeviceIt it = deviceMap.begin();it!=deviceMap.end();++it)
    {
        if(it->first == serial)
        {
            delete it->second;
            deviceMap.erase(it);
            break;
        }
    }
}

void DeviceManager::deleteAll()
{
    for(DeviceIt it = deviceMap.begin();it!=deviceMap.end();++it)
    {
        delete it->second;
        deviceMap.erase(it);
    }
}
