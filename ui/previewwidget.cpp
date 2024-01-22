#include "previewwidget.h"
#include "ui_previewwidget.h"
extern "C"
{
#include "libavutil/frame.h"
}
#include <QStyleOption>
#include <QPainter>
PreviewWidget::PreviewWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreviewWidget)
{
    ui->setupUi(this);
    this->frameBuffer = new FrameBuffer();
    initFlag = false;

}

PreviewWidget::~PreviewWidget()
{
    delete ui;
}
//void PreviewWidget::initSize(int w ,int h)
//{
//    frameSize.setWidth(w);
//    frameSize.setHeight(h);
//    updateShowSize();
//}

void PreviewWidget::updateFrame()
{
    AVFrame *frame = av_frame_alloc();
    this->frameBuffer->consume(frame);
    QSize newSize(frame->width,frame->height);
    updateShowSize(newSize);
    ui->videoWidget->updateTextures(frame->data[0],frame->data[1],frame->data[2],frame->linesize[0],frame->linesize[1],frame->linesize[2]);
    av_frame_free(&frame);
    if(!this->isVisible())
        this->setVisible(true);
}

void PreviewWidget::setFrameBuffer(FrameBuffer *frameBuffer)
{
    this->frameBuffer = frameBuffer;
}

void PreviewWidget::updateShowSize(QSize newSize)
{
    if(!initFlag)
    {
        initFlag = true;
        emit start();
    }
    if(frameSize != newSize)
    {
        frameSize = newSize;
        int w = 1.0f*this->height()/frameSize.height()*frameSize.width();
        int h = this->height();
        ui->videoWidget->setGeometry((this->width()-w)/2,0,w,h);
        ui->videoWidget->setFrameSize(newSize);
    }

}
void PreviewWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
}

