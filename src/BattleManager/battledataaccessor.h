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
    Q_PROPERTY(int lifePercent READ lifePercent STORED false NOTIFY lifeChanged)

    enum Status {
        Koed = Pokemon::Koed,
        Paralysed = Pokemon::Paralysed,
        Asleep = Pokemon::Asleep,
        Burnt = Pokemon::Burnt,
        Frozen = Pokemon::Frozen,
        Fine = Pokemon::Fine,
        Poisoned = Pokemon::Poisoned
    };

    Q_ENUMS(Status)

    QString nickname() const {return d()->nick();}
    int status() const {return d()->status();}
    Pokemon::uniqueId num() const {return d()->num();}
    bool shiny() const {return d()->shiny();}
    int gender() const {return d()->gender();}
    int level() const {return d()->level();}
    int numRef() const {return d()->num().toPokeRef();}
    int life() const {return d()->life();}
    int lifePercent() const {return d()->lifePercent();}
    int totalLife() const {return d()->totalLife();}
    int ability() const {return dd()->ability();}
    int item() const {return dd()->item();}
    int happiness() const {return dd()->happiness();}
    int nature() const {return dd()->nature();}
    int basestat(int stat) const;
    const QList<int> &dvs() const { return dd()->dvs();}
    const QList<int> &evs() const { return dd()->evs();}
    BattleMove &move(int slot) {return dd()->move(slot);}
    const BattleMove &move(int slot) const {return dd()->move(slot);}
    Q_INVOKABLE bool isKoed() const { return d()->ko();}

    void adaptTo(const ShallowBattlePoke *pokemon, bool soft=false);
    void adaptTo(const PokeBattle *pokemon);
    void changeStatus(int fullStatus);
    void setNum(Pokemon::uniqueId num);
    void setLife(int newLife);

    bool hasExposedData() const {
        return dynamic_cast<PokeBattle*>(pokeData) != NULL;
    }

    void setOwnerShip(bool o) {hasOwnerShip = o;}

    ShallowBattlePoke *exposedData() { return d();}
signals:
    void numChanged();
    void statusChanged();
    void pokemonReset();
    void lifeChanged();
    void ko();
private:
    bool hasOwnerShip;
    ShallowBattlePoke *pokeData;
    inline ShallowBattlePoke *d() {return pokeData;}
    inline const ShallowBattlePoke *d() const {return pokeData;}
    inline PokeBattle* dd() {return (PokeBattle*)pokeData;}
    inline const PokeBattle* dd() const {return (PokeBattle*)pokeData;}
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

    const PokeProxy* poke(int index) const {
        return pokemons[index];
    }

    void setTeam(const TeamBattle *team);
    void setPoke(int index, ShallowBattlePoke *pokemon, bool soft = false);
    void setName(const QString &name);

    void switchPokemons(int index, int prevIndex);

    Q_PROPERTY(QString name READ name CONSTANT)
    QString name() const;
signals:
    void pokemonsSwapped(int slot1, int slot2);
private:
    TeamData *teamData;
    bool hasOwnerShip;

    QVector<PokeProxy *> pokemons;
};

#endif // BATTLEDATAACCESSOR_H
