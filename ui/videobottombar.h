#ifndef VIDEOBOTTOMBAR_H
#define VIDEOBOTTOMBAR_H

#include <QWidget>
#include "./core/controller.h"
namespace Ui {
class VideoBottomBar;
}
enum VideoBottomBarAction
{
    HOME = 0,
    BACK,
    MENU,
};
class VideoBottomBar : public QWidget
{
    Q_OBJECT

public:
    explicit VideoBottomBar(QWidget *parent = nullptr);
    ~VideoBottomBar();
    void resizeEvent(QResizeEvent *event);
signals:
    void barAction(VideoBottomBarAction action);
private:
    Ui::VideoBottomBar *ui;
    ControllerThread *controller;
};

#endif // VIDEOBOTTOMBAR_H
