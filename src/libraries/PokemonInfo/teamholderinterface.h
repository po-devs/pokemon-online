#ifndef TEAMHOLDERINTERFACE_H
#define TEAMHOLDERINTERFACE_H

struct TrainerInfo;
class Team;
class QString;
class QColor;

class ProfileInterace {
public:
    virtual const QString &name() const = 0;
    virtual const QColor &color() const = 0;
    virtual const TrainerInfo &info() const = 0;
};

class TeamHolderInterface
{
public:
    virtual const Team &team() const = 0;
    virtual const ProfileInterace &profile() const = 0;
};

#endif // TEAMHOLDERINTERFACE_H
