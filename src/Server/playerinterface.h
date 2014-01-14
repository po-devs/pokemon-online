#ifndef PLAYERINTERFACE_H
#define PLAYERINTERFACE_H

#include <QtCore>

class QColor;
class TeamBattle;

class PlayerInterface
{
public:
    virtual ~PlayerInterface(){}
    virtual bool ladder() const = 0;
    virtual const quint16 &avatar() const = 0;
    virtual const QColor &color() const = 0;
    virtual const bool &battleSearch() const = 0;
    virtual const QString &winningMessage() const = 0;
    virtual const QString &losingMessage() const = 0;
    virtual const QString &tieMessage() const = 0;
    virtual const QString &lastFindBattleIp() const = 0;
    virtual const TeamBattle &team(int team=0) const = 0;
 };

#endif // PLAYERINTERFACE_H
