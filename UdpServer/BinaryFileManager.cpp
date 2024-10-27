#include "BinaryFileManager.h"

BinaryFileManager::BinaryFileManager(const QString &fileName)
{
    mFile = new QFile(fileName);
}

void BinaryFileManager::Write(QByteArray &ba, QIODevice::OpenMode mode)
{
    if (!mFile->open(mode))
        return;
    QDataStream stream(mFile);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << ba;
    mFile->close();
}

int BinaryFileManager::Read(QByteArray &ba, int position, int length)
{
    if (!mFile->open(QIODevice::ReadOnly))
        return 0;
    if (!mFile->seek(position))
        return 0;
    ba = mFile->read(length);
    mFile->close();
    if (length < ba.size())
        return 0;
    return ba.size();
}
