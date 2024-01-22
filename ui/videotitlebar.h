#ifndef VIDEOTITLEBAR_H
#define VIDEOTITLEBAR_H

#include <QWidget>
#include "./core/controller.h"
#include <string>
namespace Ui {
class VideoTitleBar;
}
enum VideoTitleBarAction
{
    MASTER = 0,
    RECODER,
    VHSHOW,
    COPY,
    PASTE,
    FULLSCREEN,
    MAX,
    MIN,
    CLOSE,
};

class VideoTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit VideoTitleBar(QWidget *parent = nullptr);
    ~VideoTitleBar();
    //void resize(int w,int h);
    virtual void resizeEvent(QResizeEvent *event);
    void setController(ControllerThread *handle);
    void setDeviceName(const std::string &name);
signals:
    void barAction(VideoTitleBarAction action);

private:
    Ui::VideoTitleBar *ui;
    std::string deviceName;
    ControllerThread *controller;

};

#endif // VIDEOTITLEBAR_H
