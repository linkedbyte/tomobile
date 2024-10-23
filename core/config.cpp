#include "config.h"
#include "utils.h"

Config *Config::_instance = 0;

Config::Config()
{
    settings = new QSettings(TM_FILE_NAME,QSettings::IniFormat);
    settings->setIniCodec("UTF-8");
}

Config::~Config()
{
    if(settings)
        delete settings;
}

Config *Config::instance()
{
    static QMutex mutex;
    if (!_instance) {
        QMutexLocker locker(&mutex);
        if (!_instance) {
            _instance = new Config();
        }
    }
    return _instance;
}

void Config::loadComm(CommParams &params)
{
    settings->beginGroup("COMM");
    params.record = settings->value("record",true).toBool();
    params.recordPath = settings->value("record_path",TM_RECORD_PATH).toString().toStdString();
    params.logLevel = settings->value("log_level","info").toString().toStdString();
    Utils::parseVideoCodec(settings->value("video_codec","h264").toString().toStdString().c_str(),&params.videoCodec);
    Utils::parseAudioCodec(settings->value("audio_codec","opus").toString().toStdString().c_str(),&params.audioCodec);
    Utils::parseVideoSource(settings->value("video_source","display").toString().toStdString().c_str(),&params.videoSource);
    Utils::parseAudioSource(settings->value("audio_source","output").toString().toStdString().c_str(),&params.audioSource);
    params.port = settings->value("port",TM_DEFAULT_PORT).toInt();
    params.videoBitRate = settings->value("video_bit_rate",TM_DEFAULT_VIDEO_BIT_RATE).toInt();
    params.audioBitRate = settings->value("audio_bit_rate",TM_DEFAULT_VIDEO_BIT_RATE).toInt();
    params.maxFps = settings->value("max_fps",TM_DEFAULT_MAX_FPS).toInt();
    params.control = settings->value("control",true).toBool();
    params.video = settings->value("video",true).toBool();
    params.audio = settings->value("audio",true).toBool();
    params.showTouches = settings->value("show_touches",false).toBool();
    params.stayAwake = settings->value("stay_awake",true).toBool();
    params.powerOffOnClose = settings->value("power_off_on_close",false).toBool();
    params.clipboardAutosync = settings->value("clipboard_autosync",true).toBool();
    params.powerOn = settings->value("power_on",true).toBool();
    params.screenCapturePath = settings->value("screen_capture_path",TM_SCREEN_CAPTURE_PATH).toString().toStdString();
    settings->endGroup();

}
bool Config::loadDevice(const std::string &serial,DeviceParams &params)
{
    QStringList groups = settings->childGroups();

    if(groups.indexOf(serial.c_str()) != -1)
    {
        settings->beginGroup(serial.c_str());
        params.record = settings->value("record",true).toBool();
        params.recordPath = settings->value("record_path",TM_RECORD_PATH).toString().toStdString();
        params.logLevel = settings->value("log_level","info").toString().toStdString();
        Utils::parseVideoCodec(settings->value("video_codec","h264").toString().toStdString().c_str(),&params.videoCodec);
        Utils::parseAudioCodec(settings->value("audio_codec","opus").toString().toStdString().c_str(),&params.audioCodec);
        Utils::parseVideoSource(settings->value("video_source","display").toString().toStdString().c_str(),&params.videoSource);
        Utils::parseAudioSource(settings->value("audio_source","output").toString().toStdString().c_str(),&params.audioSource);
        params.port = settings->value("port",TM_DEFAULT_PORT).toInt();
        params.videoBitRate = settings->value("video_bit_rate",TM_DEFAULT_VIDEO_BIT_RATE).toInt();
        params.audioBitRate = settings->value("audio_bit_rate",TM_DEFAULT_VIDEO_BIT_RATE).toInt();
        params.maxFps = settings->value("max_fps",TM_DEFAULT_MAX_FPS).toInt();
        params.control = settings->value("control",true).toBool();
        params.video = settings->value("video",true).toBool();
        params.audio = settings->value("audio",true).toBool();
        params.showTouches = settings->value("show_touches",false).toBool();
        params.stayAwake = settings->value("stay_awake",true).toBool();
        params.powerOffOnClose = settings->value("power_off_on_close",false).toBool();
        params.clipboardAutosync = settings->value("clipboard_autosync",true).toBool();
        params.powerOn = settings->value("power_on",true).toBool();
        params.screenCapturePath = settings->value("screen_capture_path",TM_SCREEN_CAPTURE_PATH).toString().toStdString();
        settings->endGroup();
        return true;
    }
    else
    {
        return false;
    }

}
void Config::setValue(const QString &key, const QVariant &value)
{
    settings->setValue(key,value);
    settings->sync();
}

void Config::updateComm(const CommParams &params)
{
    settings->beginGroup("COMM");
    settings->setValue("record",params.record);
    settings->setValue("record_path",params.recordPath.c_str());
    //settings->setValue("log_level",params.);
    settings->setValue("video_codec",Utils::getVideoAudioCodec(params.videoCodec).c_str());
    settings->setValue("audio_codec",Utils::getVideoAudioCodec(params.videoCodec).c_str());
    settings->setValue("video_source",Utils::getVideoSource(params.videoSource).c_str());
    settings->setValue("audio_source",Utils::getVideoSource(params.videoSource).c_str());
    settings->setValue("port",params.port);
    settings->setValue("video_bit_rate",params.videoBitRate);
    settings->setValue("audio_bit_rate",params.audioBitRate);
    settings->setValue("max_fps",params.maxFps);
    settings->setValue("control",params.control);
    settings->setValue("video",params.video);
    settings->setValue("audio",params.audio);
    settings->setValue("show_touches",params.showTouches);
    settings->setValue("stay_awake",params.stayAwake);
    settings->setValue("power_off_on_close",params.powerOffOnClose);
    settings->setValue("clipboard_autosync",params.clipboardAutosync);
    settings->setValue("power_on",params.powerOn);
    settings->setValue("screen_capture_path",params.screenCapturePath.c_str());
    settings->endGroup();
    settings->sync();
}

void Config::updateDevice(const DeviceParams &params)
{
    settings->beginGroup(params.serial.c_str());
    settings->setValue("record",params.record);
    settings->setValue("record_path",params.recordPath.c_str());
    settings->setValue("video_codec",Utils::getVideoAudioCodec(params.videoCodec).c_str());
    settings->setValue("audio_codec",Utils::getVideoAudioCodec(params.videoCodec).c_str());
    settings->setValue("video_source",Utils::getVideoSource(params.videoSource).c_str());
    settings->setValue("audio_source",Utils::getVideoSource(params.videoSource).c_str());
    settings->setValue("port",params.port);
    settings->setValue("video_bit_rate",params.videoBitRate);
    settings->setValue("audio_bit_rate",params.audioBitRate);
    settings->setValue("max_fps",params.maxFps);
    settings->setValue("control",params.control);
    settings->setValue("video",params.video);
    settings->setValue("audio",params.audio);
    settings->setValue("show_touches",params.showTouches);
    settings->setValue("stay_awake",params.stayAwake);
    settings->setValue("power_off_on_close",params.powerOffOnClose);
    settings->setValue("clipboard_autosync",params.clipboardAutosync);
    settings->setValue("power_on",params.powerOn);
    settings->setValue("screen_capture_path",params.screenCapturePath.c_str());
    settings->endGroup();
    settings->sync();
}
