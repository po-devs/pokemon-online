#include "networkstructs.h"
#include "../Utilities/coreclasses.h"
#include "../Shared/config.h"

PokePersonal & TeamInfo::pokemon(int num)
{
    return m_pokes[num];
}


const PokePersonal & TeamInfo::pokemon(int num) const
{
    return m_pokes[num];
}

DataStream &operator << (DataStream &out, const TeamInfo& team)
{
    out << team.name;
    out << team.info;
    out << team.lose;
    out << team.win;
    out << team.avatar;
    out << team.defaultTier;

    out << team.gen;

    for (int i = 0; i < 6; i++)
    out << team.pokemon(i);

    return out;
}

DataStream &operator >> (DataStream &in, TeamInfo& team)
{
    in >> team.name;
    in >> team.info;
    in >> team.lose;
    in >> team.win;
    in >> team.avatar;
    in >> team.defaultTier;

    in >> team.gen;

    if (team.gen < GEN_MIN || team.gen > GEN_MAX)
        team.gen = GEN_MAX;

    for (int i = 0; i < 6; i++)
        team.pokemon(i).gen() = team.gen;

    for (int i = 0; i < 6; i++) {
        in >> team.pokemon(i);
    }

    if (team.info.length() > 300) {
        team.info.resize(300);
    }
    if (team.lose.length() > 200) {
        team.lose.resize(200);
    }
    if (team.win.length() > 200) {
        team.win.resize(200);
    }

    return in;
}

DataStream &operator << (DataStream &out, const BasicInfo& team)
{
    out << team.name;
    out << team.info;

    return out;
}

DataStream &operator >> (DataStream &in, BasicInfo& team)
{
    in >> team.name;
    in >> team.info;

    /* To avoid server overloads */
    if (team.info.length() > 250)
        team.info.resize(250);

    return in;
}


DataStream & operator >> (DataStream &in, PlayerInfo &p)
{
    in >> p.id;
    in >> p.team;
    in >> p.auth;
    in >> p.flags;
    in >> p.rating;

    for (int i = 0; i < 6; i++) {
        in >> p.pokes[i];
    }

    in >> p.avatar;
    in >> p.tier;
    in >> p.color;
    in >> p.gen;

    return in;
}

DataStream & operator << (DataStream &out, const PlayerInfo &p)
{
    out << p.id;
    out << p.team;
    out << p.auth;
    out << p.flags;
    out << p.rating;

    for (int i = 0; i < 6; i++) {
        out << p.pokes[i];
    }

    out << p.avatar;
    out << p.tier;
    out << p.color;
    out << p.gen;

    return out;
}

DataStream & operator >> (DataStream &in, FullInfo &p)
{
    in >> p.team >> p.ladder >> p.showteam >> p.nameColor;

    return in;
}

DataStream & operator << (DataStream &out, const FullInfo &p)
{
    out << p.team << p.ladder << p.showteam << p.nameColor;

    return out;
}

Battle::Battle(int id1, int id2) : id1(id1), id2(id2)
{

}

DataStream & operator >> (DataStream &in, Battle &p)
{
    //in >> p.battleid >> p.id1 >> p.id2;
    in >> p.id1 >> p.id2;

    return in;
}

DataStream & operator << (DataStream &out, const Battle &p)
{
    //out << p.battleid << p.id1 << p.id2;
    out << p.id1 << p.id2;

    return out;
}

Flags::Flags()
{
    memset(data, 0, size);
}

bool Flags::operator [](int index) {
    /* out of bounds */
    if (index >= size*7 || index < 0) {
        return false;
    }

    return data[index/7] & (1 << (index % 7));
}

DataStream &operator << (DataStream &out, const Flags &f) {
    const uchar *d = f.data;
    do {
        out << d[0];
        d++;
    } while (d[-1] & (1 << 7));

    return out;
}

DataStream &operator >> (DataStream &in, Flags &f) {
    memset(f.data, 0, Flags::size);

    uchar *d = f.data;
    do {
        in >> d[0];
        d++;
    } while (d[-1] & (1 << 7));

    return in;
}

ProtocolVersion::ProtocolVersion()
{
    version = PROTOCOL_VERSION;
    subversion = PROTOCOL_SUBVERSION;
}

DataStream &operator >> (DataStream &in, ProtocolVersion &p)
{
    in >> p.version >> p.subversion;
    return in;
}

DataStream &operator << (DataStream &in, const ProtocolVersion &p)
{
    out << p.version << p.subversion;
    return out;
}
