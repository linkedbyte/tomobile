#ifndef LEFTBUTTON_H
#define LEFTBUTTON_H

#include <QWidget>

namespace Ui {
class LeftButton;
}

class LeftButton : public QWidget
{
    Q_OBJECT

public:
    explicit LeftButton(QChar icon,QString text,QWidget *parent = nullptr);
    ~LeftButton();
protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
signals:
    void click(bool state);
private:
    Ui::LeftButton *ui;
    QString styleSheet;
    bool state;
};

#endif // LEFTBUTTON_H
