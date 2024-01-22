#include "leftform.h"
#include "ui_leftform.h"
#include <QStyleOption>
#include <QPainter>
#include "iconfont.h"
#include <QLabel>
#include <QPushButton>
#include <QDebug>
#include <QDesktopServices>
#include <QUrl>
#include "leftbutton.h"
LeftForm::LeftForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeftForm)
{
    ui->setupUi(this);

    int x,y,w,h,fontSize,spacer;
    x = 6;
    y = 6;
    w = 20;
    h = 20;
    fontSize = 10;
    spacer = 0;

    QPushButton *closeBtn = new QPushButton(this);
    IconFont::instance()->setFont(closeBtn,QChar(0xf057),fontSize);
    closeBtn->setGeometry(x,y,w,h);
    closeBtn->setStyleSheet("color: #E15045;border:none");
    connect(closeBtn,&QPushButton::clicked,this,[=](){
        emit closed();
    });

    QPushButton *minBtn = new QPushButton(this);
    IconFont::instance()->setFont(minBtn,QChar(0xf056),fontSize);
    x+=w;
    x+=spacer;
    minBtn->setGeometry(x,y,w,h);
    minBtn->setStyleSheet("color: #ECAE31;border:none");
    connect(minBtn,&QPushButton::clicked,this,[=](){
        emit mined();
    });

    QPushButton *dotBtn = new QPushButton(this);
    IconFont::instance()->setFont(dotBtn,QChar(0xf111),fontSize);
    x+=w;
    x+=spacer;
    dotBtn->setGeometry(x,y,w,h);
    dotBtn->setStyleSheet("color: #D6D6D6;border:none");

    QLabel *verLabel = new QLabel(this);
    x+=w;
    x+=70;
    verLabel->setGeometry(x,y,60,20);
    verLabel->setText("V1.0.1");
    verLabel->setStyleSheet("color:#5d6579;");

    int x1,y1,w1,h1,s1;
    x1 = 15;
    y1 = 50;
    w1 = 150;
    h1 = 36;
    s1 = 6;
    LeftButton *deviceBtn = new LeftButton(QChar(0xf10a),QString("设备列表"),this);
    deviceBtn->setGeometry(x1,y1,w1,h1);
    //device count lab
    deviceCountLab = new QLabel(this);
    deviceCountLab->setGeometry(deviceBtn->width()-30+x1,y1+10,12,12);
    deviceCountLab->setAlignment(Qt::AlignCenter);
    deviceCountLab->setStyleSheet("background-color:#E15045;color:#ffffff;font-size:8px;border-radius: 6px");
    deviceCountLab->setVisible(false);
    connect(deviceBtn,&LeftButton::click,this,[=](bool state){
        emit click(ButtonType::DEVICE);
    });

    y1+=h1;
    y1+=s1;
    LeftButton *settingsBtn = new LeftButton(QChar(0xf013),QString("高级设置"),this);
    settingsBtn->setGeometry(x1,y1,w1,h1);
    connect(settingsBtn,&LeftButton::click,this,[=](bool state){
        emit click(ButtonType::SETTINGS);
    });


    y1+=h1;
    y1+=s1;
    LeftButton *aboutBtn = new LeftButton(QChar(0xf007),QString("About"),this);
    aboutBtn->setGeometry(x1,y1,w1,h1);
    connect(aboutBtn,&LeftButton::click,this,[=](bool state){
        emit click(ButtonType::ABOUT);
    });

    y1+=h1;
    y1+=s1;
    QPushButton *logoBtn = new QPushButton(this);
    logoBtn->setStyleSheet("border-image:url(:/res/img/logo.png);");

    logoBtn->setGeometry(16,this->height()-40-20,150,36);
    connect(logoBtn,&QPushButton::clicked,this,[=](bool state){
        QUrl url("http://www.linkedbyte.com");
        QDesktopServices::openUrl(url);
    });
}

LeftForm::~LeftForm()
{
    delete ui;
}

void LeftForm::setDeviceCount(int count)
{
    this->deviceCountLab->setText(QString::number(count));
    deviceCountLab->setVisible(true);
}
void LeftForm::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
}
