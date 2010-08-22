#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include <QtCore>

class QColor;
class TeamBattle;

class PlayerInterface
{
public:
    virtual ~PlayerInterface(){}
    virtual const int& rating() const = 0;
    virtual const bool& ladder() const = 0;
    virtual const bool &showteam() const = 0;
    virtual const QString &tier() const = 0;
    virtual const quint16 &avatar() const = 0;
    virtual const QColor &color() const = 0;
    virtual const bool &battleSearch() const = 0;
    virtual const QString &winningMessage() const = 0;
    virtual const QString &losingMessage() const = 0;
    virtual const QString &lastFindBattleIp() const = 0;
    virtual const TeamBattle &team() const = 0;
    virtual int gen() const = 0;
};

#endif // PLAYERINTERFACE_H
