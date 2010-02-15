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

    QString name, info, win, lose;
};

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
        NonExistant = 8
    };

    qint8 flags;
    qint8 auth;
    QString ip;
    QString name;

    UserInfo (QString name = "", qint8 flags = Online, qint8 auth = 0, QString ip = "")
        : flags(flags), auth(auth), ip(ip), name(name)
    {

    }

    bool exists() const { return ! (flags & NonExistant);}
    bool online() const { return flags & Online;}
    bool banned() const { return flags & Banned;}
    bool muted() const { return flags & Muted;}
};

inline QDataStream & operator << (QDataStream &d, const UserInfo &ui) {
    d << ui.flags << ui.auth << ui.ip << ui.name;
    return d;
}

inline QDataStream & operator >> (QDataStream &d, UserInfo &ui) {
    d >> ui.flags >> ui.auth >> ui.ip >> ui.name;
    return d;
}


QDataStream & operator << (QDataStream & out,const TeamInfo & team);
QDataStream & operator >> (QDataStream & in,TeamInfo & team);

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
};

QDataStream & operator >> (QDataStream &in, PlayerInfo &p);
QDataStream & operator << (QDataStream &out, const PlayerInfo &p);

#endif // NETWORKSTRUCTS_H
