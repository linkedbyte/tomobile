#include "videobottombar.h"
#include "ui_videobottombar.h"
#include <QResizeEvent>
#include "iconfont.h"
#define BUTTON_WIDTH 50
#define BUTTON_HEIGHT 25
#define SPACE 10
#define ICON_SIZE 18
VideoBottomBar::VideoBottomBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoBottomBar)
{
    ui->setupUi(this);
    this->controller = NULL;

    this->ui->homeBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->homeBtn,QChar(0xf015),ICON_SIZE);
    connect(this->ui->homeBtn,&QPushButton::clicked,this,[=](){
        emit barAction(VideoBottomBarAction::HOME);
    });

    this->ui->backBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->backBtn,QChar(0xf100),ICON_SIZE);
    connect(this->ui->backBtn,&QPushButton::clicked,this,[=](){
        emit barAction(VideoBottomBarAction::BACK);
    });

    this->ui->menuBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
    IconFont::instance()->setFont(this->ui->menuBtn,QChar(0xf0c9),ICON_SIZE);
    connect(this->ui->menuBtn,&QPushButton::clicked,this,[=](){
        emit barAction(VideoBottomBarAction::MENU);
    });

    this->setStyleSheet("QPushButton:hover{color:#ffffff;}");
}

VideoBottomBar::~VideoBottomBar()
{
    delete ui;
}
void VideoBottomBar::resizeEvent(QResizeEvent *event)
{
    int x = this->width()/2 - BUTTON_WIDTH/2;
    int y = 8;
    this->ui->homeBtn->move(x,y);
    this->ui->backBtn->move(x-BUTTON_WIDTH-SPACE,y);
    this->ui->menuBtn->move(x+BUTTON_WIDTH+SPACE,y);
    event->accept();
}
