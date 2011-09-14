#ifndef BATTLEDATA_H
#define BATTLEDATA_H

#include "battlecommandmanager.h"
#include "datacontainer.h"

template <class T=DataContainer>
class BattleData : public BattleCommandManager<BattleData<T> >
{
public:
    typedef T container;
    typedef decltype(container().team(0)) teamTypePtr;
    typedef decltype(*teamTypePtr(0)) teamType;
    typedef decltype(teamTypePtr(0)->poke(0)) pokeTypePtr;
    typedef decltype(*pokeTypePtr(0)) pokeType;
    typedef decltype(container().fieldPoke(0)) auxTypeRef;

    BattleData(){}

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

    void onSendBack(int spot)
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
        fieldPoke(spot).subsitute = substitute;
    }

    void onPokemonVanish(int spot)
    {
        fieldPoke(spot).showing = false;
    }

    void onPokemonReappear(int spot)
    {
        fieldPoke(spot).showing = true;
    }

    void onSpriteChange(int spot, int newSprite)
    {
        fieldPoke(spot).alternateSprite = newSprite;
    }

    void onDefiniteFormeChange(int player, int poke, int newPoke)
    {
        team(player).poke(poke)->setNum(newPoke);
    }

    void onCosmeticFormeChange(int spot, int subforme)
    {
        fieldPoke(spot).alternateSprite.subnum = subforme;
    }

    void onShiftSpots(int player, int spot1, int spot2, bool)
    {
        d()->swapFieldPokemons(spot(player, spot1), spot(player, spot2));
        team(player).switchPokemons(spot1, spot2);
    }

    teamType &team(int player) { return *d()->team(player); }
    pokeType &poke(int player) { return *team(player).poke(slotNum(player));}
    int player(int spot) { return spot%2;}
    int opponent(int player) { return (player+1)%2;}
    QString name(int player) { return team(player).name();}
    int slotNum(int player) { return player/2;}
    int spot(int player, int slot) {return player+2*slot;}
    auxTypeRef fieldPoke(int player) {return d()->fieldPoke(player);}
    int gen() { return GEN_MAX; }

    enum {
        Player1,
        Player2
    };

    container *exposedData() { return d(); }
private:
    container cont;
    container* d() { return &cont;}
};

#endif // BATTLEDATA_H
