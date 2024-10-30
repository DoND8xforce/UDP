#include "UdpServer.h"
#include <QNetworkDatagram>
#include <QDebug>
#include <QApplication>
UdpServer::UdpServer(quint16 port, QObject *parent)
    : QObject(parent)
    , mPort(port)
{
    mUdpSocket = new QUdpSocket(this);
    mUdpSocket->bind(8880);

    QString dirPath = QApplication::applicationDirPath() + "/data.bin";
    mBinFile = new BinaryFileManager(dirPath);
    //create file with 240 package
    // int index = 240;
    // while(index--){
    //     EncodeMsg(&datagram);
    //     mBinFile->Write(datagram, QIODevice::Append);
    //     datagram.clear();
    // }

    mTimer = new QTimer(this);
    mTimer->setInterval(1000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(OnSendDataHandle()));
    mTimer->start();
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpServer::OnReceiverDataHandle);
}

UdpServer::~UdpServer()
{

}

int16_t getHEnc()
{
    static int16_t h_Enc = 0;
    h_Enc += 410;
    if (h_Enc > 16384)
        h_Enc = 0;
    return h_Enc;
}
int16_t GetVEnc()
{
    static int16_t v_Enc = -8193;
    v_Enc += 410;
    if (v_Enc > 8193)
        v_Enc = 0;
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

    /*---------------------SUB3--------------------------*/
    /*---Hit---*/
    QByteArray hit1;
    uint16_t loc3 = 0x1001;
    uint8_t channelType3 = 0x01;
    uint16_t vEnc = 0x3c50;
    uint16_t doppler = 0x0123;
    uint8_t freqIndexHit = 0x01;
    uint8_t merit = 0x51;
    uint8_t enBit3 = 0x03;
    uint32_t time3 = 0x43215432;
    uint32_t power = 0x00004321;
    QDataStream streamHit1(&hit1, QDataStream::ReadWrite);
    streamHit1.setByteOrder(QDataStream::LittleEndian);
    streamHit1 << loc3 << channelType3 << vEnc << doppler << freqIndexHit << merit << enBit3 << time3
               << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << power;

    QByteArray sub3;
    QDataStream stream3(&sub3, QDataStream::ReadWrite);
    stream3.setByteOrder(QDataStream::LittleEndian);
    stream3 << STX << SEQ << SRC_ID << TYPE << len;
    stream3.writeRawData(hit1.data(), hit1.size());
    stream3.writeRawData(hit1.data(), hit1.size());
    stream3.writeRawData(hit1.data(), hit1.size());
    stream3 << checksum << ETX;
    uint16_t len3 = sub3.size();
    // qDebug() << sub3.size();
    sub3[6] = uint8_t(len3 & 0xFF);
    sub3[7] = uint8_t(len3 >> 8);
    uint16_t checksum3 = 0;
    for (int i = 0; i < len3 - 4; i++)
        checksum3 ^= sub3[i];
    sub3[len3 - 3] = uint8_t(checksum3 >> 8);
    sub3[len3 - 4] = uint8_t(checksum3 & 0xFF);
    SEQ++;
    TYPE++;

    /*---------------------SUB4--------------------------*/
    /*---Hit---*/
    QByteArray hit2;
    QDataStream streamHit2(&hit2, QDataStream::ReadWrite);
    streamHit2.setByteOrder(QDataStream::LittleEndian);
    streamHit2 << loc3 << channelType3 << vEnc << freqIndexHit << merit << enBit3 << time3
               << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << (uint8_t)0xFF << power;
    QByteArray sub4;
    QDataStream stream4(&sub4, QDataStream::ReadWrite);
    stream4.setByteOrder(QDataStream::LittleEndian);
    stream4 << STX << SEQ << SRC_ID << TYPE << len;
    stream4.writeRawData(hit2.data(), hit2.size());
    stream4.writeRawData(hit2.data(), hit2.size());
    stream4 << checksum << ETX;
    uint16_t len4 = sub4.size();
    // qDebug() << sub4.size();
    sub4[6] = uint8_t(len4 & 0xFF);
    sub4[7] = uint8_t(len4 >> 8);
    uint16_t checksum4 = 0;
    for (int i = 0; i < len4 - 4; i++)
        checksum4 ^= sub4[i];
    sub4[len4 - 3] = uint8_t(checksum4 >> 8);
    sub4[len4 - 4] = uint8_t(checksum4 & 0xFF);
    SEQ++;
    TYPE++;

    /*---------------------SUB5--------------------------*/
    QByteArray sub5;
    QDataStream stream5(&sub5, QDataStream::ReadWrite);
    stream5.setByteOrder(QDataStream::LittleEndian);
    uint32_t freq_0 = 0x00;
    uint32_t freq_1 = 0x00;
    uint32_t freq_2 = 0x01;
    uint32_t freq_3 = 0x00;
    uint32_t freq_4 = 0x01;
    uint32_t freq_5 = 0x00;
    stream5 << STX << SEQ << SRC_ID << TYPE << len << freq_0
            << freq_1 << freq_2 << freq_3 << freq_4 << freq_5 << checksum << ETX;
    uint16_t len5 = sub5.size();
    // qDebug() << sub5.size();
    sub5[6] = uint8_t(len5 & 0xFF);
    sub5[7] = uint8_t(len5 >> 8);
    uint16_t checksum5 = 0;
    for (int i = 0; i < len5 - 4; i++)
        checksum5 ^= sub5[i];
    sub5[len5 - 3] = uint8_t(checksum5 >> 8);
    sub5[len5 - 4] = uint8_t(checksum5 & 0xFF);
    SEQ++;
    TYPE++;

    /*---------------------SUB6--------------------------*/
    QByteArray sub6;
    QDataStream stream6(&sub6, QDataStream::ReadWrite);
    stream6.setByteOrder(QDataStream::LittleEndian);
    stream6 << STX << SEQ << SRC_ID << TYPE << len << (uint8_t)0x01 << checksum << ETX;
    uint16_t len6 = sub6.size();
    // qDebug() << sub6.size();
    sub6[6] = uint8_t(len6 & 0xFF);
    sub6[7] = uint8_t(len6 >> 8);
    uint16_t checksum6 = 0;
    for (int i = 0; i < len6 - 4; i++)
        checksum6 ^= sub6[i];
    sub6[len6 - 3] = uint8_t(checksum6 >> 8);
    sub6[len6 - 4] = uint8_t(checksum6 & 0xFF);


    /*----------------- Message ------------------*/
    static uint8_t package = 0;
    package++;
    QDataStream stream(message, QDataStream::ReadWrite);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << (uint8_t)0xF0 << (uint8_t)package;
    stream.writeRawData(sub1.data(), sub1.size());
    stream.writeRawData(sub2.data(), sub2.size());
    stream.writeRawData(sub3.data(), sub3.size());
    stream.writeRawData(sub4.data(), sub4.size());
    stream.writeRawData(sub5.data(), sub5.size());
    stream.writeRawData(sub6.data(), sub6.size());
    qDebug() << __FUNCTION__ << "Size Message: " << message->size();
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
    uint8_t t;
    QByteArray bytesLength;
    static int index = 0;
    static int package = 0;
    if (!mBinFile->Read(bytesLength, index, 4))
        return;
    uint8_t u0 = bytesLength[0];
    uint8_t u1 = bytesLength[1];
    uint8_t u2 = bytesLength[2];
    uint8_t u3 = bytesLength[3];
    int len = (u3 << 24) + (u2 << 16) + (u1 << 8) + u0;
    index += 4;
    if (!mBinFile->Read(datagram, index, index+len))
        return;
    t = (uint8_t)datagram[0];
    package = datagram[1];
    QByteArray ba(datagram.data() + 2, len-2);
    qDebug() << __FUNCTION__ << ba.size();
    if (mUdpSocket){
        mUdpSocket->writeDatagram(ba, QHostAddress::Any, mPort);
    }
    index += len;
    datagram.clear();
    mTimer->stop();
    mTimer->setInterval(t);
    mTimer->start();
}



