#ifndef ADVANCEDBATTLEDATA_H
#define ADVANCEDBATTLEDATA_H

#include "battledata.h"
#include "proxydatacontainer.h"
#include "zoneproxy.h"

class AdvancedBattleData : public QObject, public BattleDataInherit<ProxyDataContainer, AdvancedBattleData>
{
public:
    typedef BattleDataInherit<ProxyDataContainer, AdvancedBattleData> baseClass;

    AdvancedBattleData(const BattleConfiguration *conf) : baseClass(conf) {
        timer.start(1000, this);
    }

    ~AdvancedBattleData() {

    }

    void onUseAttack(int spot, int attack, bool silent)
    {
        (void) silent;
        if (isPlayer(spot)) {
            return;
        }

        for (int i = 0; i < 4; i++) {
            if (poke(spot).move(i)->num() == Move::NoMove) {
                poke(spot).move(i)->setNum(attack);
                poke(spot).move(i)->changePP(poke(spot).move(i)->totalPP() - 1);

                break;
            } else if (poke(spot).move(i)->num() == attack) {
                poke(spot).move(i)->changePP(poke(spot).move(i)->PP() - 1);

                break;
            }
        }
    }

    /* The code could be refactored to be included in battledata directly,
      with the regular battledata container having also the extras functions */
    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool)
    {
        int player = this->player(spot);
        int slot = this->slotNum(spot);

        if (slot != previndex) {
            team(player).switchPokemons(slot, previndex);

            fieldPoke(spot).setPoke(team(player).poke(slot));
        }

        team(player).setPoke(slot, pokemon, isPlayer(spot));

        fieldPoke(spot).onSendOut(pokemon);
    }

    void onKo(int spot)
    {
        if (isOut(spot)) {
            tempPoke(spot).changeStatus(Pokemon::Koed);
        }
        if (isPlayer(spot)) {
            poke(spot).changeStatus(Pokemon::Koed);
        }
    }

    void onHpChange(int spot, int newHp)
    {
        if (isOut(spot)) {
            tempPoke(spot).setLife(newHp);
        }
        if (isPlayer(spot)) {
            poke(spot).setLife(newHp);
        }
    }

    void onMajorStatusChange(int spot, int status, bool, bool)
    {
        //TODO: handle confusion better
        if (status != Pokemon::Confused) {
            tempPoke(spot).changeStatus(status);
            if (isPlayer(spot)) {
                poke(spot).changeStatus(status);
            }
        }
    }

    void onStatBoost(int spot, int stat, int boost, bool silent) {
        (void) silent;
        fieldPoke(spot).boostStat(stat, boost);
    }

    void onDynamicInfo(int spot, const BattleDynamicInfo &info) {
        for (int i = 1; i < 8; i++) {
            fieldPoke(spot).setBoost(i, info.boosts[i]);
        }
        field().zone(player(spot))->setHazards(info.flags);
    }

    void onDynamicStats(int spot, const BattleStats &stats) {
        for (int i = 1; i < 6; i++) {
            fieldPoke(spot).setStat(i, stats.stats[i]);
        }
    }

    void onStartWeather(int spot, int weather, bool ability) {
        (void) ability;
        (void) spot;
        field().setWeather(weather);
    }

    void onContinueWeather(int weather) {
        field().setWeather(weather);
    }

    void onEndWeather(int weather) {
        (void) weather;
        field().setWeather(Weather::NormalWeather);
    }

    void onClockStart(int player, int time) {
        team(player).setTimeLeft(time,true);
    }

    void onClockStop(int player, int time) {
        team(player).setTimeLeft(time,false);
    }

    void onTeamOrderChosen(int player, const RearrangeChoice &order) {
        if (isPlayer(player)) {
            conf->teams[player]->setIndexes(order.pokeIndexes);
            reloadTeam(player);
        }
    }

    void onPPChange(int spot, int move, int PP) {
        poke(spot).move(move)->changePP(PP);
        if (isOut(spot)) {
            tempPoke(spot).move(move)->changePP(PP);
        }
    }

    void onTempPPChange(int spot, int move, int PP) {
        tempPoke(spot).move(move)->changePP(PP);
    }

    void onMoveChange(int spot, int slot, int move, bool definite) {
        tempPoke(spot).move(slot)->setNum(move);
        if (definite) {
            poke(spot).move(slot)->setNum(move);
        }
    }

    /* the temp poke code could be made generic */
    PokeProxy &tempPoke(int spot) {
        return *fieldPoke(spot).pokemon();
    }

    void timerEvent(QTimerEvent *) {
        if (team(Player1).ticking()) {
            team(Player1).setTimeLeft(team(Player1).time()-1, true);
        }
        if (team(Player2).ticking()) {
            team(Player2).setTimeLeft(team(Player2).time()-1, true);
        }
    }

private:
    QBasicTimer timer;
};

#endif // ADVANCEDBATTLEDATA_H
