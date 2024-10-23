#include "control_msg.h"
#include "binary.h"
#include <string.h>
#include <QDebug>
static void
write_position(uint8_t *buf, const struct TMPosition *position) {
    write32be(&buf[0], position->point.x);
    write32be(&buf[4], position->point.y);
    write16be(&buf[8], position->screenSize.width);
    write16be(&buf[10], position->screenSize.height);
}
size_t str_utf8_truncation_index(const char *utf8, size_t max_len) {
    size_t len = strlen(utf8);
    if (len <= max_len) {
        return len;
    }
    len = max_len;
    // see UTF-8 encoding <https://en.wikipedia.org/wiki/UTF-8#Description>
    while ((utf8[len] & 0x80) != 0 && (utf8[len] & 0xc0) != 0xc0) {
        // the next byte is not the start of a new UTF-8 codepoint
        // so if we would cut there, the character would be truncated
        len--;
    }
    return len;
}
// write length (4 bytes) + string (non null-terminated)
static size_t
write_string(const char *utf8, size_t max_len, unsigned char *buf) {
    size_t len = str_utf8_truncation_index(utf8, max_len);
    write32be(buf, len);
    memcpy(&buf[4], utf8, len);
    return 4 + len;
}
ssize_t ControlMsg::serialize(const struct ControlMsg *msg, unsigned char *buf) {
    buf[0] = msg->type;
    switch (msg->type) {
    case CONTROL_MSG_TYPE_INJECT_KEYCODE:
        buf[1] = msg->injectKeycode.action;
        write32be(&buf[2], msg->injectKeycode.keycode);
        write32be(&buf[6], msg->injectKeycode.repeat);
        write32be(&buf[10], msg->injectKeycode.metastate);
        return 14;
    case CONTROL_MSG_TYPE_INJECT_TEXT: {
        size_t len =
            write_string(msg->injectText.text,
                         CONTROL_MSG_INJECT_TEXT_MAX_LENGTH, &buf[1]);
        return 1 + len;
    }
    case CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT: {
        buf[1] = msg->injectTouchEvent.action;
        write64be(&buf[2], msg->injectTouchEvent.pointerId);
        write_position(&buf[10], &msg->injectTouchEvent.position);
        uint16_t pressure = float_to_u16fp(msg->injectTouchEvent.pressure);
        write16be(&buf[22], pressure);
        write32be(&buf[24], msg->injectTouchEvent.actionButton);
        write32be(&buf[28], msg->injectTouchEvent.buttons);
        return 32;
    }
    case CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT:{
        write_position(&buf[1], &msg->injectScrollEvent.position);
        int16_t hscroll =
            float_to_i16fp(msg->injectScrollEvent.hscroll);
        int16_t vscroll =
            float_to_i16fp(msg->injectScrollEvent.vscroll);
        write16be(&buf[13], (uint16_t) hscroll);
        write16be(&buf[15], (uint16_t) vscroll);
        write32be(&buf[17], msg->injectScrollEvent.buttons);
        return 21;
    }
    case CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON:
        buf[1] = msg->injectKeycode.action;
        return 2;
    case CONTROL_MSG_TYPE_GET_CLIPBOARD:
        buf[1] = msg->getClipboard.ckey;
        return 2;
    case CONTROL_MSG_TYPE_SET_CLIPBOARD:
    {
        write64be(&buf[1], msg->setClipboard.sequence);
        buf[9] = !!msg->setClipboard.paste;
        size_t len = write_string(msg->setClipboard.text,
                                  CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH,
                                  &buf[10]);
        return 10 + len;
    }
    case CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE:
        buf[1] = msg->setScreenPowerMode.mode;
        return 2;
    case CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL:
    case CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL:
    case CONTROL_MSG_TYPE_COLLAPSE_PANELS:
    case CONTROL_MSG_TYPE_ROTATE_DEVICE:
        // no additional data
        return 1;
    default:
        qDebug()<<"Unknown message type: "<<(unsigned) msg->type;
        return 0;
    }
}

ssize_t DeviceMsg::deserialize(const unsigned char *buf, size_t len, DeviceMsg *msg)
{
    if (len < 5) {
        // at least type + empty string length
        return 0; // not available
    }

    msg->type = (enum DeviceMsgType)buf[0];
    switch (msg->type) {
    case DEVICE_MSG_TYPE_CLIPBOARD: {
        size_t clipboardLen = read32be(&buf[1]);
        if (clipboardLen > len - 5) {
            return 0; // not available
        }
        char *text = (char*)malloc(clipboardLen + 1);
        if (!text) {
            //LOG_OOM();
            return -1;
        }
        if (clipboardLen) {
            memcpy(text, &buf[5], clipboardLen);
        }
        text[clipboardLen] = '\0';

        msg->clipboard.text = text;
        return 5 + clipboardLen;
    }
    case DEVICE_MSG_TYPE_ACK_CLIPBOARD: {
        uint64_t sequence = read64be(&buf[1]);
        msg->ackClipboard.sequence = sequence;
        return 9;
    }
    default:
        qDebug("Unknown device message type: %d", (int) msg->type);
        return -1; // error, we cannot recover
    }
}
