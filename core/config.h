#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <QSettings>
#include <QMutex>
#include "common.h"
#include <vector>

class Config
{
public:
    Config();
    ~Config();
    static Config *instance();
    void loadComm(CommParams &params);
    bool loadDevice(const std::string &serial,DeviceParams &params);
    void setValue(const QString &key, const QVariant &value);

    void updateComm(const CommParams &params);
    void updateDevice(const DeviceParams &params);

private:
    static Config *_instance;
    QSettings *settings;
};

#endif // CONFIG_H
