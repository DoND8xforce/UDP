#include "UdpClient.h"
#include <QDebug>

UdpClient::UdpClient(QObject *parent)
    : QObject{parent}
{
    mUdpSocket = new QUdpSocket{this};
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpClient::processPendingDatagrams);
}

UdpClient::~UdpClient()
{
    delete mUdpSocket;
}

void UdpClient::sendMessage(const QString &message, const QHostAddress &address, quint16 port)
{
    QByteArray datagram = message.toUtf8();
    mUdpSocket->writeDatagram(datagram, address, port);
}

void UdpClient::startListening(quint16 port)
{
    mUdpSocket->bind(QHostAddress("127.0.0.1"), port);
}

void UdpClient::processPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mUdpSocket->pendingDatagramSize());

        mUdpSocket->readDatagram(datagram.data(), datagram.size());
        qDebug() << __FUNCTION__ << ":" << datagram.toHex();
    }
}
