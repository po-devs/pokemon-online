#ifndef BATTLEDATAACCESSOR_H
#define BATTLEDATAACCESSOR_H

#include <QObject>
#include <QVector>
#include "../PokemonInfo/pokemonstructs.h"

class BattleData;
class TeamData;
class ShallowBattlePoke;

class PokeProxy : public QObject
{
    Q_OBJECT
public:
    PokeProxy(ShallowBattlePoke *pokemon=0);

    Q_PROPERTY(QString nick READ nickname)
    Q_PROPERTY(int status READ status)
    Q_PROPERTY(Pokemon::uniqueId num READ num CONSTANT)
    Q_PROPERTY(bool shiny READ shiny)
    Q_PROPERTY(int gender READ gender)
    Q_PROPERTY(int level READ level)

    QString nickname();
    int status();
    Pokemon::uniqueId num();
    bool shiny();
    int gender();
    int level();
private:
    ShallowBattlePoke *pokeData;
    inline ShallowBattlePoke *d() {return pokeData;}
};

class TeamProxy : public QObject
{
    Q_OBJECT
public:
    TeamProxy(TeamData *teamData=0);
    ~TeamProxy();

    Q_INVOKABLE PokeProxy* poke(int index) {
        return pokemons[index];
    }

    Q_PROPERTY(QString name READ name)
    QString name();

private:
    TeamData *teamData;

    QVector<PokeProxy *> pokemons;
};

class BattleDataProxy : public QObject
{
    Q_OBJECT
public:
    BattleDataProxy(BattleData *battleData=0);
    ~BattleDataProxy();

    Q_INVOKABLE TeamProxy *team(int player) {
        return teams[player];
    }
private:
    BattleData *battleData;

    QVector<TeamProxy*> teams;
};

#endif // BATTLEDATAACCESSOR_H
