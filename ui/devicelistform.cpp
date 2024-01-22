#include "devicelistform.h"
#include "tabitemform.h"
#include "ui_devicelistform.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include "./core/device.h"
#include "./core/config.h"
DeviceListForm::DeviceListForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceListForm)
{
    ui->setupUi(this);
    tabWidget = new QTabWidget(this);
    tabWidget->setGeometry(0,0,this->width(),this->height());

    nodeviceLabel = new QLabel(this);
    int w,h;
    w = this->width();
    h = this->height();
    QPixmap image;
    image.load(":/res/img/no_device.png");
    nodeviceLabel->setPixmap(image);
    nodeviceLabel->setGeometry((w-image.width())/2,(h-image.height())/2,image.width(),image.height());
}

DeviceListForm::~DeviceListForm()
{
    delete ui;
}
TabItemForm *DeviceListForm::findTabItem(const std::string &serial)
{
    for(int t = 0;t<tabWidget->count();++t)
    {
        TabItemForm *item = (TabItemForm*)tabWidget->widget(t);
        if(item->getSerial() == serial)
        {
            return item;
        }
    }
    return NULL;
}
void DeviceListForm::updateDeviceList(std::vector<DeviceInfo> &list)
{
    for(int i=0;i<list.size();++i)
    {
        DeviceInfo it = list[i];
        Device *device = DeviceManager::instance()->getDevice(it.serial);
        TabItemForm *tabItem = this->findTabItem(device->getSerial());
        if(it.state == "offline")
        {
            if(tabItem){
                int index = this->tabWidget->indexOf(tabItem);
            }
            if(device)
                DeviceManager::instance()->deleteDevice(it.serial);
        }
        else if(it.state == "device")
        {
            if(tabItem)
                break;
            if(!device)
            {
                DeviceParams deviceParams;
                bool ret = Config::instance()->loadDevice(it.serial,deviceParams);
                if(!ret)
                {
                    CommParams commParams;
                    Config::instance()->loadComm(commParams);
                    deviceParams = commParams;
                }
                deviceParams.serial = it.serial;
                device = new Device(deviceParams,it);
                Config::instance()->updateDevice(deviceParams);
                DeviceManager::instance()->addDevice(device);
            }
            tabItem = new TabItemForm(tabWidget);
            tabItem->setDevice(device);
            tabItem->setGeometry(0,0,tabWidget->width(),tabWidget->height());
            tabWidget->addTab(tabItem,it.model.c_str());
            tabWidget->setTabIcon(tabWidget->count()-1,QIcon(":/res/img/android.png"));
            nodeviceLabel->setVisible(false);
            device->setPreviewWidget(tabItem->getPreviewWidget());
            device->usbConnect();
        }
        else
        {
            //other states
        }
    }
}
//bool DeviceListForm::addItem(device_info &item)
//{
//    QListWidgetItem *listItem = new QListWidgetItem(ui->deviceList);
//    //listItem->setText(item.serial.c_str());
//    listItem->setSizeHint(QSize(ITEM_WIDTH,ITEM_HEIGHT));
//    //listItem->setTextAlignment(Qt::AlignCenter);
//    listItem->setData(Qt::UserRole,item.serial.c_str());
//    ui->deviceList->addItem(listItem);
//    DeviceListItem *itemWidget = new DeviceListItem(item,ui->deviceList);
//    ui->deviceList->setItemWidget(listItem,itemWidget);
//}
