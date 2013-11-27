#ifndef CORECLASSES_H
#define CORECLASSES_H

#include <QDataStream>
#include <QPair>
#include <QStringList>
#include <QList>
#include <map>

class DataStream : public QDataStream
{
public:
    DataStream();
    DataStream(const QByteArray &array, quint16 version=0);
    DataStream(QByteArray *array, QIODevice::OpenMode mode, quint16 version=0);
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

    /* Can be set to reflect Protocol's major version and change how things are serialized/deserialized */
    quint16 version;
};

/* Serialize in UTF-8 */
DataStream &operator >>(DataStream &in, QString &s);
DataStream &operator <<(DataStream &out, const QString &s);
inline DataStream &operator >>(DataStream &in, QByteArray &s) { return (DataStream&) ::operator >>((QDataStream&)in, s); }
inline DataStream &operator <<(DataStream &out, const QByteArray &s) { return (DataStream&) ::operator <<((QDataStream&)out, s); }
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

inline DataStream &operator>>(DataStream &in, QStringList &list)
{
    return operator>>(in, static_cast<QList<QString> &>(list));
}
inline DataStream &operator<<(DataStream &out, const QStringList &list)
{
    return operator<<(out, static_cast<const QList<QString> &>(list));
}


/* Flags are like so: for each byte, 7 bits of flag and one bit to tell if there are higher flags (in network)
  so as to limit the number of bytes sent by networking. That's why you should never have a flag that's 7,
    15, 23, etc. because it'd possibly mess the networking */
struct Flags
{
    /* For now no flags need more than 2 bytes. If there really needs to be a huge number of flags this
      number may increase; however for now there's no reason for dynamic allocation & what not */
    quint32 data;

    Flags(quint32 data=0);

    bool operator [] (int index) const;
    void setFlag(int index, bool value);
    void setFlags(quint32 flags);
};

DataStream & operator >> (DataStream &in, Flags &p);
DataStream & operator << (DataStream &out, const Flags &p);


struct VersionControl
{
    VersionControl(quint8 versionNumber=0);

    QByteArray data;
    DataStream stream;
    quint8 versionNumber;
};

DataStream & operator >> (DataStream &in, VersionControl &v);
DataStream & operator << (DataStream &out, const VersionControl &v);

/* Way it works: Refreshes a value, if necessary, when it is needed.

  And gives the value.*/
template <class T, class Lambda>
struct Cache
{
    Cache(Lambda f) {
        upToDate = false;
        converter = f;
    }

    const T& value() const {
        if (!updated()) {
            converter(m_value);
            upToDate = true;
        }
        return m_value;
    }

    void outdate() const {upToDate = false;}
    bool updated()const {return upToDate;}

    mutable T m_value;
    mutable bool upToDate;
    Lambda converter;

    operator T() const {
        return m_value;
    }
};

template <class T>
class reference
{
public:
    reference(const T *val=0) : mRef(val) {}

    const T *mRef;
};

template <class T>
DataStream &operator<<(DataStream &out, const reference<T> &ref)
{
    out << (*ref.mRef);

    return out;
}

/* Serializes a container without count param */
template <class T>
class Expander
{
public:
    Expander(const T& ref) : mRef(ref){}

    const T& mRef;
};

template <class T>
DataStream &operator<<(DataStream &out, const Expander<T> &list)
{
    auto it = list.mRef.begin();

    while (it != list.mRef.end()) {
        out << *it;
        ++it;
    }

    return out;
}

struct icompare{
    bool operator ()(const QString &a, const QString &b) {
        return a.compare(b, Qt::CaseInsensitive) < 0;
    }
};

/* Map with case insensitive key binding */
#ifdef QT5
template <class ValType>
using istringmap = std::map<QString, ValType, icompare>;
#else
template <class ValType>
class istringmap : public std::map<QString, ValType, icompare>{};
#endif
#endif // CORECLASSES_H
