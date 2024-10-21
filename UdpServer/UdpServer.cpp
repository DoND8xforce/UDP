#include "UdpServer.h"
#include <QNetworkDatagram>
#include <QDebug>

UdpServer::UdpServer(quint16 port, QObject *parent)
    : QObject(parent)
    , mPort(port)
{
    mUdpSocket = new QUdpSocket(this);
    mUdpSocket->bind(QHostAddress::LocalHost, 1234);
    // connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpServer::OnReceiverDataHandle);
    mTimer = new QTimer(this);
    mTimer->setInterval(1000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(OnSendDataHandle()));
    mTimer->start();
}

UdpServer::~UdpServer()
{

}

uint16_t getHEnc()
{
    uint16_t v = (1 << 14) - 1;
    uint16_t angle = (v/360);
    static uint16_t count = 0;
    uint16_t h_Enc = 0;
    if(count == 360)
        count = 1;
    h_Enc = angle * count;
    count ++;

    return h_Enc;
}
uint16_t GetVEnc()
{
    int16_t min_value = -(1 << 13);
    int16_t max_value = (1 << 13) -1;
    uint16_t angle = (max_value - min_value +1)/(180-(-180) + 1);
    static int16_t count = -180;
    if(count == 180)
        count = -180;
    int16_t v_Enc = angle * count;
    count++;

    return v_Enc;
}

void UdpServer::EncodeMsg(QByteArray *message)
{
    uint16_t STX = 0x0D0D;
    uint8_t SRC_ID = 0x02;
    uint16_t ETX = 0x0A0A;
    uint16_t len = 0x0000;
    uint16_t SEQ = 0;
    uint8_t TYPE = 0;
    uint16_t checksum = 0x000;

    /*----------------SUB1: Encoder------------------------------*/
    QByteArray sub1;
    uint16_t horizonalEnc = 0x0102;
    uint16_t verticalEnc = 0x0304;
    static uint32_t time = 0;
    time++;
    uint8_t enBit1 = 14;

    QDataStream stream1(&sub1, QDataStream::ReadWrite);
    stream1.setByteOrder(QDataStream::LittleEndian);
    stream1 << STX << SEQ << SRC_ID << TYPE << len << horizonalEnc << verticalEnc << time << enBit1<< (uint8_t)0xFF
            << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << checksum << ETX;
    uint16_t len1 = sub1.size();
    // qDebug() << sub1.size();
    sub1[6] = uint8_t(len1 & 0xFF);
    sub1[7] = uint8_t(len1 >> 8);
    uint16_t checksum1 = 0;
    for (int i = 0; i < len1 - 4; i++)
        checksum1 ^= sub1[i];
    sub1[len1 - 3] = uint8_t(checksum1 >> 8);
    sub1[len1 - 4] = uint8_t(checksum1 & 0xFF);
    SEQ++;
    TYPE++;

    /*---------------------SUB2: Video--------------------------*/
    QByteArray sub2;
    uint16_t loc = 0x1001;
    uint16_t numPixel = 0x0002;
    static uint8_t channelType = 0x00;
    channelType = !channelType;
    uint8_t compressRate = 0x05;
    uint8_t enBit2 = 14;
    int16_t hOrV_Enc = 0;
    if(channelType) {
        hOrV_Enc =  GetVEnc();
    }
    else {
        hOrV_Enc = getHEnc();
    }
    uint8_t freqIndex = 0x01;
    uint32_t resverved = 0xFFFFFFFF;
    uint32_t vData1 = 0x0f0f0f0f;
    uint32_t vData2 = 0x01010101;
    QDataStream stream2(&sub2, QDataStream::ReadWrite);
    stream2.setByteOrder(QDataStream::LittleEndian);
    stream2 << STX << SEQ << SRC_ID << TYPE << len << loc << numPixel << channelType << compressRate
            << hOrV_Enc << enBit2 << freqIndex << time << resverved << vData1 << vData2 << checksum << ETX;
    uint16_t len2 = sub2.size();
    // qDebug() << sub2.size();
    sub2[6] = uint8_t(len2 & 0xFF);
    sub2[7] = uint8_t(len2 >> 8);
    uint16_t checksum2 = 0;
    for (int i = 0; i < len2 - 4; i++)
        checksum2 ^= sub2[i];
    sub2[len2 - 3] = uint8_t(checksum2 >> 8);
    sub2[len2 - 4] = uint8_t(checksum2 & 0xFF);
    SEQ++;
    TYPE++;

    /*----------------- Message ------------------*/
    QDataStream stream(message, QDataStream::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(sub1.data(), sub1.size());
    stream.writeRawData(sub2.data(), sub2.size());
}

void UdpServer::OnReceiverDataHandle()
{
    while (mUdpSocket->hasPendingDatagrams()) {
        QNetworkDatagram dataGram = mUdpSocket->receiveDatagram();
        qDebug() << __FUNCTION__ << dataGram.data();
    }
}

void UdpServer::OnSendDataHandle()
{
    QByteArray datagram;
    EncodeMsg(&datagram);
    qDebug() << __FUNCTION__ << "Size Message: " << datagram.size();

    if (mUdpSocket)
        mUdpSocket->writeDatagram(datagram, QHostAddress::LocalHost, 8889);
}



