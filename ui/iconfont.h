#ifndef ICONFONT_H
#define ICONFONT_H
#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QMutex>
#include <QObject>
#include <QPushButton>

class IconFont
{
public:
    IconFont();
    static IconFont *instance();

    void setFont(QLabel *label,QChar c,int fontSize = 12);
    void setFont(QPushButton *button,QChar c,int fontSize = 12);
private:
    static IconFont *_instance;
    QFont m_font;
};

#endif // ICONFONT_H
