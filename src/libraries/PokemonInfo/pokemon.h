#ifndef POKEMON_H
#define POKEMON_H

namespace Pokemon {
class uniqueId;
class gen;
}
unsigned int qHash (const Pokemon::uniqueId &key);
unsigned int qHash (const Pokemon::gen &key);

#include <QString>
#include <QHash>

class DataStream;
class QTextStream;

namespace Pokemon {
class uniqueId
{
public:
    quint16 pokenum;
    quint8 subnum;
    uniqueId() : pokenum(0), subnum(0) {}
    uniqueId(int num, int subnum) : pokenum(num), subnum(subnum) {}
    uniqueId(const uniqueId &id) { pokenum = id.pokenum; subnum = id.subnum; }
    uniqueId(const QString &s) { pokenum = s.section(':', 0, 0).toInt(); subnum = s.section(':', 1, 1).toInt(); }
    uniqueId(quint32 pokeRef) {
        subnum = pokeRef >> 16;
        pokenum = pokeRef & 0xFFFF;
    }
    bool operator == (const uniqueId &other) const {
        return (pokenum == other.pokenum) && (subnum == other.subnum);
    }
    bool operator != (const uniqueId &other) const {
        return (pokenum != other.pokenum) || (subnum != other.subnum);
    }
    bool operator < (const uniqueId &other) const {
        return (pokenum < other.pokenum) || ((pokenum == other.pokenum) && (subnum < other.subnum));
    }
    bool operator > (const uniqueId &other) const {
        return (pokenum > other.pokenum) || ((pokenum == other.pokenum) && (subnum > other.subnum));
    }

    inline uniqueId original() const {
        return uniqueId(pokenum);
    }

    inline bool isForme() const {
        return subnum != 0;
    }

    QString toString() const;
    QString toLine(const QString &data) const;
    quint32 toPokeRef() const;
    // Separates pokenum:subnum:1-letter-options data from
    // the other part of a string.
    // 'data' will be modified to hold extracted data.
    // 'remaining' will be modified to hold remaining part.
    // Will return true if everything is fine. And false otherwise.
    static bool extract(const QString &raw, uniqueId &id, QString &info, QString *options = NULL);
    // Extracts short data in a "pokenum data_text" form.
    static bool extract_short(const QString &from, quint16 &pokenum, QString &remaining);
};
}

inline unsigned int qHash(const Pokemon::uniqueId &key)
{
    return qHash(key.toPokeRef());
}

DataStream & operator << (DataStream &out, const Pokemon::uniqueId &id);
DataStream & operator >> (DataStream &in, Pokemon::uniqueId &id);
QTextStream & operator >> (QTextStream &in, Pokemon::uniqueId &id);

#include <QMetaType>

Q_DECLARE_METATYPE(Pokemon::uniqueId)

#endif // POKEMON_H
