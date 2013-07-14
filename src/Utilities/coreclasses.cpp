#include "coreclasses.h"

#include <QColor>

DataStream::DataStream() : version(0)
{
    setVersion(QDataStream::Qt_4_8);
}

DataStream::DataStream(const QByteArray &array, quint16 version) : QDataStream(array), version(version)
{
    setVersion(QDataStream::Qt_4_8);
}

DataStream::DataStream(QByteArray *array, QIODevice::OpenMode mode, quint16 version) : QDataStream(array, mode), version(version)
{
    setVersion(QDataStream::Qt_4_8);
}

DataStream::DataStream(QIODevice *device) : QDataStream(device), version(0)
{
    setVersion(QDataStream::Qt_4_8);
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


Flags::Flags(quint32 data) : data(data) {
}

bool Flags::operator [](int index) const {
    return data & (1 << index);
}

void Flags::setFlag(int index, bool value)
{
    if (value) {
        data|= (1 << index);
    } else {
        data &=  ~(1 << index);
    }
}

void Flags::setFlags(quint32 flags)
{
    data = flags;
}

DataStream &operator << (DataStream &out, const Flags &f)
{
    for (int i = 0; i==0 || f.data>>(i*8); i++) {
        quint8 c = f.data >> (i*8);
        if (f.data >> ((i+1)*8)) {
            c |= 1 << 7;
        }
        out << c;
    }

    return out;
}

DataStream &operator >> (DataStream &in, Flags &f) {
    f.data = 0;

    quint8 c(0);

    for (int i = 0; i == 0 || c & (1 << 7); i++) {
        in >> c;
        f.data |= c << (i * 8);
        f.data &= ~(1 << (i*8+7)); /* Remove marker bit if present */
    }

    return in;
}


VersionControl::VersionControl(quint8 versionNumber) : stream(&data, QIODevice::ReadWrite), versionNumber(versionNumber)
{

}

DataStream & operator >> (DataStream &in, VersionControl &v)
{
    quint16 length;
    in >> length;

    v.data.resize(length);
    in.readRawData(v.data.data(), length);
    v.stream >> v.versionNumber;

    return in;
}

DataStream & operator << (DataStream &out, const VersionControl &v)
{
    out << quint16(v.data.length() + 1);
    out << v.versionNumber;
    out.writeRawData(v.data, v.data.length());

    return out;
}

DataStream &operator<<(DataStream &in, const QColor &c) {
    return (DataStream&)::operator <<((QDataStream&)in, c);
}

DataStream &operator>>(DataStream &out, QColor &c) {
    return (DataStream&)::operator >>((QDataStream&)out, c);
}
