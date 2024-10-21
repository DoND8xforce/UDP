#include <QCoreApplication>
#include "UdpServer.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    UdpServer udpServer(8889);

    return a.exec();
}
