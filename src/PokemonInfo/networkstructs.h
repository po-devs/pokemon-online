#ifndef NETWORKSTRUCTS_H
#define NETWORKSTRUCTS_H

#include "pokemonstructs.h"
#include "../Utilities/coreclasses.h"

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

struct Battle
{
    qint32 id1, id2;

    Battle(int id1=0, int id2=0);
};

DataStream & operator >> (DataStream &in, Battle &p);
DataStream & operator << (DataStream &out, const Battle &p);

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

namespace PlayerFlags {
    enum {
        SupportsZipCompression,
        LadderEnabled,
        IdsWithMessage,
        Idle
    };
}

struct ProtocolVersion
{
    quint16 version;
    quint16 subversion;

    ProtocolVersion();
};

DataStream & operator >> (DataStream &in, ProtocolVersion &p);
DataStream & operator << (DataStream &out, const ProtocolVersion &p);

struct VersionControl
{
    VersionControl(quint8 versionNumber=0);

    QByteArray data;
    DataStream stream;
    quint8 versionNumber;
};

DataStream & operator >> (DataStream &in, VersionControl &v);
DataStream & operator << (DataStream &out, const VersionControl &v);

struct TrainerInfo
{
    static const quint8 version = 0;
    enum Flags {
        HasWinningMessages
    };
    TrainerInfo();

    quint16 avatar;
    QString info;
    QString winning, losing, tie;
};

DataStream & operator >> (DataStream &in, TrainerInfo &i);
DataStream & operator << (DataStream &out, const TrainerInfo &i);

/* Only the infos needed by the server */
struct LoginInfo
{
    LoginInfo();
    ~LoginInfo();

    ProtocolVersion version;
    Flags network, data, events;
    QString clientType;
    quint16 clientVersion;
    QString trainerName;
    quint8 reconnectBits;
    QColor trainerColor;
    QList<Team> *teams;
    QString *channel;
    QStringList *additionalChannels;
    TrainerInfo *trainerInfo;
    QStringList *plugins;
};

DataStream & operator >> (DataStream & in, LoginInfo & team);

#endif // NETWORKSTRUCTS_H
