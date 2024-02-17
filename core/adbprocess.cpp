#include "adbprocess.h"
#include <QDebug>
#include "utils.h"
AdbProcess::AdbProcess(QObject *parent)
    : QProcess(parent)
{
    adbPath = "adb";

    connect(this,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus){
                if (QProcess::NormalExit == exitStatus && 0 == exitCode) {
                    emit adbState(AdbState::SUCCESS_EXEC);
                } else {
                    emit adbState(AdbState::ERROR_AUTH);
                }
                qDebug() << "adb return " << exitCode << ",exit status " << exitStatus;
    });

    connect(this, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error){
        if (QProcess::FailedToStart == error) {
            emit adbState(AdbState::ERROR_MISSING_BINARY);
        } else {
            emit adbState(AdbState::ERROR_START);
            qDebug()<<QString("qprocess start error:%1 %2").arg(program(),arguments().join(" "));
        }
        qDebug()<<"process error:"<<error;
    });

    connect(this, &QProcess::readyReadStandardError, this, [this]() {
        this->errorOutput = QString::fromUtf8(readAllStandardError()).trimmed();
        qDebug()<<this->errorOutput;
        emit adbState(AdbState::ERROR_EXEC);
    });

    connect(this, &QProcess::readyReadStandardOutput, this, [this]() {
        this->standardOutput = QString::fromUtf8(readAllStandardOutput()).trimmed();
        qDebug()<<this->standardOutput;
        if(this->arguments().contains("devices"))
        {
            this->parserDevicesFromOutput();
            emit updateDeviceVector(this->deviceVector);
        }
    });

    connect(this, &QProcess::started, this, [this](){
        qDebug()<<"adb process start";
    });

}

AdbProcess::~AdbProcess()
{
    this->killServer();
}

bool AdbProcess::startServer()
{
    QStringList args;
    args << "start-server";
    return execute(args);
}

bool AdbProcess::push(const std::string &serial)
{
    if(serial.empty())
        return false;
    QStringList args;
    args<<"-s";
    args<<serial.c_str();
    args<<"push";
    args<<LOCAL_SERVER_PATH;
    args<<DEVICE_SERVER_PATH;

    return execute(args);
}
bool AdbProcess::executeDeviceService(const struct DeviceParams &params)
{
    QStringList args;
    args << "-s";
    args << params.serial.c_str();
    args << "shell";
    args << QString("CLASSPATH=%1").arg(DEVICE_SERVER_PATH);
    args << "app_process";

#ifdef SERVER_DEBUGGER
#define SERVER_DEBUGGER_PORT "5005"

    args <<
#ifdef SERVER_DEBUGGER_METHOD_NEW
        /* Android 9 and above */
        "-XjdwpProvider:internal -XjdwpOptions:transport=dt_socket,suspend=y,server=y,address="
#else
        /* Android 8 and below */
        "-agentlib:jdwp=transport=dt_socket,suspend=y,server=y,address="
#endif
        SERVER_DEBUGGER_PORT,
#endif

    args << "/";
    args << "com.genymobile.scrcpy.Server";
    args << DEVICE_SERVER_VERSION;
    args << QString("scid=%1").arg(QString::asprintf("%08x",params.id));

    if (!params.video) {
        args << "video=false";
    }
    if (params.videoBitRate) {
        args << QString("video_bit_rate=%1").arg(QString::number(params.videoBitRate));
    }
    if (!params.audio) {
        args << "audio=false";
    }
    if (params.audioBitRate) {
        args << QString("audio_bit_rate=%1").arg(QString::number(params.audioBitRate));
    }
    if (!params.control) {
        args << "control=false";
    }
    if (params.videoCodec != CODEC_H264) {
        args <<"video_codec=" <<Utils::getVideoAudioCodec(params.videoCodec).c_str();
    }
    if (params.audioCodec != CODEC_OPUS) {
        args <<"audio_codec=" <<Utils::getVideoAudioCodec(params.audioCodec).c_str();
    }
    if (params.videoSource != VIDEO_SOURCE_DISPLAY) {
        args << "video_source=camera";
    }
    if (params.audioSource == AUDIO_SOURCE_MIC) {
        args << "audio_source=mic";
    }
    /*if (params.max_size) {
        args << QString("max_size=%1").arg(QString::number(params.max_size));
    }*/
    if (params.maxFps) {
        args << QString("max_fps=%1").arg(QString::number(params.maxFps));
    }
    if (params.lockVideoOrientation != LOCK_VIDEO_ORIENTATION_UNLOCKED) {
        args << QString("lock_video_orientation=%1").arg(QString::number(params.lockVideoOrientation));
    }
    if (params.showTouches) {
        args << "show_touches=true";
    }
    if (params.stayAwake) {
        args << "stay_awake=true";
    }
    if (!params.videoCodecOptions.empty()) {
        args << QString("video_codec_options=%1").arg(params.videoCodecOptions.c_str());
    }
//    if (!params.audioCodecOptionsodec_options.empty()) {
//        args << QString("audio_codec_options=%1").arg(params.audio_codec_options.c_str());
//    }
//    if (!params.video_encoder.empty()) {
//        args << QString("video_encoder=%1").arg(params.video_encoder.c_str());
//    }
//    if (!params.audio_encoder.empty()) {
//        args << QString("audio_encoder=%1").arg(params.audio_encoder.c_str());
//    }
    if (params.powerOffOnClose) {
        args << "power_off_on_close=true";
    }
    if (!params.clipboardAutosync) {
        args << "clipboard_autosync=false";
    }
//    if (!params->downsize_on_error) {
//        // By default, downsize_on_error is true
//        ADD_PARAM("downsize_on_error=false");
//    }
//    if (!params->cleanup) {
//        // By default, cleanup is true
//        ADD_PARAM("cleanup=false");
//    }
//    if (!params->power_on) {
//        // By default, power_on is true
//        ADD_PARAM("power_on=false");
//    }

    // mark: crop input format: "width:height:x:y" or "" for no crop, for example: "100:200:0:0"
    // 这条adb命令是阻塞运行的,进程不会退出了
    return execute(args);
}

