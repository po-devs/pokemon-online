#ifndef TEAMHOLDER_H
#define TEAMHOLDER_H

#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/networkstructs.h"
#include "teamholderinterface.h"

class Profile : public ProfileInterace
{
    PROPERTY(TrainerInfo, info);
    PROPERTY(QString, name);
    PROPERTY(QColor, color);

    bool loadFromFile(const QString &path);
    void toXml(QDomDocument &doc) const;
    QString toXml() const;
    bool saveToFile(const QString &path) const;
};

class TeamHolder : public TeamHolderInterface
{
    PROPERTY(Profile, profile);
    PROPERTY(Team, team);

    TrainerInfo &info() {return profile().info();}
    QString &name() { return profile().name();}
    QColor &color() { return profile().color();}
};

#endif // TEAMHOLDER_H
