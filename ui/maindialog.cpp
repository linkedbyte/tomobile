#include "maindialog.h"
#include "./ui_maindialog.h"
#include <QDebug>
#include <QThread>
#include <QDesktopServices>
#include <QUrl>
#include <QList>
#include <QStyleOption>
#include <QPainter>
#include <QStackedLayout>
#include "./core/config.h"
#include <QLabel>

#define TITLE_BAR_HEIGHT 40
#define LEFT_FORM_WIDTH 180
MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    this->setObjectName("MainDialog");
    this->setWindowTitle("ToMobile");
    this->setWindowIcon(QIcon(":/res/img/logo.ico"));

#ifndef Q_OS_OSX
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
#endif
    leftForm = new LeftForm(this);
    leftForm->setGeometry(0,0,LEFT_FORM_WIDTH,this->height());
    connect(leftForm,&LeftForm::closed,this,[=](){
        this->close();
    });
    connect(leftForm,&LeftForm::mined,this,[=](){
        showMinimized();
    });
    connect(leftForm,&LeftForm::click,this,[=](LeftForm::ButtonType buttonType){
        stackLayout->setCurrentIndex((int)buttonType);

    });
    titleBar = new TitleBar(this);
    titleBar->setGeometry(LEFT_FORM_WIDTH,0,this->width()-LEFT_FORM_WIDTH,TITLE_BAR_HEIGHT);


    Config::instance()->loadComm(commParams);

    deviceForm = new DeviceListForm(this);
    deviceForm->setObjectName("DeviceWidget");
    deviceForm->setGeometry(LEFT_FORM_WIDTH,TITLE_BAR_HEIGHT,this->width()-LEFT_FORM_WIDTH,this->height()-TITLE_BAR_HEIGHT);

    settingsForm = new SettingsForm(this);
    settingsForm->setGeometry(LEFT_FORM_WIDTH,TITLE_BAR_HEIGHT,this->width()-LEFT_FORM_WIDTH,this->height()-TITLE_BAR_HEIGHT);

    aboutForm = new AboutForm(this);
    aboutForm->setGeometry(LEFT_FORM_WIDTH,TITLE_BAR_HEIGHT,this->width()-LEFT_FORM_WIDTH,this->height()-TITLE_BAR_HEIGHT);

    stackLayout = new QStackedWidget(this);
    stackLayout->addWidget(deviceForm);
    stackLayout->addWidget(settingsForm);
    stackLayout->addWidget(aboutForm);
    QRect rect(LEFT_FORM_WIDTH,TITLE_BAR_HEIGHT,this->width()-LEFT_FORM_WIDTH,this->height()-TITLE_BAR_HEIGHT);
    stackLayout->setGeometry(rect);


    connect(&adbWorkder,&AdbProcess::updateDeviceVector,this,[=](std::vector<DeviceInfo> &vec){
        qDebug()<<"device count:"<<vec.size();
        leftForm->setDeviceCount(vec.size());
        if(deviceForm && !vec.empty())
        {
            deviceForm->updateDeviceList(vec);
        }
        else {
            stackLayout->setVisible(false);
        }
    });
    connect(&adbWorkder,&AdbProcess::adbState,this,[=](AdbState state){
        if(state == AdbState::ERROR_START ||
            state == AdbState::ERROR_AUTH ||
            state == AdbState::ERROR_EXEC ||
            state == AdbState::ERROR_MISSING_BINARY)
            {
            qDebug()<<"adb process failed!";
        }
    });
    adbWorkder.deviceList();
}

void MainDialog::deleteChildren(const QWidget *widget)
{
    QList<QObject*> list = widget->children();
    for(int i = 0;i<list.size();++i)
    {
        list[i]->deleteLater();
    }
}
MainDialog::~MainDialog()
{
    delete ui;
}
void MainDialog::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);

}
void MainDialog::mouseMoveEvent(QMouseEvent *event)
{
    if(titleBar->geometry().contains(event->pos()))
    {
        if(!dragPoint.isNull())
        {
            if (event->buttons() & Qt::LeftButton) {
                move(event->globalPos() - dragPoint);
            }
        }
    }
    event->accept();
}

void MainDialog::mousePressEvent(QMouseEvent *event)
{
    if(titleBar->geometry().contains(event->pos()))
    {
        if(event->button() == Qt::LeftButton)
        {
            dragPoint = event->globalPos() - frameGeometry().topLeft();
        }
    }
    event->accept();
}

void MainDialog::mouseReleaseEvent(QMouseEvent *event)
{
    dragPoint = QPoint(0,0);
}

void MainDialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

