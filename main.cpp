#include "./ui/maindialog.h"

#include <QApplication>
#include <QFile>
#include <QtDebug>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
static QString logFileName = QString("%1.log").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
static QMutex logMutex;
void logMessageOutput(QtMsgType type,const QMessageLogContext &context,const QString &msg)
{
    QString logLine;
    QString logHeader = QString("%1:%2:%3:%4-").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
                                                   context.file,context.function,QString::number(context.line));
    switch (type)
    {
    case QtDebugMsg:
        logLine = "[DEBUG]"+logLine + msg;
        break;
    case QtWarningMsg:
        logLine = "[WARNING]"+logLine + msg;
        break;
    case QtCriticalMsg:
        logLine = "[CRITICAL]"+logLine + msg;
        break;
    case QtInfoMsg:
        logLine = "[INFO]"+logLine + msg;
        break;
    case QtFatalMsg:
        logLine = "[FATAL]"+logLine + msg;
        abort();
        break;
    default:
        break;
    }
    QMutexLocker locker(&logMutex);
    QFile outFile(logFileName);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream textStream(&outFile);
    textStream << logLine;
    outFile.close();
}
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
    qInstallMessageHandler(logMessageOutput);
    MainDialog w;
    w.show();

    return a.exec();
}
