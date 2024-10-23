#include "tcpsocket.h"

TcpSocket::TcpSocket(QObject *parent)
    : QTcpSocket{parent}
{

}

int TcpSocket::recv(char *buf, int len)
{
    while (bytesAvailable() < len) {
        if (!waitForReadyRead(-1)) {
            return 0;
        }
    }
    return this->read(buf,len);
}
