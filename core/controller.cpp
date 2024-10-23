#include "controller.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <string>

#define CONTROL_MSG_QUEUE_MAX 64
ControllerThread::ControllerThread(TcpSocket *sock,QObject *parent)
    : QThread{parent}
{
    this->stopped = false;
    this->socket = sock;
    this->socket->moveToThread(this);
    connect(socket,&QTcpSocket::readyRead,this,[=](){
        QByteArray buf = this->socket->readAll();
        /*while (p_socket->bytesAvailable()) {
            QByteArray byteArray = p_socket->peek(p_socket->bytesAvailable());
            device_msg msg;
            int consume = device_msg::deserialize((const unsigned char*)byteArray.constData(),byteArray.length(),&msg);
            if (0 >= consume) {
                break;
            }
            p_socket->read(consume);
            //p_socket->recvDeviceMsg(&deviceMsg);
        }*/
    });
}

void ControllerThread::run()
{
    while(true)
    {
        if(this->isInterruptionRequested())
            break;
        mutex.lock();
        while(!this->stopped && queue.empty())
        {
            cond.wait(&mutex);
        }
        if(this->stopped)
        {
            mutex.unlock();
            break;
        }
        ControlMsg msg = queue.front();
        queue.pop();
        mutex.unlock();
        unsigned char serializedMsg[CONTROL_MSG_MAX_SIZE];
        size_t length = ControlMsg::serialize(&msg, serializedMsg);
        if(!length)
        {
            qDebug()<<"serialize msg failed!";
            continue;
        }
        size_t w = this->socket->write((char*)serializedMsg,length);
        this->socket->flush();
        if(w != length)
        {
            qDebug()<<"send control msg failed!";
        }      
    }
}
void ControllerThread::actionHome()
{
    enum MdcAction down = MdcAction::DOWN;
    sendKeycode(AKEYCODE_HOME, down, "HOME");
    enum MdcAction up = MdcAction::UP;
    sendKeycode(AKEYCODE_HOME, up, "HOME");
}
void ControllerThread::actionBack() {
    enum MdcAction down = MdcAction::DOWN;
    sendKeycode(AKEYCODE_BACK, down, "BACK");
    enum MdcAction up = MdcAction::UP;
    sendKeycode(AKEYCODE_BACK, up, "BACK");
//    struct ControlMsg msg;
//    msg.type = CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON;
//    msg.back_or_screen_on.action = AKEY_EVENT_ACTION_DOWN;
//    if (!pushMsg(&msg)) {
//        qDebug("Could not request actionBack");
//    }
}

void ControllerThread::actionAppSwitch() {
    enum MdcAction action = MdcAction::DOWN;
    sendKeycode(AKEYCODE_APP_SWITCH, action, "APP_SWITCH");
}

void ControllerThread::actionPower() {
    enum MdcAction action = MdcAction::DOWN;
    sendKeycode(AKEYCODE_POWER, action, "POWER");
}

void ControllerThread::actionVolumeUp() {
    enum MdcAction action = MdcAction::DOWN;
    sendKeycode(AKEYCODE_VOLUME_UP, action, "VOLUME_UP");
}

void ControllerThread::actionVolumeDown() {
    enum MdcAction action = MdcAction::DOWN;
    sendKeycode(AKEYCODE_VOLUME_DOWN, action, "VOLUME_DOWN");
}

void ControllerThread::actionMenu() {
    enum MdcAction down = MdcAction::DOWN;
    sendKeycode(AKEYCODE_MENU, down, "MENU");
    enum MdcAction up = MdcAction::UP;
    sendKeycode(AKEYCODE_MENU, up, "MENU");
}

void ControllerThread::pressBackOrTurnScreenOn()
{
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON;
    msg.backOrScreenOn.action = AKEY_EVENT_ACTION_DOWN;

    if(!pushMsg(&msg)) {
        qDebug("Could not request 'press back or turn screen on'");
    }
}
void ControllerThread::setScreenPowerMode(bool state)
{
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE;
    msg.setScreenPowerMode.mode = state ? SCREEN_POWER_MODE_NORMAL:SCREEN_POWER_MODE_OFF;

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'set screen power mode'");
    }
}
void ControllerThread::expandNotificationPanel() {
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL;

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'expand notification panel'");
    }
}
void ControllerThread::rotateDevice() {
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_ROTATE_DEVICE;

    if (!pushMsg(&msg)) {
        qDebug("Could not request device rotation");
    }
}

