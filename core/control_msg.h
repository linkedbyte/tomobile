#ifndef CONTROL_MSG_H
#define CONTROL_MSG_H
#include "android/input.h"
#include "android/keycodes.h"
#include <stdint.h>
#define CONTROL_MSG_MAX_SIZE (1 << 18) // 256k

#define CONTROL_MSG_INJECT_TEXT_MAX_LENGTH 300
// type: 1 byte; sequence: 8 bytes; paste flag: 1 byte; length: 4 bytes
#define CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH (CONTROL_MSG_MAX_SIZE - 14)

#define POINTER_ID_MOUSE UINT64_C(-1)
#define POINTER_ID_GENERIC_FINGER UINT64_C(-2)

// Used for injecting an additional virtual pointer for pinch-to-zoom
#define POINTER_ID_VIRTUAL_MOUSE UINT64_C(-3)
#define POINTER_ID_VIRTUAL_FINGER UINT64_C(-4)

enum ControlMsgType {
    CONTROL_MSG_TYPE_INJECT_KEYCODE,
    CONTROL_MSG_TYPE_INJECT_TEXT,
    CONTROL_MSG_TYPE_INJECT_TOUCH_EVENT,
    CONTROL_MSG_TYPE_INJECT_SCROLL_EVENT,
    CONTROL_MSG_TYPE_BACK_OR_SCREEN_ON,
    CONTROL_MSG_TYPE_EXPAND_NOTIFICATION_PANEL,
    CONTROL_MSG_TYPE_EXPAND_SETTINGS_PANEL,
    CONTROL_MSG_TYPE_COLLAPSE_PANELS,
    CONTROL_MSG_TYPE_GET_CLIPBOARD,
    CONTROL_MSG_TYPE_SET_CLIPBOARD,
    CONTROL_MSG_TYPE_SET_SCREEN_POWER_MODE,
    CONTROL_MSG_TYPE_ROTATE_DEVICE,
};

enum ScreenPowerMode {
    // see <https://android.googlesource.com/platform/frameworks/base.git/+/pie-release-2/core/java/android/view/SurfaceControl.java#305>
    SCREEN_POWER_MODE_OFF = 0,
    SCREEN_POWER_MODE_NORMAL = 2,
};

enum CopyKey {
    COPY_KEY_NONE,
    COPY_KEY_COPY,
    COPY_KEY_CUT,
};
struct TMSize {
    uint16_t width;
    uint16_t height;
};

struct TMPoint {
    int32_t x;
    int32_t y;
};

struct TMPosition {
    struct TMSize screenSize;
    struct TMPoint point;
};
struct ControlMsg {
    enum ControlMsgType type;
    union {
        struct {
            enum android_keyevent_action action;
            enum android_keycode keycode;
            uint32_t repeat;
            enum android_metastate metastate;
        } injectKeycode;
        struct {
            char *text; // owned, to be freed by free()
        } injectText;
        struct {
            enum android_motionevent_action action;
            enum android_motionevent_buttons actionButton;
            enum android_motionevent_buttons buttons;
            uint64_t pointerId;
            struct TMPosition position;
            float pressure;
        } injectTouchEvent;
        struct {
            struct TMPosition position;
            float hscroll;
            float vscroll;
            enum android_motionevent_buttons buttons;
        } injectScrollEvent;
        struct {
            enum android_keyevent_action action; // action for the BACK key
            // screen may only be turned on on ACTION_DOWN
        } backOrScreenOn;
        struct {
            enum CopyKey ckey;
        } getClipboard;
        struct {
            uint64_t sequence;
            char *text; // owned, to be freed by free()
            bool paste;
        } setClipboard;
        struct {
            enum ScreenPowerMode mode;
        } setScreenPowerMode;
    };
    static ssize_t serialize(const struct ControlMsg *msg, unsigned char *buf);

};

#define DEVICE_MSG_MAX_SIZE (1 << 18) // 256k
// type: 1 byte; length: 4 bytes
#define DEVICE_MSG_TEXT_MAX_LENGTH (DEVICE_MSG_MAX_SIZE - 5)

enum DeviceMsgType {
    DEVICE_MSG_TYPE_CLIPBOARD,
    DEVICE_MSG_TYPE_ACK_CLIPBOARD,
};

struct DeviceMsg {
    enum DeviceMsgType type;
    union {
        struct {
            char *text; // owned, to be freed by free()
        } clipboard;
        struct {
            uint64_t sequence;
        } ackClipboard;
    };
    static int deserialize(const unsigned char *buf, size_t len,struct DeviceMsg *msg);

};
#endif // CONTROL_MSG_H
