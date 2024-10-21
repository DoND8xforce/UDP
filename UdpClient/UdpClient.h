#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QObject>
#include <QUdpSocket>

class UdpClient : public QObject
{
    Q_OBJECT
public:
    explicit UdpClient(QObject *parent = nullptr);
    ~UdpClient();
    void sendMessage(const QString &message, const QHostAddress &address, quint16 port);
    void startListening(quint16 port);
private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *mUdpSocket;
};

#endif // UDPCLIENT_H
