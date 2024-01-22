#include "leftbutton.h"
#include "ui_leftbutton.h"
#include "iconfont.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>

LeftButton::LeftButton(QChar icon,QString text,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LeftButton)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Hover);
    IconFont::instance()->setFont(this->ui->icon,icon);
    this->ui->label->setText(text);
    styleSheet = "QWidget{color:#565656;font-size:14px;font-weight:300;}";
    styleSheet += "QWidget:hover{background-color:#e0e9f5;border:none;border-radius: 6px;color:#1164f2;}";
    //m_style += "QWidget{background-color:#c4d8f4}";
    this->setStyleSheet(styleSheet);
    state = false;
}
void LeftButton::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
}

void LeftButton::mousePressEvent(QMouseEvent *event)
{
    state = !state;
    emit click(state);
    event->accept();
}

LeftButton::~LeftButton()
{
    delete ui;
}
