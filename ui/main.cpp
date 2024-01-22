#include "maindialog.h"

#include <QApplication>
#include <QFile>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qssFile(":/res/qss/main.css");
    if(qssFile.open(QFile::ReadOnly))
    {
        QString style = QLatin1String(qssFile.readAll());
        a.setStyleSheet(style);
        qssFile.close();
    }
    MainDialog w;
    w.show();

    return a.exec();
}
