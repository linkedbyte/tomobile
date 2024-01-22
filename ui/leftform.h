#ifndef LEFTFORM_H
#define LEFTFORM_H

#include <QWidget>

#include <QLabel>

namespace Ui {
class LeftForm;
}

class LeftForm : public QWidget
{
    Q_OBJECT

public:
    explicit LeftForm(QWidget *parent = nullptr);
    ~LeftForm();
    enum ButtonType{
        DEVICE = 0,
        SETTINGS,
        ABOUT,
    };
    void setDeviceCount(int count);

signals:
    void mined();
    void closed();
    void click(ButtonType buttonType);
protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::LeftForm *ui;
    QLabel *deviceCountLab;
};

#endif // LEFTFORM_H
