#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

class TcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = nullptr);
    virtual int recv(char *buf,int len);

signals:

};

#endif // TCPSOCKET_H
