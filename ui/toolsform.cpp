#include "toolsform.h"
#include "ui_toolsform.h"

ToolsForm::ToolsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ToolsForm)
{
    ui->setupUi(this);
#ifndef Q_OS_OSX
    //Qt::WindowStaysOnTopHint
    //setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    //setAttribute(Qt::WA_TranslucentBackground);
#endif
}

ToolsForm::~ToolsForm()
{
    delete ui;
}

void ToolsForm::setController(controller *handle)
{
    this->p_controller = handle;
}

void ToolsForm::on_fullscreenBtn_clicked()
{
    emit signal_tools_active(ToolsActive::FULLSCREEN);
}


void ToolsForm::on_vhswitchBtn_clicked()
{

}


void ToolsForm::on_closeBtn_clicked()
{
    emit signal_tools_active(ToolsActive::CLOSE);
}