bool AdbProcess::killServer()
{
    QStringList args;
    args << "kill-server";
    return execute(args);
}

bool AdbProcess::deviceList()
{
    QStringList args;
    args<<"devices"<<"-l";
    return execute(args);
}
bool AdbProcess::isRunning()
{
    if(QProcess::NotRunning == state())
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool AdbProcess::reverse(const std::string &serial, const std::string &device_socket_name, uint16_t local_port)
{
    if(serial.empty() || device_socket_name.empty())
        return false;
    QStringList args;
    args <<"-s" << serial.c_str();
    args << "reverse";
    args << QString("localabstract:%1").arg(device_socket_name.c_str());
    args << QString("tcp:%1").arg(local_port);
    return execute(args);
}

bool AdbProcess::reverseRemove(const std::string &serial, const std::string &device_socket_name)
{
    if(serial.empty() || device_socket_name.empty())
        return false;
    QStringList args;
    args <<"-s";
    args << serial.c_str();
    args << "reverse";
    args << "--remove";
    args << QString("localabstract:%1").arg(device_socket_name.c_str());
    return execute(args);
}

bool AdbProcess::execute(QStringList &args)
{
    if(args.empty())
        return false;

    standardOutput = "";
    errorOutput = "";
    qDebug()<<"execute : adb "<<args;
    start(this->adbPath.c_str(),args);
    return true;
}

bool AdbProcess::parserDevicesFromOutput()
{
    deviceVector.clear();

    QStringList device_info_list = this->standardOutput.split(QRegExp("\r\n|\n"), Qt::SkipEmptyParts);
    if(device_info_list.size() <= 1)
    {
        qDebug()<<"none devices!";
        return false;
    }
    for (int i = 1;i<device_info_list.size();++i) {
        DeviceInfo device;
        QStringList device_it = device_info_list[i].split(" ");
        device_it.removeAll("");
        device.serial = device_it[0].trimmed().toStdString();
        device.state = device_it[1].toStdString();
        if(device.state == "offline")
        {
            deviceVector.push_back(device);
            continue;
        }
        device.product = device_it[2].split(':')[1].toStdString();
        device.model = device_it[3].split(':')[1].toStdString();
        device.device = device_it[4].split(':')[1].toStdString();
        deviceVector.push_back(device);
    }
    return true;
}
