#ifndef TOOLSFORM_H
#define TOOLSFORM_H

#include <QWidget>
#include <QPushButton>
#include "controller.h"
namespace Ui {
class ToolsForm;
}
enum ToolsActive
{
    FULLSCREEN,
    CLOSE,
};

class ToolsForm : public QWidget
{
    Q_OBJECT

public:
    explicit ToolsForm(QWidget *parent = nullptr);
    ~ToolsForm();
    void setController(controller *handle);

private slots:
    void on_fullscreenBtn_clicked();

    void on_vhswitchBtn_clicked();

    void on_closeBtn_clicked();
signals:
    void signal_tools_active(ToolsActive active);
private:
    Ui::ToolsForm *ui;
    controller *p_controller;
};

#endif // TOOLSFORM_H
