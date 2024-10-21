#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>

class UdpServer : public QObject
{
    Q_OBJECT
public:
    UdpServer(quint16 port, QObject *parent = nullptr);
    ~UdpServer();
    void startListening(quint16 port);
    void EncodeMsg(QByteArray *message);
private slots:
    void OnReceiverDataHandle();
    void OnSendDataHandle();
private:
    QUdpSocket * mUdpSocket;
    QTimer *mTimer;
    quint16 mPort;
};

#endif // UDPSERVER_H
