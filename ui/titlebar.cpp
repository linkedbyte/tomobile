#include "titlebar.h"
#include "ui_titlebar.h"
#include <QStyleOption>
#include <QPainter>
TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
    dragPosition = QPoint(0,0);
    ui->setupUi(this);

    this->ui->menuBtn->setFixedSize(16,16);
    this->ui->alertBtn->setFixedSize(20,20);
}
void TitleBar::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&opt,&painter,this);
}

TitleBar::~TitleBar()
{
    delete ui;
}

void TitleBar::setTitle(const std::string &title)
{
    this->title = title;
    this->ui->titleLabel->setText(title.c_str());
}


void TitleBar::on_menuBtn_clicked()
{

}

