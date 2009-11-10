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

QDataStream & operator << (QDataStream & out,const TeamInfo & team);
QDataStream & operator >> (QDataStream & in,TeamInfo & team);

QDataStream & operator << (QDataStream & out,const BasicInfo & team);
QDataStream & operator >> (QDataStream & in,BasicInfo & team);


#endif // NETWORKSTRUCTS_H
