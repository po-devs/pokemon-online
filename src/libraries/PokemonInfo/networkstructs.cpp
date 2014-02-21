#include "networkstructs.h"
#include <Utilities/coreclasses.h>
#include "../Shared/config.h"
#include "../Shared/networkcommands.h"

DataStream & operator >> (DataStream &in, PlayerInfo &p)
{
    VersionControl v;
    in >> v;

    if (v.versionNumber != 0) {
        return in;
    }

    Flags network;
    v.stream >> p.id >> network >> p.flags >> p.name >> p.color >> p.avatar >> p.info >> p.auth;

    qint8 numTiers;

    v.stream >> numTiers;

    p.ratings.clear();
    for (int i = 0; i < numTiers; i++) {
        QString tier;
        quint16 rating;
        v.stream >> tier >> rating;

        p.ratings.insert(tier, rating);
    }

    return in;
}

DataStream & operator << (DataStream &out, const PlayerInfo &p)
{
    VersionControl v;

    v.stream << p.id << Flags(0) << p.flags << p.name << p.color << p.avatar << p.info << p.auth << qint8(p.ratings.size());

    QHashIterator<QString, quint16> it(p.ratings);

    while (it.hasNext()) {
        it.next();
        v.stream << it.key() << it.value();
    }

    out << v;

    return out;
}

Battle::Battle(int id1, int id2, int mode, const QString &tier) : id1(id1), id2(id2), mode(mode), tier(tier)
{

}

DataStream & operator >> (DataStream &in, Battle &p)
{
    //in >> p.battleid >> p.id1 >> p.id2;
    if (in.version <= 1) {
        in >> p.id1 >> p.id2;
    } else {
        Flags flags;
        in >> flags >> p.mode >> p.id1 >> p.id2;
        if (flags[0]) {
            in >> p.tier;
        }
    }

    return in;
}

DataStream & operator << (DataStream &out, const Battle &p)
{
    //out << p.battleid << p.id1 << p.id2;
    if (out.version <= 1) {
        out << p.id1 << p.id2;
    } else {
        out << Flags(p.tier.length() > 0) << p.mode << p.id1 << p.id2;
        if (p.tier.length() > 0) {
            out << p.tier;
        }
    }

    return out;
}

ProtocolVersion::ProtocolVersion(int v, int s) : version(v), subversion(s)
{
}

DataStream &operator >> (DataStream &in, ProtocolVersion &p)
{
    in >> p.version >> p.subversion;
    return in;
}

DataStream &operator << (DataStream &out, const ProtocolVersion &p)
{
    out << p.version << p.subversion;
    return out;
}

TrainerInfo::TrainerInfo() : avatar(0) {
}

DataStream & operator >> (DataStream &in, TrainerInfo &i)
{
    VersionControl v;
    in >> v;

    if (i.version != v.versionNumber) {
        return in;
    }

    Flags network;
    v.stream >> network >> i.avatar >> i.info;

    if (network[TrainerInfo::HasWinningMessages]) {
        v.stream >> i.winning >> i.losing >> i.tie;
    }

    i.sanitize();

    return in;
}

void TrainerInfo::sanitize()
{
    if (info.length() > 300) {
        info.resize(300);
    }
    if (winning.length() > 200) {
        winning.resize(200);
    }
    if (losing.length() > 200) {
        losing.resize(200);
    }
    if (tie.length() > 200) {
        tie.resize(200);
    }
}

DataStream & operator << (DataStream &out, const TrainerInfo &i)
{
    VersionControl v(i.version);
    Flags network;

    bool messages = !i.winning.isEmpty() || !i.losing.isEmpty() || !i.tie.isEmpty();

    network.setFlag(TrainerInfo::HasWinningMessages, messages);

    v.stream << network << i.avatar << i.info;
    if (messages) {
        v.stream << i.winning << i.losing << i.tie;
    }

    out << v;
    return out;
}

DataStream & operator >> (DataStream & in, PersonalTeam & team)
{
    VersionControl v;
    in >> v;

    if (v.versionNumber != 0) {
        return in;
    }

    Flags network;
    v.stream >> network;

    if (network[0]) {
        QString s;
        v.stream >> s;
        team.defaultTier() = s;
    }

    v.stream >> team.gen();

    for (int i = 0; i < 6; i++) {
        team.poke(i).gen() = team.gen();
    }

    quint8 count = 6;

    if (network[1]) {
        v.stream >> count;
    }

    for(int i=0;i<count;i++)
    {
        v.stream >> team.poke(i);
    }

    /* In case the sender overrode the gen parameter in the individual pokemons */
    for (int i = 0; i < 6; i++) {
        team.poke(i).gen() = team.gen();
    }

    return in;
}

LoginInfo::LoginInfo() : teams(0), channel(0), additionalChannels(0), trainerInfo(0), plugins(0), cookie(0)
{
}

LoginInfo::~LoginInfo()
{
    delete teams, delete channel, delete additionalChannels, delete trainerInfo, delete plugins; delete cookie;
}

DataStream & operator >> (DataStream &in, LoginInfo &l)
{
//                HasClientType = 0,
//                HasVersionNumber,
//                HasReconnect,
//                HasDefaultChannel,
//                HasAdditionalChannels,
//                HasColor,
//                HasTrainerInfo,
//                /* markerbit = 7 */
//                HasTeams = 8,
//                HasEventSpecification,
//                HasPluginList
    l.clientVersion = 0;
    in >> l.version >> l.network;

#define test(variable, flag) if (l.network[LoginCommand::flag]) { in >> l.variable;}
#define load(variable, flag) if (l.network[LoginCommand::flag]) { l.variable = new std::remove_reference<decltype(*l.variable)>::type(); in >> *l.variable;}

    test(clientType, HasClientType);
    test(clientVersion, HasVersionNumber);
    in >> l.trainerName;
    in >> l.data;
    test(reconnectBits, HasReconnect);

    load(channel, HasDefaultChannel);
    load(additionalChannels, HasAdditionalChannels);
    test(trainerColor, HasColor);
    load(trainerInfo, HasTrainerInfo);

    if (l.network[LoginCommand::HasTeams]) {
        l.teams = new QList<PersonalTeam>();
        qint8 count;
        in >> count;
        for (int i = 0; i < count; i++) {
            PersonalTeam t;
            in >> t;
            /* 6 teams tops */
            if (i < 6) {
                l.teams->push_back(t);
            }
        }
    }

    test(events, HasEventSpecification);
    load(plugins, HasPluginList);
    load(cookie, HasCookie)
#undef load
#undef test

    return in;
}

ChangeTeamInfo::ChangeTeamInfo()
{
    name = 0;
    color = 0;
    teams = 0;
    team = 0;
    teamNum = 0;
    info = 0;
}

ServerInfo::ServerInfo()
{
    num = max = port = passwordProtected = 0;
}

DataStream & operator >> (DataStream &in, ServerInfo &info)
{
    in >> info.name >> info.desc >> info.num >> info.ip >> info.max >> info.port >> info.passwordProtected;

    return in;
}
