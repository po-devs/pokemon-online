#ifndef TEAMHOLDERINTERFACE_H
#define TEAMHOLDERINTERFACE_H

class TrainerInfo;
class Team;
class QString;

class TeamHolderInterface
{
public:
    virtual const Team &team() const = 0;
    virtual const QString &name() const = 0;
    virtual const TrainerInfo &info() const = 0;
};

#endif // TEAMHOLDERINTERFACE_H
