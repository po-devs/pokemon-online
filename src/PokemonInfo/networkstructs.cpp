#include "networkstructs.h"



PokePersonal & TeamInfo::pokemon(int num)
{
    return m_pokes[num];
}


const PokePersonal & TeamInfo::pokemon(int num) const
{
    return m_pokes[num];
}

QDataStream &operator << (QDataStream &out, const TeamInfo& team)
{
    out << team.name;
    out << team.info;
    out << team.lose;
    out << team.win;
    for (int i = 0; i < 6; i++)
	out << team.pokemon(i);

    return out;
}

QDataStream &operator >> (QDataStream &in, TeamInfo& team)
{
    in >> team.name;
    in >> team.info;
    in >> team.lose;
    in >> team.win;
    for (int i = 0; i < 6; i++)
	in >> team.pokemon(i);

    return in;
}

QDataStream &operator << (QDataStream &out, const BasicInfo& team)
{
    out << team.name;
    out << team.info;

    return out;
}

QDataStream &operator >> (QDataStream &in, BasicInfo& team)
{
    in >> team.name;
    in >> team.info;

    return in;
}

