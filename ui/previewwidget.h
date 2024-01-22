#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include "./core/frame_buffer.h"
namespace Ui {
class PreviewWidget;
}

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget();
    //void initSize(int w ,int h);
    void updateFrame();
    void setFrameBuffer(FrameBuffer *frameBuffer);
    void updateShowSize(QSize newSize);
protected:
    void paintEvent(QPaintEvent *event);
signals:
    void start();
private:
    Ui::PreviewWidget *ui;
    FrameBuffer *frameBuffer;
    QSize frameSize;
    bool initFlag;
};

#endif // PREVIEWWIDGET_H
