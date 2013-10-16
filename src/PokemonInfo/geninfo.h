#ifndef GENINFO_H
#define GENINFO_H

namespace Pokemon {
class uniqueId;
class gen;
}
unsigned int qHash (const Pokemon::uniqueId &key);
unsigned int qHash (const Pokemon::gen &key);

#include <QString>
#include <QHash>

namespace Pokemon {
class gen;
}

class DataStream;

class GenInfo
{
public:
    static void init(const QString &dir="db/gens/");
    static void retranslate();

    static QString Gen(int gen);
    static QString Version(const Pokemon::gen &gen);

    static int GenMin() { return genMin; }
    static int GenMax() { return genMax; }
    static int NumberOfGens();
    static int NumberOfSubgens(int gen);

    static QList<int> AllGens();
    static QList<Pokemon::gen> AllSubGens();
private:
    static QString m_Directory;
    static QHash<int, QString> m_gens;
    static QHash<Pokemon::gen, QString> m_versions;

    static QHash<int, int> m_NumberOfSubgens;
    static int genMin, genMax;

    static QString path(const QString &filename);
};

namespace Pokemon {

class gen
{
public:
    quint8 num;
    quint8 subnum;
    gen() : num(GenInfo::GenMax()), subnum(GenInfo::NumberOfSubgens(GenInfo::GenMax())-1) {}
    gen(int num, int subnum) : num(num), subnum(subnum) {}
    gen(const gen &g) { num = g.num; subnum = g.subnum; }
    gen(quint32 genRef) {
        subnum = genRef >> 8;
        num = genRef & 0xFF;
    }
    inline bool operator == (const gen &other) const {
        return (num == other.num) && (subnum == other.subnum);
    }
    inline bool operator != (const gen &other) const {
        return (num != other.num) || (subnum != other.subnum);
    }
    inline bool operator < (const gen &other) const {
        return (num < other.num) || ((num == other.num) && (subnum < other.subnum));
    }
    inline bool operator <= (const gen &other) const {
        return (num <= other.num) || ((num == other.num) && (subnum <= other.subnum));
    }
    inline bool operator > (const gen &other) const {
        return (num > other.num) || ((num == other.num) && (subnum > other.subnum));
    }
    inline bool operator >= (const gen &other) const {
        return (num >= other.num) || ((num == other.num) && (subnum >= other.subnum));
    }
    inline bool operator < (int other) const {
        return (num < other);
    }
    inline bool operator > (int other) const {
        return (num > other);
    }
    inline bool operator <= (int other) const {
        return (num <= other);
    }
    inline bool operator >= (int other) const {
        return (num >= other);
    }

    inline QString toString() const {
        return QString("%1-%2").arg(int(num)).arg(int(subnum));
    }

    inline Pokemon::gen original() {return gen(num, 0);}

    // Will return true if everything is fine. And false otherwise.
    static bool extract(const QString &raw, gen &id, QString &info);
    // Extracts short data in a "pokenum data_text" form.
    static bool extract_short(const QString &from, quint8 &gen, QString &remaining);

    /* gen(i, wholeGen) is a gen that contains the union of the infos of all the subgens */
    static const decltype(subnum) wholeGen = -1;
};

}

inline unsigned int qHash(const Pokemon::gen &gen)
{
    return qHash(gen.num + (gen.subnum << 8));
}

DataStream & operator << (DataStream &out, const Pokemon::gen &g);
DataStream & operator >> (DataStream &in, Pokemon::gen &g);

#include <QMetaType>

Q_DECLARE_METATYPE(Pokemon::gen)

#endif // GENINFO_H
