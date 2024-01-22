#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <vector>
#include <QPoint>
#include "./core/common.h"
#include "titlebar.h"
#include "./core/adbprocess.h"
#include "leftform.h"
#include "settingsform.h"
#include "devicelistform.h"
#include "aboutform.h"
#include <QStackedWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    void displayDeviceList();
    void deleteChildren(const QWidget *widget);
private:
    Ui::MainDialog *ui;
    std::vector<DeviceInfo> deviceVector;
    AdbProcess adbWorkder;

    TitleBar *titleBar;
    QPoint dragPoint;
    QStackedWidget *stackLayout;
    SettingsForm *settingsForm;
    DeviceListForm *deviceForm;
    AboutForm *aboutForm;
    LeftForm *leftForm;
    CommParams commParams;
};
#endif // MAINDIALOG_H
