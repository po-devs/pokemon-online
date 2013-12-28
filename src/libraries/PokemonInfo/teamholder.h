#ifndef TEAMHOLDER_H
#define TEAMHOLDER_H

#include "pokemonstructs.h"
#include "networkstructs.h"
#include "teamholderinterface.h"

class Profile : public ProfileInterace
{
    PROPERTY(TrainerInfo, info)
    PROPERTY(QString, name)
    PROPERTY(QColor, color)

    QStringList getProfileList(const QString &path);
    bool loadFromFile(const QString &path);
    void toXml(QDomDocument &doc) const;
    QString toXml() const;
    bool saveToFile(const QString &path) const;
    void deleteProfile(const QString &path);

    void sanitize();
};

class TeamHolder : public TeamHolderInterface
{
    PROPERTY(Profile, profile)

    TeamHolder();
    TeamHolder(const QString &name);
    virtual ~TeamHolder();

    TrainerInfo &info() {return profile().info();}
    const TrainerInfo &info() const {return profile().info();}
    QString &name() { return profile().name();}
    const QString &name() const { return profile().name();}
    QColor &color() { return profile().color();}
    const QColor &color() const { return profile().color();}

    Team &team();
    const Team &team() const;
    Team &team(int index);
    const Team &team(int index) const;
    int currentTeam() const {return m_currentTeam;}
    void setCurrent(int t) {m_currentTeam = t;}
    int count() const;
    int officialCount() const;

    QString tier(int team) const;
    QString tier() const;

    void addTeam();
    void removeTeam();

    void save();
    void load();

    void setTiers(const QStringList &tiers);
private:
    QList<Team> m_teams;
    QStringList m_tiers; //The tier of each team, provided by the server.
    int m_currentTeam;
};

#endif // TEAMHOLDER_H
