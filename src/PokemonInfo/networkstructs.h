#ifndef NETWORKSTRUCTS_H
#define NETWORKSTRUCTS_H

#include "pokemonstructs.h"
#include "../Utilities/coreclasses.h"

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

DataStream & operator << (DataStream & out,const TeamInfo & team);
DataStream & operator >> (DataStream & in,TeamInfo & team);

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

inline DataStream & operator << (DataStream &d, const UserInfo &ui) {
    d << ui.flags << ui.auth << ui.ip << ui.name << ui.date;
    return d;
}

inline DataStream & operator >> (DataStream &d, UserInfo &ui) {
    d >> ui.flags >> ui.auth >> ui.ip >> ui.name >> ui.date;
    return d;
}

DataStream & operator << (DataStream & out,const BasicInfo & team);
DataStream & operator >> (DataStream & in,BasicInfo & team);

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

DataStream & operator >> (DataStream &in, PlayerInfo &p);
DataStream & operator << (DataStream &out, const PlayerInfo &p);

struct FullInfo
{
    TrainerTeam team;

    bool ladder;
    bool showteam;
    QColor nameColor;
};

DataStream & operator >> (DataStream &in, FullInfo &p);
DataStream & operator << (DataStream &out, const FullInfo &p);


struct Battle
{
    qint32 id1, id2;

    Battle(int id1=0, int id2=0);
};

DataStream & operator >> (DataStream &in, Battle &p);
DataStream & operator << (DataStream &out, const Battle &p);

struct Flags
{
    static const int size = 4;
    /* For now no flags need more than 2 bytes. If there really needs to be a huge number of flags this
      number may increase; however for now there's no reason for dynamic allocation & what not */
    uchar data[size];

    Flags();

    bool operator [] (int index);
};

DataStream & operator >> (DataStream &in, Flags &p);
DataStream & operator << (DataStream &out, const Flags &p);

#endif // NETWORKSTRUCTS_H
