#ifndef COMMON_H
#define COMMON_H
#include <string>
extern "C"{
#include <libavformat/version.h>
}
#define UPDATE_URL "http://linkedbyte.com/tomobile/update"
#define TOMOBILE_SERVER_PATH "/data/local/tmp/tomobile-server.apk"
#define LOCAL_SERVER_PATH "tomobile-server"
#define TOMOBILE_SERVER_VERSION "1.0"
#define TOMOBILE_NAME_FIELD_LENGTH 64
#define SOCKET_NAME_PREFIX "tomobile_"
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(58, 9, 100)
# define LAVF_HAS_NEW_MUXER_ITERATOR_API
#else
# define LAVF_REQUIRES_REGISTER_ALL
#endif
#define TOMOBILE_VERSION "1.0"

#define TM_DEFAULT_PORT 19818
#define TM_DEFAULT_VIDEO_BIT_RATE 8000000
#define TM_DEFAULT_AUDIO_BIT_RATE 128000
#define TM_DEFAULT_MAX_FPS 30
#define TM_RECORD_PATH "./record"
#define TM_SCREEN_CAPTURE_PATH "./screenshot"
#define TM_FILE_NAME "./config.ini"
struct DeviceInfo
{
    DeviceInfo(){}
    std::string product;
    std::string device;
    std::string serial;
    std::string model;
    std::string state;
    bool operator == (const DeviceInfo &v)
    {
        return this->serial == v.serial;
    }
};
enum AdbState
{
    SUCCESS_START=1,        // 启动成功
    ERROR_START,          // 启动失败
    SUCCESS_EXEC,         // 执行成功
    ERROR_EXEC,           // 执行失败
    ERROR_MISSING_BINARY, // 找不到文件
    ERROR_AUTH,
    FINISHED,
};
enum VideoAudioCodec {
    CODEC_H264,
    CODEC_H265,
    CODEC_AV1,
    CODEC_OPUS,
    CODEC_AAC,
    CODEC_RAW,
};

enum VideoSource {
    VIDEO_SOURCE_DISPLAY,
    VIDEO_SOURCE_CAMERA,
};

enum AudioSource {
    AUDIO_SOURCE_AUTO, // OUTPUT for video DISPLAY, MIC for video CAMERA
    AUDIO_SOURCE_OUTPUT,
    AUDIO_SOURCE_MIC,
};
enum CameraFacing {
    CAMERA_FACING_ANY,
    CAMERA_FACING_FRONT,
    CAMERA_FACING_BACK,
    CAMERA_FACING_EXTERNAL,
};

enum LockVideoOrientation {
    LOCK_VIDEO_ORIENTATION_UNLOCKED = -1,
    LOCK_VIDEO_ORIENTATION_INITIAL = -2,
    LOCK_VIDEO_ORIENTATION_0 = 0,
    LOCK_VIDEO_ORIENTATION_1,
    LOCK_VIDEO_ORIENTATION_2,
    LOCK_VIDEO_ORIENTATION_3,
};
enum RecordFormat {
    RECORD_FORMAT_AUTO,
    RECORD_FORMAT_MP4,
    RECORD_FORMAT_MKV,
    RECORD_FORMAT_M4A,
    RECORD_FORMAT_MKA,
    RECORD_FORMAT_OPUS,
    RECORD_FORMAT_AAC,
    RECORD_FORMAT_NONE
};
struct CommParams
{
    bool record = true;
    std::string recordPath = TM_RECORD_PATH;
    std::string logLevel = "info";
    enum VideoAudioCodec videoCodec = CODEC_H264;
    enum VideoAudioCodec audioCodec = CODEC_OPUS;
    enum VideoSource videoSource = VIDEO_SOURCE_DISPLAY;
    enum AudioSource audioSource = AUDIO_SOURCE_AUTO;
    std::string crop = "";
    std::string videoCodecOptions = "";
    std::string audioCodecOptions = "";
    std::string videoEncoder = "";
    std::string audioEncoder = "";
    uint16_t port = TM_DEFAULT_PORT;
    uint32_t videoBitRate = TM_DEFAULT_VIDEO_BIT_RATE;
    uint32_t audioBitRate = TM_DEFAULT_AUDIO_BIT_RATE;
    uint16_t maxFps = TM_DEFAULT_MAX_FPS;
    bool control = true;
    bool video = true;
    bool audio = true;
    bool showTouches = false;
    bool stayAwake = false;
    bool powerOffOnClose = false;
    bool clipboardAutosync = true;
    bool powerOn = true;
    std::string screenCapturePath = TM_SCREEN_CAPTURE_PATH;
    enum LockVideoOrientation lockVideoOrientation = LOCK_VIDEO_ORIENTATION_UNLOCKED;
};
struct DeviceParams : CommParams
{
    std::string deviceName = "";
    std::string socketName = "";
    uint32_t id = 0x00000000;
    std::string serial = "";
    std::string nickName = "";
    DeviceParams& operator=(CommParams &commParams)
    {
        this->audio = commParams.audio;
        this->recordPath = commParams.recordPath;
        this->logLevel = commParams.logLevel;
        this->videoCodec = commParams.videoCodec;
        this->audioCodec = commParams.audioCodec;
        this->videoSource = commParams.videoSource;
        this->audioSource = commParams.audioSource;
        this->port = commParams.port;
        this->videoBitRate = commParams.videoBitRate;
        this->audioBitRate = commParams.audioBitRate;
        this->maxFps = commParams.maxFps;
        this->control = commParams.control;
        this->video = commParams.video;
        this->audio = commParams.audio;
        this->showTouches = commParams.showTouches;
        this->stayAwake = commParams.stayAwake;
        this->powerOffOnClose = commParams.powerOffOnClose;
        this->clipboardAutosync = commParams.clipboardAutosync;
        this->powerOn = commParams.powerOn;
        this->screenCapturePath = commParams.screenCapturePath;
        this->lockVideoOrientation = commParams.lockVideoOrientation;
        return *this;
    }
};
#endif // COMMON_H
