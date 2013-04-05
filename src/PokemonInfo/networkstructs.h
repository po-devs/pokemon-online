#ifndef NETWORKSTRUCTS_H
#define NETWORKSTRUCTS_H

#include "pokemonstructs.h"
#include "../Utilities/coreclasses.h"
#include "../Shared/config.h"

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

namespace PlayerFlags {
    enum {
        SupportsZipCompression,
        LadderEnabled,
        IdsWithMessage,
        Idle
    };
    enum {
        NoReconnectData,
        WrongHash,
        NoHistory,
        IPMismatch
    };
}

/* Struct representing a player's data */
class PlayerInfo
{
public:
    qint32 id;
    QString name, info;
    qint8 auth;
    Flags flags;
    quint16 avatar;
    QColor color;

    QHash<QString, quint16> ratings;

    PlayerInfo() {
        avatar = id = auth = 0;
    }

    enum {
        Away = 0,
        LadderEnabled=1,
        Battling=2
    };

    bool battling() const {
        return flags[Battling];
    }

    bool away() const {
        return flags[Away];
    }

    void changeState(int state, bool on) {
        flags.setFlag(state, on);
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

struct ProtocolVersion
{
    quint16 version;
    quint16 subversion;

    bool operator < (const ProtocolVersion &other) const
    {return version < other.version || (version == other.version && subversion < other.subversion);}

    ProtocolVersion(int version=PROTOCOL_VERSION, int subversion=PROTOCOL_SUBVERSION);
};

DataStream & operator >> (DataStream &in, ProtocolVersion &p);
DataStream & operator << (DataStream &out, const ProtocolVersion &p);

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

    void sanitize();
};

DataStream & operator >> (DataStream &in, TrainerInfo &i);
DataStream & operator << (DataStream &out, const TrainerInfo &i);

class PersonalTeam
{
    PROPERTY(QString, defaultTier);
    PROPERTY(Pokemon::gen, gen);
protected:
    PokePersonal m_pokes[6];

public:
    PersonalTeam();

    const PokePersonal & poke(int index) const {return m_pokes[index];}
    PokePersonal & poke(int index) {return m_pokes[index];}
};

DataStream & operator >> (DataStream & in, PersonalTeam & team);

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
    QList<PersonalTeam> *teams;
    QString *channel;
    QStringList *additionalChannels;
    TrainerInfo *trainerInfo;
    QStringList *plugins;
};

struct ChangeTeamInfo
{
    ChangeTeamInfo();

    QString *name;
    QColor *color;
    QList<PersonalTeam> *teams;
    PersonalTeam *team;
    quint8 teamNum;
    TrainerInfo *info;
};

DataStream & operator >> (DataStream & in, LoginInfo & team);

struct ServerInfo
{
    ServerInfo();

    QString name;
    QString desc;
    quint16 num;
    QString ip;
    quint16 max;
    quint16 port;
    bool passwordProtected;
};

DataStream & operator >> (DataStream & in, ServerInfo & info);

#endif // NETWORKSTRUCTS_H
