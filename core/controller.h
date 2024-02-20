#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QMouseEvent>
#include <queue>
#include "control_msg.h"
#include "tcpsocket.h"
#include "input_events.h"

class Receiver{
public:
    Receiver(TcpSocket *sock);
    ~Receiver();
    void run();
private:
    void processMsg(struct DeviceMsg *msg);
    int processMsgs(const unsigned char *buf, size_t len);

private:
    uint64_t ack;
    TcpSocket *socket;
    bool stopped;
};

class ControllerThread : public QThread
{
    Q_OBJECT
public:
    explicit ControllerThread(TcpSocket *sock,QObject *parent = nullptr);

    virtual void run();
    bool pushMsg(ControlMsg *msg);
    void stop();
    void actionHome();
    void actionBack();
    void actionAppSwitch();
    void actionPower();
    void actionVolumeUp();
    void actionVolumeDown();
    void actionMenu();
    void pressBackOrTurnScreenOn();
    void setScreenPowerMode(bool state);
    void expandNotificationPanel();
    void expandSettingsPanel();
    void collapsePanels();
    void rotateDevice();
    bool getDeviceClipboard(CopyKey flag);
    bool setDeviceClipboard(const char *text);

    void mouseEvent(const QMouseEvent *event, const QSize &frameSize, const QSize &showSize);
    void mouseMoveEvent(const QMouseEvent *event, const QSize &frameSize, const QSize &showSize);
    void mouseWhellEvent(const QWheelEvent *event, const QSize &frameSize, const QSize &showSize);

private:
    void sendKeycode(enum android_keycode keycode,
                      enum MdcAction action, const char *name);


signals:

private:
    std::queue<ControlMsg> queue;
    QWaitCondition cond;
    QMutex mutex;
    TcpSocket *socket;
    bool stopped;
    int sequence;
};

#endif // CONTROLLER_H
