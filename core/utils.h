#ifndef UTILS_H
#define UTILS_H

#include "common.h"
class Utils
{
public:
    Utils();
    static const char *getFormatName(enum RecordFormat format);
    static bool strListContains(const char *list, char sep, const char *s);
    static bool parseVideoCodec(const char *optarg, enum VideoAudioCodec *codec);
    static bool parseAudioCodec(const char *optarg, enum VideoAudioCodec *codec);
    static bool parseVideoSource(const char *optarg, enum VideoSource *source);
    static bool parseAudioSource(const char *optarg, enum AudioSource *source);
    static std::string getVideoAudioCodec(enum VideoAudioCodec codec);
    static std::string getVideoSource(enum VideoSource source);
    static std::string getAudioSource(enum AudioSource source);
};

#endif // UTILS_H
