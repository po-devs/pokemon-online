#ifndef BATTLEDATAACCESSOR_H
#define BATTLEDATAACCESSOR_H

#include <QObject>
#include <QVector>
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/battlestructs.h"

class TeamData;
class ShallowBattlePoke;

class PokeProxy : public QObject
{
    Q_OBJECT
public:
    /* Creates a pokemon that is managed by the current object */
    PokeProxy();
    /* Creates a pokemon from external data */
    PokeProxy(ShallowBattlePoke *pokemon);
    ~PokeProxy();

    Q_PROPERTY(QString nick READ nickname NOTIFY pokemonReset)
    Q_PROPERTY(int status READ status NOTIFY statusChanged)
    Q_PROPERTY(Pokemon::uniqueId num READ num NOTIFY numChanged)
    Q_PROPERTY(bool shiny READ shiny NOTIFY pokemonReset)
    Q_PROPERTY(int gender READ gender NOTIFY pokemonReset)
    Q_PROPERTY(int level READ level NOTIFY pokemonReset)
    Q_PROPERTY(int numRef READ numRef STORED false NOTIFY numChanged)
    Q_PROPERTY(int life READ life NOTIFY lifeChanged)

    QString nickname() {return d()->nick();}
    int status() {return d()->status();}
    Pokemon::uniqueId num() {return d()->num();}
    bool shiny() {return d()->shiny();}
    int gender() {return d()->gender();}
    int level() {return d()->level();}
    int numRef() {return d()->num().toPokeRef();}
    int life() { return d()->life();}

    void adaptTo(ShallowBattlePoke *pokemon);

    void changeStatus(int fullStatus) {
        if (d()->status() == fullStatus) {
            return;
        }
        d()->changeStatus(fullStatus);
        emit statusChanged();
    }

    void setNum(Pokemon::uniqueId num) {
        if (d()->num() == num) {
            return;
        }
        d()->num() = num;
        emit numChanged();
    }

    void setLife(int newLife) {
        if (d()->life() == newLife) {
            return;
        }
        d()->setLife(newLife);
        emit lifeChanged();
    }

signals:
    void numChanged();
    void statusChanged();
    void pokemonReset();
    void lifeChanged();
private:
    bool hasOwnerShip;
    ShallowBattlePoke *pokeData;
    inline ShallowBattlePoke *d() {return pokeData;}
};

class TeamProxy : public QObject
{
    Q_OBJECT
public:
    TeamProxy();
    TeamProxy(TeamData *teamData);
    ~TeamProxy();

    Q_INVOKABLE PokeProxy* poke(int index) {
        return pokemons[index];
    }

    void setPoke(int index, ShallowBattlePoke *pokemon);

    void switchPokemons(int index, int prevIndex);

    Q_PROPERTY(QString name READ name CONSTANT)
    QString name();
private:
    TeamData *teamData;
    bool hasOwnerShip;

    QVector<PokeProxy *> pokemons;
};

#endif // BATTLEDATAACCESSOR_H
