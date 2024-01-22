#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <string>
namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();
    void setTitle(const std::string &title);
protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void on_menuBtn_clicked();
private:
    Ui::TitleBar *ui;
    std::string title;
    QPoint dragPosition;

};

#endif // TITLEBAR_H
