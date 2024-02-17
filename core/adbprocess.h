#ifndef ADBPROCESS_H
#define ADBPROCESS_H

#include <QObject>
#include <QProcess>
#include <vector>
#include "common.h"

enum AdbStep
{
    NONE,
    START,
    PUSH,
    REVERSE,
    EXECUTE,
    CONNECT,
    DONE,
};

class AdbProcess : public QProcess
{
    Q_OBJECT
public:
    explicit AdbProcess(QObject *parent = nullptr);
    virtual ~AdbProcess();

    bool startServer();
    bool push(const std::string &serial);
    bool install(const std::string &serial);
    bool killServer();
    bool deviceList();
    bool isRunning();
    bool reverse(const std::string &serial, const std::string &deviceSocketName, uint16_t local_port);
    bool reverseRemove(const std::string &serial, const std::string &deviceSocketName);
    bool execute(QStringList &args);
    bool executeDeviceService(const DeviceParams &params);

signals:
    void updateDeviceVector(std::vector<DeviceInfo> &vec);
    void adbState(AdbState state);

private:
    bool parserDevicesFromOutput();


private:
    QString standardOutput;
    QString errorOutput;
    std::string adbPath;
    std::vector<DeviceInfo> deviceVector;
    QProcess process;
    AdbStep adbStep;
};

#endif // ADBPROCESS_H
