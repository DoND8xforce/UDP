#ifndef BINARYFILEMANAGER_H
#define BINARYFILEMANAGER_H
#include <QFile>
#include <QDataStream>
#include <QString>

class BinaryFileManager
{
public:
    BinaryFileManager(const QString &fileName);
    ~BinaryFileManager();
    void Write(QByteArray &ba, QIODeviceBase::OpenMode mode = QIODevice::Append);
    int Read(QByteArray &ba,int position, int length);
private:
    QFile *mFile;
};

#endif // BINARYFILEMANAGER_H
