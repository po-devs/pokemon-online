#ifndef BATTLEDATA_H
#define BATTLEDATA_H

#include "battlecommandmanager.h"
#include "datacontainer.h"

struct BattleConfiguration;

template <class T, class Derived>
class BattleDataInherit : public BattleCommandManager<Derived>
{
public:
    BattleDataInherit(const BattleConfiguration *conf) : cont(conf), conf(conf) {
    }

    typedef T container;
    typedef typename container::teamType* teamTypePtr;
    typedef typename container::teamType teamType;
    typedef decltype(teamTypePtr(0)->poke(0)) pokeTypePtr;
    typedef decltype(*pokeTypePtr(0)) pokeType;
    typedef decltype(container(0).fieldPoke(0)) auxTypeRef;
    typedef typename container::fieldType fieldType;

    void onKo(int spot)
    {
        poke(spot).changeStatus(Pokemon::Koed);
    }

    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool)
    {
        int player = this->player(spot);
        int slot = this->slotNum(spot);

        if (slot != previndex) {
            team(player).switchPokemons(slot, previndex);
        }

        team(player).setPoke(slot, pokemon);

        fieldPoke(spot).onSendOut(pokemon);
    }

    void onSendBack(int spot, bool)
    {
        fieldPoke(spot).onSendBack();
    }

    void onHpChange(int spot, int newHp)
    {
        if (isPlayer(spot)) {
            poke(spot).setLife(newHp);
        }
    }

    void onMajorStatusChange(int spot, int status, bool, bool)
    {
        //TODO: handle confusion better
        if (status != Pokemon::Confused) {
            poke(spot).changeStatus(status);
        }
    }

    void onPokeballStatusChanged(int player, int poke, int status)
    {
        if (status != Pokemon::Confused) {
            team(player).poke(poke)->changeStatus(status);
        }
    }

    void onSubstituteStatus(int spot, bool substitute)
    {
        fieldPoke(spot).setSubstitute(substitute);
    }

    void onPokemonVanish(int spot)
    {
        fieldPoke(spot).vanish();
    }

    void onPokemonReappear(int spot)
    {
        fieldPoke(spot).reappear();
    }

    void onSpriteChange(int spot, int newSprite)
    {
        fieldPoke(spot).changeSprite(newSprite);
    }

    void onDefiniteFormeChange(int player, int poke, int newPoke)
    {
        team(player).poke(poke)->setNum(newPoke);
    }

    void onCosmeticFormeChange(int spot, int subforme)
    {
        fieldPoke(spot).changeForme(subforme);
    }

    void onShiftSpots(int player, int spot1, int spot2, bool)
    {
        d()->field()->shiftSpots(spot(player, spot1), spot(player, spot2));
        team(player).switchPokemons(spot1, spot2);
    }

    void onItemChangeCount(int player, int item, int count)
    {
        if (count <= 0) {
            team(player).removeItem(item);
        } else {
            team(player).changeItemCount(item, count);
        }
    }

    const teamType &team(int player) const { return *d()->team(this->player(player));}
    teamType &team(int player) { return *d()->team(this->player(player));}
    const QHash<quint16,quint16> &items(int player) const { return team(player).items();}
    QHash<quint16,quint16> &items(int player) { return team(player).items();}
    const fieldType &field() const { return *d()->field();}
    fieldType &field() { return *d()->field();}
    pokeType &poke(int player) { return *team(this->player(player)).poke(slotNum(player));}
    int player(int spot) const { return spot%2;}
    int opponent(int player) const { return (player+1)%2;}
    QString name(int player) const { return team(this->player(player)).name();}
    int slotNum(int player) const { return player/2;}
    bool isOut(int spot) const { return spot < numberOfSlots();}
    bool areAdjacent (int poke1, int poke2) const { return abs(slotNum(poke1)-slotNum(poke2)) <= 1;}
    int spot(int player, int slot=0) {return player+2*slot;}
    int spotFromId(int id) const { return conf->spot(id);}
    int clauses() const {return conf->clauses;}
    int countAlive(int player) const {
        int ret = 0;

        for (int i = 0; i < 6; i++) {
            if (!team(player).poke(i)->isKoed()) {
                ret++;
            }
        }
        return ret;
    }
    bool rated() const {return conf->rated();}
    bool oldConf() const {return conf->oldconf;}
    int avatar(int player) const {return conf->avatar[player];}
    int mode() const {return conf->mode;}
    auxTypeRef fieldPoke(int player) {return d()->fieldPoke(player);}
    Pokemon::gen gen() { return conf->gen; }
    bool isKoed(int spot) { return poke(spot).isKoed();}
    BattleConfiguration::ReceivingMode role(int player) const { return BattleConfiguration::ReceivingMode(conf->receivingMode[this->player(player)]);}
    bool isPlayer(int spot) const { return role(spot) == BattleConfiguration::Player;}
    int numberOfSlots() const {return conf->numberOfSlots();}
    bool multiples() const {return (conf->mode != ChallengeInfo::Singles);}

    void reloadTeam(int player) {
        d()->reloadTeam(player);
    }

    enum {
        Player1,
        Player2
    };

    container *exposedData() { return d(); }
protected:
    container cont;
    const BattleConfiguration *conf;
    container* d() { return &cont;}
    const container* d() const { return &cont;}
};

template <class T = DataContainer>
class BattleData: public BattleDataInherit<T, BattleData<T> >
{
public:
    typedef BattleDataInherit<T, BattleData<T> > baseClass;

    BattleData(const BattleConfiguration *conf) : baseClass(conf) {}
};

#endif // BATTLEDATA_H
