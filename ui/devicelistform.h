#ifndef DEVICELISTFORM_H
#define DEVICELISTFORM_H

#include <QWidget>
#include <string>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QLabel>
#include "core/common.h"
#include "tabitemform.h"

namespace Ui {
class DeviceListForm;
}

class DeviceListForm : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceListForm(QWidget *parent = nullptr);
    ~DeviceListForm();
    void updateDeviceList(std::vector<DeviceInfo> &list);
private:
    TabItemForm *findTabItem(const std::string &serial);
//    bool addItem(device_info &item);
//    void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    Ui::DeviceListForm *ui;
    QTabWidget *tabWidget;
    QLabel *nodeviceLabel;

};

#endif // DEVICELISTFORM_H
