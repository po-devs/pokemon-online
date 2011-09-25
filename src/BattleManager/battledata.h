#ifndef BATTLEDATA_H
#define BATTLEDATA_H

#include "battlecommandmanager.h"
#include "datacontainer.h"

class BattleConfiguration;

template <class T, class Derived>
class BattleDataInherit : public BattleCommandManager<Derived>
{
public:
    BattleDataInherit(BattleConfiguration *conf) : cont(conf) {
    }

    typedef T container;
    typedef decltype(container(0).team(0)) teamTypePtr;
    typedef decltype(*teamTypePtr(0)) teamType;
    typedef decltype(teamTypePtr(0)->poke(0)) pokeTypePtr;
    typedef decltype(*pokeTypePtr(0)) pokeType;
    typedef decltype(container(0).fieldPoke(0)) auxTypeRef;

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

        fieldPoke(spot).onSendOut();
    }

    void onSendBack(int spot, bool)
    {
        fieldPoke(spot).onSendBack();
    }

    void onHpChange(int spot, int newHp)
    {
        poke(spot).setLife(newHp);
    }

    void onMajorStatusChange(int spot, int status, bool)
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

    teamType &team(int player) { return *d()->team(this->player(player)); }
    pokeType &poke(int player) { return *team(this->player(player)).poke(slotNum(player));}
    int player(int spot) { return spot%2;}
    int opponent(int player) { return (player+1)%2;}
    QString name(int player) { return team(this->player(player)).name();}
    int slotNum(int player) { return player/2;}
    int spot(int player, int slot) {return player+2*slot;}
    auxTypeRef fieldPoke(int player) {return d()->fieldPoke(player);}
    int gen() { return GEN_MAX; }
    BattleConfiguration::ReceivingMode role(int player) { return conf->receivingMode[this->player(player)];}

    enum {
        Player1,
        Player2
    };

    container *exposedData() { return d(); }
protected:
    container cont;
    BattleConfiguration *conf;
    container* d() { return &cont;}
};

template <class T = DataContainer>
class BattleData: public BattleDataInherit<T, BattleData<T> >
{
public:
    typedef BattleDataInherit<T, BattleData<T> > baseClass;

    BattleData(BattleConfiguration *conf) : baseClass(conf) {}
};

#endif // BATTLEDATA_H
