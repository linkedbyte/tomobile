#ifndef VideoForm_H
#define VideoForm_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include "video_widget.h"
extern "C"
{
#include "libavutil/frame.h"
}
#include "./core/frame_buffer.h"
#include "./core/controller.h"
#include "videotitlebar.h"
#include "videobottombar.h"

namespace Ui {
class VideoForm;
}
class VideoForm : public QWidget
{
    Q_OBJECT
public:
    explicit VideoForm(QWidget *parent = nullptr);
    ~VideoForm();
    void setTitle(const std::string &title);
    void updateFrame();
    void setController(ControllerThread *handle);
    void setFrameBuffer(FrameBuffer *fb);
    void setVisible(bool visible) override;
private:
    void updateRender(int width, int height, uint8_t *data_y, uint8_t *data_u, uint8_t *data_v, int linesize_y, int linesize_u, int linesize_v);
    void updateShowSize(const QSize &newSize);
    QRect getScreenRect();
    void moveCenter();
    void showFPS(bool flag);
    void updateFPS(uint fps);
    void initShortcut();
    void switchFullScreen();
protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool isShortcut(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resize(int w, int h);
public slots:
    void showMaximized();
private:
    Ui::VideoForm *ui;
    VideoWidget *videoWidget;
    bool hasframe;
    QSize frameSize;
    float widthHeightRatio;
    QSize normalSize;
    QPoint fullScreenBeforePos;
    QPoint dragPosition;
    QLabel *fpsLabel;
    ControllerThread *controller;
    //设备屏幕是否点亮
    bool screenMode;
    VideoTitleBar *titleBar;
    VideoBottomBar *bottomBar;
    FrameBuffer *frameBuffer;
    AVFrame *lastFrame;

signals:
    void signal_close();

};

#endif // VideoForm_H
