#include <QCoreApplication>
#include "UdpClient.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    UdpClient udpClient;
    udpClient.startListening(8889);

    return a.exec();
}
