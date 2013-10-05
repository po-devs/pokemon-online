#include "pokemon.h"
#include "geninfo.h"
#include "../Utilities/coreclasses.h"
#include <QTextStream>

QString Pokemon::uniqueId::toString() const
{
    QString result = QString("%1").arg(pokenum);
    if(subnum) result.append(QString("-%1").arg(subnum));
    return result;
}

QString Pokemon::uniqueId::toLine(const QString &data) const
{
    return QString ("%1:%2 %3").arg(pokenum).arg(subnum).arg(data);
}

quint32 Pokemon::uniqueId::toPokeRef() const
{
    return pokenum + (subnum << 16);
}

/* Extracts the pokenum part and data from a line of text. The pokenum part will be put in the "id" field, the
   content of the line (without the index) in the "lineData" field. If options are given in the index, they'll
   be put in the "options" field. */
bool Pokemon::uniqueId::extract(const QString &from, Pokemon::uniqueId &id, QString &lineData, QString *options)
{
    if (from.isEmpty() || from.indexOf(' ') == -1)
        return false;

   // ":" delimeter for values. pokenum:subnum:1-letter-options
    QStringList values = from.section(' ', 0, 0).split(':');

    if (values.size() < 2)
        return false;

    bool ok, ok2;
    uint num = values[0].toUInt(&ok);
    uint sub = values[1].toUInt(&ok2);

    if (!ok || !ok2)
        return false;

    lineData = from.section(' ', 1);
    id.pokenum = num;
    id.subnum = sub;

    // optional: options
    if (options) {
        options->clear();
        if(values.size() > 2)
            *options = values[2];
    }

    return true;
}

bool Pokemon::uniqueId::extract_short(const QString &from, quint16 &pokenum, QString &remaining)
{
    bool result = false;
    if(!from.isEmpty()) {
        int space_pos = from.indexOf(' '); // 1 space delimeter (first)
        if(space_pos != -1 && space_pos + 1 < from.length()) {
            QString other_data = from.mid(space_pos + 1);
            QString text_pokenum = from.left(space_pos);
            bool ok;
            quint16 read_pokenum = text_pokenum.toUInt(&ok);
            if(ok) {
                pokenum = read_pokenum;
                remaining = other_data;
                result = true;
            }
        } // if space_pos
    } // if !from
    return result;
}

/* Extracts the pokenum part and data from a line of text. The pokenum part will be put in the "id" field, the
   content of the line (without the index) in the "lineData" field. If options are given in the index, they'll
   be put in the "options" field. */
bool Pokemon::gen::extract(const QString &from, Pokemon::gen &id, QString &lineData)
{
    if (from.isEmpty() || from.indexOf(' ') == -1)
        return false;

   // ":" delimeter for values. pokenum:subnum:1-letter-options
    QStringList values = from.section(' ', 0, 0).split(':');

    if (values.size() < 2)
        return false;

    bool ok, ok2;
    uint num = values[0].toUInt(&ok);
    uint sub = values[1].toUInt(&ok2);

    if (!ok || !ok2)
        return false;

    lineData = from.section(' ', 1);
    id.num = num;
    id.subnum = sub;

    return true;
}

bool Pokemon::gen::extract_short(const QString &from, quint8 &gen, QString &remaining)
{
    bool result = false;
    if(!from.isEmpty()) {
        int space_pos = from.indexOf(' '); // 1 space delimeter (first)
        if(space_pos != -1 && space_pos + 1 < from.length()) {
            QString other_data = from.mid(space_pos + 1);
            QString text_pokenum = from.left(space_pos);
            bool ok;
            quint16 read_pokenum = text_pokenum.toUInt(&ok);
            if(ok) {
                gen = read_pokenum;
                remaining = other_data;
                result = true;
            }
        } // if space_pos
    } // if !from
    return result;
}

DataStream & operator << (DataStream &out, const Pokemon::uniqueId &id)
{
    out << id.pokenum;
    out << id.subnum;
    return out;
}

DataStream & operator >> (DataStream &in, Pokemon::uniqueId &id)
{
    in >> id.pokenum;
    in >> id.subnum;
    return in;
}

QTextStream & operator >> (QTextStream &in, Pokemon::uniqueId &id)
{
    QString s;
    in >> s;

    if (s.contains(":")) {
        id = Pokemon::uniqueId(s.section(":", 0, 0).toInt(), s.section(":", 1, 1).toInt());
    } else {
        id = Pokemon::uniqueId(s.toInt());
    }

    return in;
}

DataStream & operator << (DataStream &out, const Pokemon::gen &id)
{
    out << id.num;
    out << id.subnum;
    return out;
}

DataStream & operator >> (DataStream &in, Pokemon::gen &id)
{
    in >> id.num;
    in >> id.subnum;

    if (GenInfo::GenMax() > 0) {
        if (id.num < GenInfo::GenMin()) {
            id.num = GenInfo::GenMin();
        } else if (id.num > GenInfo::GenMax()) {
            id.num = GenInfo::GenMax();
        }

        if (id.subnum >= GenInfo::NumberOfSubgens(id.num)) {
            id.subnum = GenInfo::NumberOfSubgens(id.num) - 1;
        }
    }

    return in;
}