bool ControllerThread::getDeviceClipboard(CopyKey flag) {
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_GET_CLIPBOARD;
    msg.getClipboard.ckey = flag;

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'get device clipboard'");
        return false;
    }

    return true;
}
bool ControllerThread::setDeviceClipboard(const char *text)
{
    char *text_dup = strdup(text);

    if (!text_dup) {
        qDebug("Could not strdup input text");
        return false;
    }
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_SET_CLIPBOARD;
    msg.setClipboard.sequence = sequence;
    msg.setClipboard.text = text_dup;
    msg.setClipboard.paste = true;

    if (!pushMsg(&msg)) {
        free(text_dup);
        qDebug("Could not request 'set device clipboard'");
        return false;
    }

    return true;
}
android_motionevent_buttons convert_mouse_buttons(Qt::MouseButtons buttonState)
{
    uint32_t buttons = 0;
    if (buttonState & Qt::LeftButton) {
        buttons |= AMOTION_EVENT_BUTTON_PRIMARY;
    }
    if (buttonState & Qt::RightButton) {
        buttons |= AMOTION_EVENT_BUTTON_SECONDARY;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    if (buttonState & Qt::MiddleButton) {
#else
    if (buttonState & Qt::MidButton) {
#endif
        buttons |= AMOTION_EVENT_BUTTON_TERTIARY;
    }
    if (buttonState & Qt::XButton1) {
        buttons |= AMOTION_EVENT_BUTTON_BACK;
    }
    if (buttonState & Qt::XButton2) {
        buttons |= AMOTION_EVENT_BUTTON_FORWARD;
    }
    return (android_motionevent_buttons)buttons;
}
void ControllerThread::mouseEvent(const QMouseEvent *event, const QSize &frameSize, const QSize &showSize)
{
    android_motionevent_action action;
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        action = AMOTION_EVENT_ACTION_DOWN;
        break;
    case QEvent::MouseButtonRelease:
        action = AMOTION_EVENT_ACTION_UP;
        break;
    case QEvent::MouseMove:
        // only support left button drag
        if (!(event->buttons() & Qt::LeftButton)) {
            return;
        }
        action = AMOTION_EVENT_ACTION_MOVE;
        break;
    default:
        return;
    }

    // pos
    QPoint pos = event->pos();
    // convert pos
    pos.setX(pos.x() * frameSize.width() / showSize.width());
    pos.setY(pos.y() * frameSize.height() / showSize.height());

    // set data
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;
    msg.injectTouchEvent.action = action;
    msg.injectTouchEvent.pointerId = POINTER_ID_GENERIC_FINGER;
    msg.injectTouchEvent.position.screenSize.width = (uint16_t)frameSize.width();
    msg.injectTouchEvent.position.screenSize.height = (uint16_t)frameSize.height();
    msg.injectTouchEvent.position.point.x = pos.x();
    msg.injectTouchEvent.position.point.y = pos.y();
    msg.injectTouchEvent.pressure = action == AMOTION_EVENT_ACTION_DOWN?1.f:0.f;
    msg.injectTouchEvent.actionButton = convert_mouse_buttons(event->button());
    msg.injectTouchEvent.buttons = convert_mouse_buttons(event->buttons());

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'inject mouse click event'");
    }

}
void ControllerThread::mouseMoveEvent(const QMouseEvent *event, const QSize &frameSize, const QSize &showSize)
{
    android_motionevent_action action = AMOTION_EVENT_ACTION_MOVE;

    // pos
    QPoint pos = event->pos();
    // convert pos
    pos.setX(pos.x() * frameSize.width() / showSize.width());
    pos.setY(pos.y() * frameSize.height() / showSize.height());

    // set data
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT;
    msg.injectTouchEvent.action = action;
    msg.injectTouchEvent.pointerId = POINTER_ID_GENERIC_FINGER;
    msg.injectTouchEvent.position.screenSize.width = (uint16_t)frameSize.width();
    msg.injectTouchEvent.position.screenSize.height = (uint16_t)frameSize.height();
    msg.injectTouchEvent.position.point.x = pos.x();
    msg.injectTouchEvent.position.point.y = pos.y();
    msg.injectTouchEvent.pressure = 1.f;
    msg.injectTouchEvent.buttons = convert_mouse_buttons(event->buttons());
    msg.injectTouchEvent.actionButton = (enum android_motionevent_buttons)0;


    if (!pushMsg(&msg)) {
        qDebug("Could not request 'inject mouse click event'");
    }

}

