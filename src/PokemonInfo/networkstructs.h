#ifndef NETWORKSTRUCTS_H
#define NETWORKSTRUCTS_H

#include "pokemonstructs.h"

/* Only the infos needed by the server */
class TeamInfo
{
public:
    PokePersonal m_pokes[6];
    PokePersonal &pokemon(int num);
    const PokePersonal &pokemon(int num) const;

    QString name, info, win, lose, defaultTier;
    quint16 avatar;
    quint8 gen;
};

QDataStream & operator << (QDataStream & out,const TeamInfo & team);
QDataStream & operator >> (QDataStream & in,TeamInfo & team);

/* Only infos needed by other players */
class BasicInfo
{
public:
    QString name, info;
};

struct UserInfo
{
    enum Flags {
        Online = 1,
        Banned = 2,
        Muted = 4,
        NonExistant = 8,
        TempBanned = 16
    };

    qint8 flags;
    qint8 auth;
    QString ip;
    QString name;
    QString date;

    UserInfo (QString name = "", qint8 flags = Online, qint8 auth = 0, QString ip = "", QString date = "")
        : flags(flags), auth(auth), ip(ip), name(name), date(date)
    {

    }

    bool exists() const { return ! (flags & NonExistant);}
    bool online() const { return flags & Online;}
    bool banned() const { return flags & Banned;}
    bool muted() const { return flags & Muted;}
    bool tempBanned() const { return flags & TempBanned;}
};

inline QDataStream & operator << (QDataStream &d, const UserInfo &ui) {
    d << ui.flags << ui.auth << ui.ip << ui.name << ui.date;
    return d;
}

inline QDataStream & operator >> (QDataStream &d, UserInfo &ui) {
    d >> ui.flags >> ui.auth >> ui.ip >> ui.name >> ui.date;
    return d;
}

QDataStream & operator << (QDataStream & out,const BasicInfo & team);
QDataStream & operator >> (QDataStream & in,BasicInfo & team);

/* Struct representing a player's data */
class PlayerInfo
{
public:
    qint32 id;
    BasicInfo team;
    qint8 auth;
    quint8 flags;
    qint16 rating;
    Pokemon::uniqueId pokes[6];
    quint16 avatar;
    QString tier;
    QColor color;
    quint8 gen;

    enum {
        LoggedIn = 1,
        Battling = 2,
        Away = 4
    };

    bool battling() const {
        return flags & Battling;
    }

    bool away() const {
        return flags & Away;
    }

    void changeState(int state, bool on) {
        if (on) {
            flags |= state;
        } else {
            flags &= 0xFF ^ state;
        }
    }
};

QDataStream & operator >> (QDataStream &in, PlayerInfo &p);
QDataStream & operator << (QDataStream &out, const PlayerInfo &p);

struct FullInfo
{
    TrainerTeam team;

    bool ladder;
    bool showteam;
    QColor nameColor;
};

QDataStream & operator >> (QDataStream &in, FullInfo &p);
QDataStream & operator << (QDataStream &out, const FullInfo &p);


struct Battle
{
    qint32 id1, id2;

    Battle(int id1=0, int id2=0);
};

QDataStream & operator >> (QDataStream &in, Battle &p);
QDataStream & operator << (QDataStream &out, const Battle &p);

#endif // NETWORKSTRUCTS_H
