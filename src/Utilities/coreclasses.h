#ifndef CORECLASSES_H
#define CORECLASSES_H

#include <QDataStream>
#include <QColor>
#include <QPair>

class DataStream : public QDataStream
{
public:
    DataStream();
    DataStream(const QByteArray &array);
    DataStream(QByteArray *array, QIODevice::OpenMode mode);
    DataStream(QIODevice *device);

    /* multiple serialization */
    void pack() {
        return;
    }

    template <typename T, typename ...Params>
    void pack(const T &item, Params&&...params) {
        *(this) << item;
        pack(std::forward<Params>(params)...);
    }

    /* As we need the DataStream return type (so that we can have a string in a multiple << expression)
      we need to overload every function */
    inline DataStream &operator>>(qint8 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(quint8 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(qint16 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(quint16 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(qint32 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(quint32 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(qint64 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(quint64 &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(bool &i) {return (DataStream&)QDataStream::operator >>(i);}
    inline DataStream &operator>>(float &f) {return (DataStream&)QDataStream::operator >>(f);}
    inline DataStream &operator>>(double &f) {return (DataStream&)QDataStream::operator >>(f);}
    inline DataStream &operator>>(char *&str) {return (DataStream&)QDataStream::operator >>(str);}

    inline DataStream &operator<<(qint8 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(quint8 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(qint16 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(quint16 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(qint32 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(quint32 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(qint64 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(quint64 i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(bool i) {return (DataStream&)QDataStream::operator <<(i);}
    inline DataStream &operator<<(float f) {return (DataStream&)QDataStream::operator <<(f);}
    inline DataStream &operator<<(double f) {return (DataStream&)QDataStream::operator <<(f);}
    inline DataStream &operator<<(const char *str) {return (DataStream&)QDataStream::operator <<(str);}
};

/* Serialize in UTF-8 */
DataStream &operator >>(DataStream &in, QString &s);
DataStream &operator <<(DataStream &out, const QString &s);
inline DataStream &operator >>(DataStream &in, QByteArray &s) { return (DataStream&) ::operator >>((QDataStream&)in, s); }
inline DataStream &operator <<(DataStream &out, const QByteArray &s) { return (DataStream&) ::operator <<((QDataStream&)out, s); }
inline DataStream &operator<<(DataStream &in, const QColor &c) { return (DataStream&) ::operator <<((QDataStream&)in, c); }
inline DataStream &operator>>(DataStream &out, QColor &c) { return (DataStream&) ::operator >>((QDataStream&)out, c); }

template <class T1, class T2>
inline DataStream& operator>>(DataStream& s, QPair<T1, T2>& p)
{
    s >> p.first >> p.second;
    return s;
}

template <class T1, class T2>
inline DataStream& operator<<(DataStream& s, const QPair<T1, T2>& p)
{
    s << p.first << p.second;
    return s;
}

template <typename T>
DataStream& operator>>(DataStream& s, QList<T>& l)
{
    l.clear();
    quint32 c;
    s >> c;
    l.reserve(c);
    for(quint32 i = 0; i < c; ++i)
    {
        T t;
        s >> t;
        l.append(t);
        if (s.atEnd())
            break;
    }
    return s;
}

template <typename T>
DataStream& operator<<(DataStream& s, const QList<T>& l)
{
    s << quint32(l.size());
    for (int i = 0; i < l.size(); ++i)
        s << l.at(i);
    return s;
}

template <typename T>
DataStream& operator>>(DataStream& s, QLinkedList<T>& l)
{
    l.clear();
    quint32 c;
    s >> c;
    for(quint32 i = 0; i < c; ++i)
    {
        T t;
        s >> t;
        l.append(t);
        if (s.atEnd())
            break;
    }
    return s;
}

template <typename T>
DataStream& operator<<(DataStream& s, const QLinkedList<T>& l)
{
    s << quint32(l.size());
    typename QLinkedList<T>::ConstIterator it = l.constBegin();
    for(; it != l.constEnd(); ++it)
        s << *it;
    return s;
}

template<typename T>
DataStream& operator>>(DataStream& s, QVector<T>& v)
{
    v.clear();
    quint32 c;
    s >> c;
    v.resize(c);
    for(quint32 i = 0; i < c; ++i) {
        T t;
        s >> t;
        v[i] = t;
    }
    return s;
}

template<typename T>
DataStream& operator<<(DataStream& s, const QVector<T>& v)
{
    s << quint32(v.size());
    for (typename QVector<T>::const_iterator it = v.begin(); it != v.end(); ++it)
        s << *it;
    return s;
}

template <typename T>
DataStream &operator>>(DataStream &in, QSet<T> &set)
{
    set.clear();
    quint32 c;
    in >> c;
    for (quint32 i = 0; i < c; ++i) {
        T t;
        in >> t;
        set << t;
        if (in.atEnd())
            break;
    }
    return in;
}

template <typename T>
DataStream& operator<<(DataStream &out, const QSet<T> &set)
{
    out << quint32(set.size());
    typename QSet<T>::const_iterator i = set.constBegin();
    while (i != set.constEnd()) {
        out << *i;
        ++i;
    }
    return out;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE DataStream &operator>>(DataStream &in, QHash<Key, T> &hash)
{
    QDataStream::Status oldStatus = in.status();
    in.resetStatus();
    hash.clear();

    quint32 n;
    in >> n;

    for (quint32 i = 0; i < n; ++i) {
        if (in.status() != QDataStream::Ok)
            break;

        Key k;
        T t;
        in >> k >> t;
        hash.insertMulti(k, t);
    }

    if (in.status() != QDataStream::Ok)
        hash.clear();
    if (oldStatus != QDataStream::Ok)
        in.setStatus(oldStatus);
    return in;
}

template <class Key, class T>
Q_OUTOFLINE_TEMPLATE DataStream &operator<<(DataStream &out, const QHash<Key, T>& hash)
{
    out << quint32(hash.size());
    typename QHash<Key, T>::ConstIterator it = hash.end();
    typename QHash<Key, T>::ConstIterator begin = hash.begin();
    while (it != begin) {
        --it;
        out << it.key() << it.value();
    }
    return out;
}

#endif // CORECLASSES_H
