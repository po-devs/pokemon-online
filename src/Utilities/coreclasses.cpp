#include "coreclasses.h"

DataStream::DataStream()
{
    setVersion(QDataStream::Qt_4_7);
}

DataStream::DataStream(const QByteArray &array) : QDataStream(array)
{
    setVersion(QDataStream::Qt_4_7);
}

DataStream::DataStream(QByteArray *array, QIODevice::OpenMode mode) : QDataStream(array, mode)
{
    setVersion(QDataStream::Qt_4_7);
}

DataStream::DataStream(QIODevice *device) : QDataStream(device)
{
    setVersion(QDataStream::Qt_4_7);
}

DataStream &operator >> (DataStream& in, QString &s)
{
    QByteArray x;
    in >> x;
    s = QString::fromUtf8(x);

    return in;
}

DataStream &operator << (DataStream &out, const QString &s)
{
    out << s.toUtf8();

    return out;
}
