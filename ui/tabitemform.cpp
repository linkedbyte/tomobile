#include "tabitemform.h"
#include "ui_tabitemform.h"
#include <QStyleOption>
#include <QPainter>
#include <QStackedLayout>
#include <QPixmap>
#include "previewwidget.h"
#include <string>

TabItemForm::TabItemForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabItemForm)
{
    ui->setupUi(this);
    ui->videoCodecList->addItem("h264");
    ui->videoCodecList->addItem("h265");
    ui->videoCodecList->addItem("av1");

    ui->audioCodecList->addItem("opus");
    ui->audioCodecList->addItem("aac");
    ui->audioCodecList->addItem("raw");
    ui->updateDeviceNameBtn->setStyleSheet(tr("QPushButton{border: none;\
background-color: #2965fa;\
border-radius: 3px;\
color: #ffffff;\
font-family: Microsoft YaHei;\
font-weight: bold;\
padding: 3px 10px 3px 10px;}\
QPushButton:hover{color:#ffffff;background-color: #3688f5;}"));
    ui->previewWidget->setVisible(false);

    connect(ui->previewWidget,&PreviewWidget::start,this,[=](){
        ui->loadLabel->setVisible(false);
    });
    this->ui->previewWidget->setStyleSheet("background-color:#000000");
    device = NULL;
}

TabItemForm::~TabItemForm()
{
    delete ui;
}

void TabItemForm::setDevice(Device *device)
{
    if(!device)
        return;
    this->device = device;

    DeviceInfo *info = device->getDeviceInfo();
    ui->snLabel->setText(info->serial.c_str());
    ui->modelLabel->setText(info->model.c_str());
    ui->productLabel->setText(info->product.c_str());

    DeviceParams *params = device->getDeviceParams();
    ui->videoChk->setCheckState(params->video?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    ui->audioChk->setCheckState(params->audio?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    ui->controlChk->setCheckState(params->control?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    ui->powerOffOnCloseChk->setCheckState(params->powerOffOnClose?Qt::CheckState::Checked:Qt::CheckState::Unchecked);

}

std::string TabItemForm::getSerial()
{
    return device->getDeviceInfo()->serial;
}

PreviewWidget *TabItemForm::getPreviewWidget()
{
    return ui->previewWidget;
}
void TabItemForm::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
}

void TabItemForm::on_playBtn_clicked()
{
    if(device)
    {
        device->getVideoForm()->setVisible(true);
    }
}


void TabItemForm::on_updateDeviceNameBtn_clicked()
{
    std::string name = ui->deviceNameText->text().toStdString();
    if(!name.empty())
    {
        if(name.length()<10)
        {
            device->getDeviceParams()->deviceName = name;
            device->getVideoForm()->setTitle(name);
        }

    }
    else
    {
//
    }
}

