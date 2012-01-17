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

    QStringList getProfileList(const QString &path);
    bool loadFromFile(const QString &path);
    void toXml(QDomDocument &doc) const;
    QString toXml() const;
    bool saveToFile(const QString &path) const;
    void deleteProfile(const QString &path);
};

class TeamHolder : public TeamHolderInterface
{
    PROPERTY(Profile, profile);

    TeamHolder();

    TrainerInfo &info() {return profile().info();}
    QString &name() { return profile().name();}
    QColor &color() { return profile().color();}

    Team &team();
    const Team &team() const;
    Team &team(int index);
    const Team &team(int index) const;
    int currentTeam() const {return m_currentTeam;}
    int count() const;

    void save();
    void load();
private:
    QList<Team> m_teams;
    int m_currentTeam;
};

#endif // TEAMHOLDER_H