void ControllerThread::mouseWhellEvent(const QWheelEvent *event, const QSize &frameSize, const QSize &showSize)
{
    QPoint pos = event->position().toPoint();
    // convert pos
    pos.setX(pos.x() * frameSize.width() / showSize.width());
    pos.setY(pos.y() * frameSize.height() / showSize.height());

    int32_t hscroll = event->angleDelta().x() == 0 ? 0 : event->angleDelta().x() / abs(event->angleDelta().x());
    int32_t vscroll = event->angleDelta().y() == 0 ? 0 : event->angleDelta().y() / abs(event->angleDelta().y());

    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT;
    msg.injectScrollEvent.position.point.x = pos.x();
    msg.injectScrollEvent.position.point.y = pos.y();
    msg.injectScrollEvent.position.screenSize.width = (uint16_t)frameSize.width();
    msg.injectScrollEvent.position.screenSize.height = (uint16_t)frameSize.height();
    msg.injectScrollEvent.hscroll = hscroll;
    msg.injectScrollEvent.vscroll = vscroll;
    msg.injectScrollEvent.buttons = convert_mouse_buttons(event->buttons());

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'inject mouse scroll event'");
    }
}
void ControllerThread::expandSettingsPanel() {
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL;

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'expand settings panel'");
    }
}
void ControllerThread::collapsePanels() {
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_COLLAPSE_PANELS;

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'collapse notification panel'");
    }
}


void ControllerThread::sendKeycode(enum android_keycode keycode,
             enum MdcAction action, const char *name) {
    // send DOWN event
    struct ControlMsg msg;
    msg.type = CONTROL_MSG_TYPE_INJECT_KEYCODE;
    msg.injectKeycode.action = action == MdcAction::DOWN
                                    ? AKEY_EVENT_ACTION_DOWN
                                    : AKEY_EVENT_ACTION_UP;
    msg.injectKeycode.keycode = keycode;
    msg.injectKeycode.metastate = AMETA_NONE;
    msg.injectKeycode.repeat = 0;

    if (!pushMsg(&msg)) {
        qDebug("Could not request 'inject %s'", name);
    }
}
bool ControllerThread::pushMsg(ControlMsg *msg)
{
    mutex.lock();

    if(queue.size() >= CONTROL_MSG_QUEUE_MAX)
    {
        mutex.unlock();
        return false;
    }
    bool was_empty = queue.empty();
    queue.push(*msg);
    if(was_empty)
        cond.notify_one();
    mutex.unlock();
    return true;
}

void ControllerThread::stop()
{
    mutex.lock();
    stopped = true;
    mutex.unlock();
}

Receiver::Receiver(TcpSocket *sock)
{
    this->socket = sock;
    stopped = false;

}
void Receiver::processMsg(struct DeviceMsg *msg)
{
    switch (msg->type) {
    case DEVICE_MSG_TYPE_CLIPBOARD:
    {
        QClipboard *board = QApplication::clipboard();
        std::string tmp = board->text().toStdString();
        char *current = (char*)tmp.c_str();
        bool same = current && !strcmp(current, msg->clipboard.text);

        if (same) {
            qDebug("Computer clipboard unchanged");
            return;
        }

        qDebug("Device clipboard copied");
        board->setText(msg->clipboard.text);

        break;
    }
    case DEVICE_MSG_TYPE_ACK_CLIPBOARD:
        //assert(receiver->acksync);
        qDebug("Ack device clipboard sequence=%I64u",msg->ackClipboard.sequence);
        //sc_acksync_ack(receiver->acksync, msg->ack_clipboard.sequence);
        break;
    }
}
int Receiver::processMsgs(const unsigned char *buf, size_t len)
{
    int head = 0;
    for (;;) {
        struct DeviceMsg msg;
        int r = DeviceMsg::deserialize(&buf[head], len - head, &msg);
        if (r == -1) {
            return -1;
        }
        if (r == 0) {
            return head;
        }
        processMsg(&msg);

        head += r;
        assert(head <= len);
        if (head == len) {
            return head;
        }
    }
}
void Receiver::run()
{
    char buf[DEVICE_MSG_MAX_SIZE] = {0};
    size_t head = 0;
    while(true)
    {
        if(socket->bytesAvailable()<=0)
        {
            socket->waitForReadyRead(-1);
        }
        int r = socket->read((char*)buf,sizeof(buf)-head);
        if(r <= 0)
        {
            qDebug()<<"Receiver stopped!";
            this->stopped = true;
            break;
        }
        head += r;
        int consumed = processMsgs((const unsigned char*)buf,head);
        if(consumed == -1)
            break;
        if(consumed)
        {
            head -= consumed;
            memmove(buf,&buf[consumed],head);
        }
    }
}
