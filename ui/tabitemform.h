#ifndef TABITEMFORM_H
#define TABITEMFORM_H

#include <QWidget>
#include "previewwidget.h"
#include "./core/device.h"
namespace Ui {
class TabItemForm;
}

class TabItemForm : public QWidget
{
    Q_OBJECT

public:
    explicit TabItemForm(QWidget *parent = nullptr);
    ~TabItemForm();
    void setDevice(Device *device);
    std::string getSerial();
    PreviewWidget *getPreviewWidget();
protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void on_playBtn_clicked();

    void on_updateDeviceNameBtn_clicked();

private:
    Ui::TabItemForm *ui;
    Device *device;
};

#endif // TABITEMFORM_H
