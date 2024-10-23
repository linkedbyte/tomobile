#include "utils.h"
#include <string>
#include <string.h>
Utils::Utils()
{

}
const char *Utils::getFormatName(enum RecordFormat format)
{
    switch (format) {
    case RECORD_FORMAT_MP4:
    case RECORD_FORMAT_M4A:
    case RECORD_FORMAT_AAC:
        return "mp4";
    case RECORD_FORMAT_MKV:
    case RECORD_FORMAT_MKA:
        return "matroska";
    case RECORD_FORMAT_OPUS:
        return "opus";
    default:
        return NULL;
    }
}
bool Utils::strListContains(const char *list, char sep, const char *s) {
    char *p;
    do {
        p = strchr(list, sep);

        size_t token_len = p ? (size_t) (p - list) : strlen(list);
        if (!strncmp(list, s, token_len)) {
            return true;
        }

        if (p) {
            list = p + 1;
        }
    } while (p);
    return false;
}
bool Utils::parseVideoCodec(const char *optarg, enum VideoAudioCodec *codec) {
    if (!strcmp(optarg, "h264")) {
        *codec = CODEC_H264;
        return true;
    }
    if (!strcmp(optarg, "h265")) {
        *codec = CODEC_H265;
        return true;
    }
    if (!strcmp(optarg, "av1")) {
        *codec = CODEC_AV1;
        return true;
    }
    return false;
}
std::string Utils::getVideoAudioCodec(enum VideoAudioCodec codec) {
    std::string ret = "unknow";
    switch(codec)
    {
    case CODEC_H264:
        ret = "h264";
        break;
    case CODEC_H265:
        ret = "h265";
        break;
    case CODEC_AV1:
        ret = "av1";
        break;
    case CODEC_OPUS:
        ret = "opus";
        break;
    case CODEC_AAC:
        ret = "acc";
        break;
    case CODEC_RAW:
        ret = "raw";
        break;
    default:
        break;
    }
    return ret;
}
std::string Utils::getVideoSource(enum VideoSource source)
{
    std::string ret = "unknow";
    switch(source)
    {
    case VIDEO_SOURCE_DISPLAY:
        ret = "display";
        break;
    case VIDEO_SOURCE_CAMERA:
        ret = "camera";
        break;
    default:
        break;
    }
    return ret;
}
std::string Utils::getAudioSource(enum AudioSource source)
{
    std::string ret = "unknow";
    switch(source)
    {
    case AUDIO_SOURCE_MIC:
        ret = "mic";
        break;
    case AUDIO_SOURCE_OUTPUT:
        ret = "output";
        break;
    default:
        break;
    }
    return ret;
}
bool Utils::parseAudioCodec(const char *optarg, enum VideoAudioCodec *codec) {
    if (!strcmp(optarg, "opus")) {
        *codec = CODEC_OPUS;
        return true;
    }
    if (!strcmp(optarg, "aac")) {
        *codec = CODEC_AAC;
        return true;
    }
    if (!strcmp(optarg, "raw")) {
        *codec = CODEC_RAW;
        return true;
    }
    return false;
}

bool Utils::parseVideoSource(const char *optarg, enum VideoSource *source) {
    if (!strcmp(optarg, "display")) {
        *source = VIDEO_SOURCE_DISPLAY;
        return true;
    }

    if (!strcmp(optarg, "camera")) {
        *source = VIDEO_SOURCE_CAMERA;
        return true;
    }
    return false;
}
bool Utils::parseAudioSource(const char *optarg, enum AudioSource *source) {
    if (!strcmp(optarg, "mic")) {
        *source = AUDIO_SOURCE_MIC;
        return true;
    }

    if (!strcmp(optarg, "output")) {
        *source = AUDIO_SOURCE_OUTPUT;
        return true;
    }
    return false;
}
