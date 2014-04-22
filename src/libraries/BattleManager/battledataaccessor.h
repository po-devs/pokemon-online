#ifndef BATTLEDATAACCESSOR_H
#define BATTLEDATAACCESSOR_H

#include <QObject>
#include <QVector>
#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/battlestructs.h>
#include <cassert>

class TeamData;
class ShallowBattlePoke;
class PokeProxy;
class TeamProxy;

class MoveProxy : public QObject
{
    Q_OBJECT
public:
    MoveProxy();
    MoveProxy(BattleMove *move);
    ~MoveProxy();

    Q_PROPERTY(int PP READ PP NOTIFY PPChanged)
    Q_PROPERTY(int totalPP READ totalPP NOTIFY numChanged)
    Q_PROPERTY(int num READ num NOTIFY numChanged)

    int PP() const {return d()->PP();}
    int totalPP() const { return d()->totalPP();}
    int num() const { return d()->num();}

    void setNum(int newnum);
    void changePP(int newPP);
    Pokemon::gen gen() const;

    void adaptTo(const BattleMove *move);
    BattleMove &exposedData();
    const BattleMove &exposedData() const;
signals:
    void numChanged();
    void PPChanged();
private:
    bool hasOwnerShip;
    BattleMove *moveData;

    inline BattleMove *d() {return moveData;}
    inline const BattleMove *d() const {return moveData;}
    PokeProxy *master() const;
};

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
    Q_PROPERTY(int happiness READ happiness NOTIFY pokemonReset)
    Q_PROPERTY(int item READ item NOTIFY itemChanged)
    Q_PROPERTY(int nature READ nature NOTIFY pokemonReset)

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
    Q_INVOKABLE int basestat(int stat) const;
    Q_INVOKABLE int iv(int stat) const;
    Q_INVOKABLE int ev(int stat) const;
    const QList<int> &dvs() const { return dd()->dvs();}
    const QList<int> &evs() const { return dd()->evs();}
    Q_INVOKABLE MoveProxy *move(int slot) { assert (slot >= 0 && slot < numMoves()); return moves[slot];}
    const MoveProxy *move(int slot) const { assert (slot >= 0 && slot < numMoves()); return moves[slot];}
    Q_INVOKABLE int numMoves() const {return moves.length(); }
    Q_INVOKABLE bool isKoed() const { return d()->ko();}
    Pokemon::gen gen() const;

    void emitReset();
    void adaptTo(const ShallowBattlePoke *pokemon, bool soft=false);
    void adaptTo(const PokeBattle *pokemon);
    void changeStatus(int fullStatus);
    void setNum(Pokemon::uniqueId num);
    void setLife(int newLife);
    void setItem(int newItem);

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
    void itemChanged();
    void ko();
private:
    bool hasOwnerShip;
    ShallowBattlePoke *pokeData;
    inline ShallowBattlePoke *d() {return pokeData;}
    inline const ShallowBattlePoke *d() const {return pokeData;}
    inline PokeBattle* dd() {return (PokeBattle*)pokeData;}
    inline const PokeBattle* dd() const {return (PokeBattle*)pokeData;}
    TeamProxy *master() const;

    QVector<MoveProxy*> moves;
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

    void removeItem(int item);
    void changeItemCount(int item, int count);

    void switchPokemons(int index, int prevIndex);

    Q_PROPERTY(QString name READ name CONSTANT)
    QString name() const;
    QHash<quint16, quint16> &items();
    /* Not really sure this should be in this class */
    Q_PROPERTY(int time READ time NOTIFY timeChanged)
    int time() const;
    void setTimeLeft(int time, bool ticking=false);
    bool ticking() const;
    Q_PROPERTY(Pokemon::gen gen READ gen CONSTANT)
    Pokemon::gen gen() const;
signals:
    void pokemonsSwapped(int slot1, int slot2);
    void timeChanged();
private:
    TeamData *teamData;
    bool hasOwnerShip;
    int mTime;
    bool mTicking;

    QVector<PokeProxy *> pokemons;
};

#endif // BATTLEDATAACCESSOR_H
