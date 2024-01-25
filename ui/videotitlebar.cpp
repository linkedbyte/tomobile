#include "videotitlebar.h"
#include "ui_videotitlebar.h"
#include "iconfont.h"
#include <QResizeEvent>

#define BUTTON_WIDTH 20
#define BUTTON_HEIGHT 20
#define SPACE 3
#define TOP_PADDING 8
#define ICON_SIZE 10
VideoTitleBar::VideoTitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoTitleBar)
{
    ui->setupUi(this);

    isMax = true;

    /*this->ui->masterBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->masterBtn,QChar(0xf500),ICON_SIZE);
    this->ui->masterBtn->setToolTip("群主");
    connect(this->ui->masterBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::MASTER);
    });*/

    this->ui->recoderBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->recoderBtn,QChar(0xf8d9),ICON_SIZE);
    this->ui->recoderBtn->setToolTip("录像");
    connect(this->ui->recoderBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::RECODER);
    });

    this->ui->vhshowBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->vhshowBtn,QChar(0xf07d),ICON_SIZE);
    this->ui->vhshowBtn->setToolTip("竖屏");
    connect(this->ui->recoderBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::VHSHOW);
    });

    this->ui->copyBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->copyBtn,QChar(0xf1f9),ICON_SIZE);
    this->ui->copyBtn->setToolTip("复制");
    connect(this->ui->copyBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::COPY);
    });

    this->ui->pasteBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->pasteBtn,QChar(0xf0ea),ICON_SIZE);
    this->ui->pasteBtn->setToolTip("粘贴");
    connect(this->ui->pasteBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::PASTE);
    });

    this->ui->fullScreenBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->fullScreenBtn,QChar(0xf78c),ICON_SIZE);
    this->ui->fullScreenBtn->setToolTip("全屏");
    connect(this->ui->fullScreenBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::FULLSCREEN);
    });

    this->ui->maxBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->maxBtn,QChar(0xf2d0),ICON_SIZE);
    this->ui->maxBtn->setToolTip("最大化");
    connect(this->ui->maxBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::MAX);
    });

    this->ui->minBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->minBtn,QChar(0xf068),ICON_SIZE);
    this->ui->minBtn->setToolTip("最小化");
    connect(this->ui->minBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::MIN);
    });

    this->ui->closeBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->closeBtn,QChar(0xf00d),ICON_SIZE);
    this->ui->closeBtn->setToolTip("关闭");
    connect(this->ui->closeBtn,&QPushButton::clicked,this,[=](bool checked){
        emit barAction(VideoTitleBarAction::CLOSE);
    });
    this->setStyleSheet(tr("QPushButton:hover{color:#ffffff;}"
                           "QToolTip{color:#ffffff;}"));
    this->ui->deviceNameLabel->setStyleSheet("color:#ffffff;");
}

VideoTitleBar::~VideoTitleBar()
{
    delete ui;
}

void VideoTitleBar::resizeEvent(QResizeEvent *event)
{
    int x = this->ui->deviceNameLabel->width();
    int y = TOP_PADDING;

    /*x+=SPACE;
    this->ui->masterBtn->move(x,y);
    x+=BUTTON_WIDTH;*/

    x+=SPACE;
    this->ui->recoderBtn->move(x,y);
    x+=BUTTON_WIDTH;
    x+=SPACE;
    this->ui->vhshowBtn->move(x,y);

    x+=BUTTON_WIDTH;
    x+=SPACE;
    this->ui->copyBtn->move(x,y);

    x+=BUTTON_WIDTH;
    x+=SPACE;
    this->ui->pasteBtn->move(x,y);

    int rx = this->width();
    int ry = TOP_PADDING;

    rx -= SPACE;
    rx -= BUTTON_WIDTH;
    this->ui->closeBtn->move(rx,ry);

    rx -= SPACE;
    rx -= BUTTON_WIDTH;
    this->ui->minBtn->move(rx,ry);

    rx -= SPACE;
    rx -= BUTTON_WIDTH;
    this->ui->maxBtn->move(rx,ry);

    rx -= SPACE;
    rx -= BUTTON_WIDTH;
    this->ui->fullScreenBtn->move(rx,ry);

    event->accept();
}

void VideoTitleBar::setController(ControllerThread *handle)
{
    this->controller = handle;
}

void VideoTitleBar::setDeviceName(const std::string &name)
{
    this->deviceName = name;
    this->ui->deviceNameLabel->setText(this->deviceName.c_str());
}

void VideoTitleBar::buttonActionIcon(VideoTitleBarAction action)
{

}
