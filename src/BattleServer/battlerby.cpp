#include "battlerby.h"
#include "../Shared/battlecommands.h"
#include "rbymoves.h"

using namespace BattleCommands;
typedef BattleBase::TurnMemory TM;

BattleRBY::BattleRBY(const BattlePlayer &p1, const BattlePlayer &p2, const ChallengeInfo &additionnalData, int id, const TeamBattle &t1, const TeamBattle &t2, BattleServerPluginManager *p)
{
    ChallengeInfo c = additionnalData;
    c.mode = ChallengeInfo::Singles;

    init(p1, p2, c, id, t1, t2, p);
}

BattleRBY::~BattleRBY()
{
    onDestroy();
}

void BattleRBY::debug(const QString &message)
{
    battleChat(conf.ids[0], message);
}

void BattleRBY::endTurn()
{
    testWin();
    requestSwitchIns();

    speedsVector = sortedBySpeed();

    for (unsigned i = 0; i < speedsVector.size(); i++) {
        /* Disable counter here ? */
        //counters(speedsVector[i]).decreaseCounters();
    }

    /* leech seed damage ? Hyper beam count ? */
}

void BattleRBY::initializeEndTurnFunctions()
{

}

void BattleRBY::changeStatus(int player, int status, bool tell, int turns)
{
    if (poke(player).status() == status) {
        return;
    }
    // Sub blocks status in Stadium
    if (hasSubstitute(player) && isStadium()) {
        return;
    }

    // Sleep clause
    if (status != Pokemon::Asleep && currentForcedSleepPoke[this->player(player)] == currentInternalId(player)) {
        currentForcedSleepPoke[this->player(player)] = -1;
    }

    notify(All, StatusChange, player, qint8(status), turns > 0, !tell);
    notify(All, AbsStatusChange, this->player(player), qint8(this->slotNum(player)), qint8(status), turns > 0);
    poke(player).addStatus(status);

    if (turns != 0) {
        poke(player).statusCount() = turns;
    }
    else if (status == Pokemon::Asleep) {
        if (!isStadium()) {
            poke(player).statusCount() = 2 + (randint(7));
        } else {
            poke(player).statusCount() = 2 + (randint(3));
        }
    }
    else {
        poke(player).statusCount() = 0;
    }

    if (status == Pokemon::Paralysed) {
        fpoke(player).stats[Speed] = std::max(getBoostedStat(player, Speed) / 4, 1);
    } else if (status == Pokemon::Burnt) {
        fpoke(player).stats[Attack] = std::max(getBoostedStat(player, Attack) / 2, 1);
    }
}

int BattleRBY::getStat(int poke, int stat)
{
    return std::max(std::min(fpoke(poke).stats[stat], 999), 1);
}

void BattleRBY::sendPoke(int slot, int pok, bool silent)
{
    int player = this->player(slot);
    int snum = slotNum(slot);

    /* reset temporary variables */
    pokeMemory(slot).clear();

//    /* Reset counters */
//    counters(slot).clear();

    notify(All, SendOut, slot, silent, quint8(pok), opoke(slot, player, pok));

    team(player).switchPokemon(snum, pok);

    PokeBattle &p = poke(slot);

    //Clears secondary statuses
    int st = p.status();
    p.fullStatus() = 0;
    p.changeStatus(st);

    /* Give new values to what needed */
    fpoke(slot).init(p, gen());

    if (p.status() != Pokemon::Asleep)
        p.statusCount() = 0;

    /* Increase the "switch count". Switch count is used to see if the pokemon has switched
       (like for an attack like attract), it is imo more effective that other means */
    slotMemory(slot).switchCount += 1;

    turnMem(slot).flags &= TurnMemory::Incapacitated;
}

BattleChoice& BattleRBY::choice(int p)
{
    return choices[p];
}

BattleChoices BattleRBY::createChoice(int slot)
{
    /* First let's see for attacks... */
    if (koed(slot)) {
        return BattleChoices::SwitchOnly(slot);
    }

    BattleChoices ret;
    ret.numSlot = slot;

    callpeffects(slot, slot, "MovesPossible");

    for (int i = 0; i < 4; i++) {
        if (!isMovePossible(slot,i)) {
            ret.attackAllowed[i] = false;
        }
    }

    return ret;
}

void BattleRBY::analyzeChoices()
{
    setupChoices();

    std::map<int, std::vector<int>, std::greater<int> > priorities;
    std::vector<int> switches;
    std::vector<int> items;

    std::vector<int> playersByOrder = sortedBySpeed();

    foreach(int i, playersByOrder) {
        if (choice(i).itemChoice()) {
            items.push_back(i);
        } else if (choice(i).switchChoice())
            switches.push_back(i);
        else if (choice(i).attackingChoice()){
            priorities[tmove(i).priority].push_back(i);
        } else {
            /* Shifting choice */
            priorities[0].push_back(i);
        }
    }

    foreach(int player, items) {
        analyzeChoice(player);

        personalEndTurn(player);
        notify(All, BlankMessage, Player1);
    }

    foreach(int player, switches) {
        analyzeChoice(player);
        //callEntryEffects(player);

        personalEndTurn(player);
        notify(All, BlankMessage, Player1);
    }

    std::map<int, std::vector<int>, std::greater<int> >::const_iterator it;
    std::vector<int> &players = speedsVector;
    players.clear();

    for (it = priorities.begin(); it != priorities.end(); ++it) {
        foreach (int player, it->second) {
            players.push_back(player);
        }
    }

    for(unsigned i = 0; i < players.size(); i++) {
        if (!multiples()) {
            if (koed(0) || koed(1))
                break;
        } else {
            requestSwitchIns();
        }

        if (!hasMoved(players[i])) {
            analyzeChoice(players[i]);

            if (!multiples() && (koed(0) || koed(1))) {
                testWin();
                selfKoer() = -1;
                break;
            }

            personalEndTurn(players[i]);
            notify(All, BlankMessage, Player1);
        }
        testWin();
        selfKoer() = -1;
    }
}

void BattleRBY::personalEndTurn(int player)
{
    if (koed(player)) return;

    switch(poke(player).status())
    {
    case Pokemon::Burnt:
        notify(All, StatusMessage, player, qint8(HurtBurn));
        //HeatProof: burn does only 1/16, also Gen 1 only does 1/16
        inflictDamage(player, poke(player).totalLifePoints()/16, player);
        break;
    case Pokemon::Poisoned:
        notify(All, StatusMessage, player, qint8(HurtPoison));

        if (poke(player).statusCount() == 0)
            inflictDamage(player, poke(player).totalLifePoints()/16, player); // 1/16 in gen 1
        else {
            inflictDamage(player, poke(player).totalLifePoints() * (16-poke(player).statusCount()) / 16, player);
            poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1);
        }
        break;
    }

    if (poke(player).hasStatus(Pokemon::Seeded)) {
        //Leech Seed increases toxic count by 1
        if (poke(player).status() == Pokemon::Poisoned && poke(player).statusCount() != 0) {
            poke(player).statusCount() = std::max(1, poke(player).statusCount() - 1);
        }
        int source = opponent(player);

        if (!koed(source)) {
            sendMoveMessage(72, 2, player, Pokemon::Grass);
            inflictDamage(player, poke(player).totalLifePoints()/16, player);
            healLife(source, poke(player).totalLifePoints()/16);
        }
    }

    testWin();
}


void BattleRBY::inflictDamage(int player, int damage, int source, bool straightattack, bool goForSub)
{
    if (koed(player)) {
        return;
    }

    if (straightattack) {
        pokeMemory(source)["DamageInflicted"] = damage; //For bide
        battleMemory()["LastDamageTakenByAny"] = damage; //For Counter
    }

    bool sub = hasSubstitute(player);

    if (sub && (player != source || goForSub) && straightattack) {
        inflictSubDamage(player, damage, source);
    } else {
        damage = std::min(int(poke(player).lifePoints()), damage);

        int hp  = poke(player).lifePoints() - damage;

        if (hp <= 0) {
            koPoke(player, source, straightattack);
        } else {
            if (straightattack) {
                notify(this->player(player), StraightDamage, player, qint16(damage));
                notify(AllButPlayer, StraightDamage, player, qint16(damage*100/poke(player).totalLifePoints()));
            }

            changeHp(player, hp);
        }
    }

    if (straightattack && player != source) {
        if (!sub) {
            /* If there's a sub its already taken care of */
            turnMem(player).damageTaken = damage;
            callpeffects(player, source, "UponOffensiveDamageReceived");
        }

        if (damage > 0) {
            inflictRecoil(source, player);
        }
    }
}

void BattleRBY::useAttack(int player, int move, bool specialOccurence, bool tellPlayers)
{
    int oldAttacker = attacker();
    int oldAttacked = attacked();

    attacker() = player;

    int attack;
    int target = opponent(player);

    if (specialOccurence) {
        attack = move;
    } else {
        attack = this->move(player,move);
        fpoke(player).lastMoveSlot = move;
    }

    turnMem(player).add(TurnMemory::HasMoved);

    calleffects(player,player,"EvenWhenCantMove");

    if (!testStatus(player)) {
        goto trueend;
    }

    turnMem(player).add(TM::HasPassedStatus);
    //turnMemory(player)["MoveChosen"] = attack;

    if (!specialOccurence) {
        callpeffects(player, target, "MovePossible");
        if (turnMemory(player).contains("ImpossibleToMove")) {
            goto trueend;
        }
    }

    calleffects(player, target, "MoveSettings");

    if (!turnMem(player).contains(TM::BuildUp) && attack != 0 && attack != Move::Struggle) {
        fpoke(player).lastMoveUsed = attack;
        slotMemory(player).lastMoveUsed = attack;
    }

    notify(All, UseAttack, player, qint16(attack), !(tellPlayers && !turnMemory(player).contains("TellPlayers") && !turnMem(player).contains(TM::BuildUp)));

    if (tmove(player).targets == Move::User || tmove(player).targets == Move::All || tmove(player).targets == Move::Field) {
        target = player;
    }

    if (!turnMem(player).contains(TM::BuildUp)) {
        if (!specialOccurence) {
            /* Placed there. DamageReceived is used by Counter */
            pokeMemory(opponent(player)).remove("DamageReceived");

            losePP(player, move, 1);
        } else {
            if (turnMem(player).contains(TurnMemory::UsePP)) {
                /* Placed there. DamageReceived is used by Counter */
                pokeMemory(opponent(player)).remove("DamageReceived");

                losePP(player, fpoke(player).lastMoveSlot, 1);
            }
        }
    }

    attacked() = target;
    if (!specialOccurence && (tmove(player).flags & Move::MemorableFlag) ) {
        //pokeMemory(target)["MirrorMoveMemory"] = attack;
    }

    turnMem(player).remove(TM::Failed);
    turnMem(player).add(TM::FailingMessage);

    // Miss
    if (target != player && !testAccuracy(player, target)) {
        pokeMemory(player).remove("DamageInflicted");
        calleffects(player,target,"AttackSomehowFailed");
        battleMemory()["LastDamageTakenByAny"] = 0; //Counter fails if last move used missed
        goto trueend;
    }
    //fixme: try to get protect to work on a calleffects(target, player), and wide guard/priority guard on callteffects(this.player(target), player)
    /* Protect, ... */

    //A pokemon can only be thawed by a move that can inflict burn. You can thaw and burn on the same turn
    if (tmove(player).type == Type::Fire && poke(target).status() == Pokemon::Frozen && attack != Move::FireSpin) {
        unthaw(target);
    }

    if (tmove(player).power > 0 && player != target && !turnMem(player).contains(TM::BuildUp))
    {
        calculateTypeModStab();

        int typemod = turnMem(player).typeMod;
        if (typemod < -50 && ((tmove(player).power > 1 && attack != Move::Bind && attack != Move::Wrap) || (MoveInfo::isOHKO(attack, gen())))) {
            /* If it's ineffective we just say it */
            notify(All, Effective, target, quint8(0));
            calleffects(player,target,"AttackSomehowFailed");
            goto trueend;
        }

        calleffects(player, target, "DetermineAttackFailure");
        if (testFail(player)){
            calleffects(player,target,"AttackSomehowFailed");
            goto trueend;
        }

        int num = repeatNum(player);
        bool hit = num > 1;

        if (tmove(player).power > 1) {
            testCritical(player, target);
        }

        calleffects(player, target, "CustomAttackingDamage");

        int damage;
        if (turnMemory(player).contains("CustomDamage")) {
            damage = turnMemory(player).value("CustomDamage").toInt();
        } else if (MoveInfo::isOHKO(attack, gen())) {
            damage = poke(target).lifePoints();
        } else {
            damage = calculateDamage(player, target);
        }

        int hitcount = 0;

        for (repeatCount() = 0; repeatCount() < num && !koed(target) && (repeatCount()==0 || !koed(player)); repeatCount()+=1) {
            fpoke(target).remove(BasicPokeInfo::HadSubstitute);
            bool sub = hasSubstitute(target);
            if (sub) {
                fpoke(target).add(BasicPokeInfo::HadSubstitute);
            }

            if (tmove(player).power > 1 && repeatCount() == 0) {
                notify(All, Effective, target, quint8(typemod > 0 ? 8 : (typemod < 0 ? 2 : 4)));
            }

            /* Even though bind/wrap work when no effect, it doesn't inflict damage */
            if ( (attack != Move::Bind && attack != Move::Wrap) || typemod > -50) {
                inflictDamage(target, damage, player, true);
            }
            hitcount += 1;

            calleffects(player, target, "UponAttackSuccessful");

            /* A broken sub stops a multi-hit attack and draining moves don't heal */
            if (hadSubstitute(target)) {
                break;
            }
            healDamage(player, target);

            /* Secondary effect of an attack: like ancient power, acid, thunderbolt, ... */
            /* Twineedle can't poison on the first hit */
            if (attack != Move::Twineedle || repeatCount() > 0) {
                applyMoveStatMods(player, target);
            }

            if (!sub && !koed(target)) {
                testFlinch(player, target);
            }
            attackCount() += 1;
        }

        if (hit) {
            notifyHits(player, hitcount);
        }

        if (koed(target))
        {
            notifyKO(target);
        }

        fpoke(target).remove(BasicPokeInfo::HadSubstitute);
    } else {
        if (tmove(player).classification == Move::StatChangingMove || (tmove(player).classification == Move::StatusInducingMove && tmove(player).status == Pokemon::Asleep)) {
            battleMemory()["LastDamageTakenByAny"] = 0; //Counter damage resets on Sleep moves and Stat changing moves
        }
        /* Needs to be called before opponentblock because lightning rod / twave */
        int type = tmove(player).type; /* move type */

        if ( target != player &&
                ((Move::StatusInducingMove && tmove(player).status == Pokemon::Poisoned && hasType(target, Type::Poison)) ||
                 ((attack == Move::ThunderWave || attack == Move::Toxic || attack == Move::PoisonGas || attack == Move::PoisonPowder)
                  && TypeInfo::Eff(type, getType(target, 1)) * TypeInfo::Eff(type, getType(target, 2)) == 0))) {
            notify(All, Failed, player);
            goto trueend;
        }


        calleffects(player, target, "DetermineAttackFailure");
        if (testFail(player)){
            calleffects(player,target,"AttackSomehowFailed");
            goto trueend;
        }

        applyMoveStatMods(player, target);
        calleffects(player, target, "UponAttackSuccessful");

        /* Side change may switch player & target */
        if (attacker() != player) {
            player = attacker();
            target = attacked();
        }

        healDamage(player, target);
    }

    //pokeMemory(target)["LastAttackToHit"] = attack;

    trueend:

    calleffects(player,player,"TrueEnd");

    if (koed(player) && tmove(player).power > 0) {
        notifyKO(player);
    }

    attacker() = oldAttacker;
    attacked() = oldAttacked;
}

bool BattleRBY::hadSubstitute(int player)
{
    return !fpoke(player).substitute() && fpoke(player).is(BasicPokeInfo::HadSubstitute);
}

bool BattleRBY::testAccuracy(int player, int target, bool silent)
{
    int acc = tmove(player).accuracy;
    int tarChoice = tmove(player).targets;
    bool multiTar = tarChoice != Move::ChosenTarget && tarChoice != Move::RandomTarget;

    //OHKO
    int move = tmove(player).attack;

    //Sleep bypasses accuracy check on recharging turn if the target pokemon has another status
    if (poke(target).status() != Pokemon::Fine && poke(target).status() != Pokemon::Asleep) {
        if (pokeMemory(target).contains("Recharging") && tmove(player).status == Pokemon::Asleep) {
            acc = 0;
        }
    }

    //test for dig/fly here
    if (acc == 0 || acc == 101) {
        return true;
    }

    /* For deliberate misses, like with counter */
    if (acc < 0 || pokeMemory(target).value("Invulnerable").toBool()) {
        if (!silent) {
            notifyMiss(multiTar, player, target);
        }
        return false;
    }

    //Keep acc 1 for rage
    if (acc != 1) {
        acc = acc*255/100 + ((isStadium()) ? 1 : 0);
    }

    if (MoveInfo::isOHKO(move, gen())) {
        bool ret;

        if (getStat(player, Speed) < getStat(target, Speed)) {
            ret = false;
        } else {
            ret = coinflip(255*30/100, 256);
        }

        if (!ret && !silent) {
            notifyMiss(multiTar, player, target);
        }
        return ret;
    }

    acc = acc * getStatBoost(player, Accuracy) * getStatBoost(target, Evasion);

    if (coinflip(acc, 256)) {
        return true;
    } else {
        if (!silent) {
            notifyMiss(multiTar, player, target);
        }
        //Hi jump kick, jump kick
        return false;
    }
}


void BattleRBY::inflictRecoil(int source, int target)
{
    int recoil = tmove(source).recoil;

    if (recoil == 0)
        return;

    //Rockhead, MagicGuard
    if (koed(source)) {
        return;
    }

    // If move KOs opponent's pokemon, no recoil damage is applied in stadium.
    if (koed(target) && recoil < 0 && isStadium()) {
        return;
    }

    // If move defeats a sub, no recoil damage is applied in RBY.
    if (hadSubstitute(target)) {
        return;
    }

    notify(All, Recoil, recoil < 0 ? source : target, bool(recoil < 0));

    // "33" means one-third
    //if (recoil == -33) recoil = -100 / 3.; -- commented out until ingame confirmation

    int damage = std::abs(int(recoil * turnMem(target).damageTaken / 100));

    if (recoil < 0) {
        inflictDamage(source, damage, source, false);

        /* Self KO Clause! */
        if (koed(source)) {
            selfKoer() = source;
        }
    } else  {
        healLife(source, damage);
    }
}

void BattleRBY::callpeffects(int source, int target, const QString &name)
{
    if (pokeMemory(source).contains("Effect_" + name)) {
        turnMemory(source)["PokeEffectCall"] = true;
        QSet<QString> &effects = *pokeMemory(source).value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = pokeMemory(source).value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            /* If a pokemons dies from leechseed,its status changes, and so nightmare function would be removed
               but still be in the foreach, causing a crash */
            if(f)
                f(source, target, *this);
        }
        turnMemory(source)["PokeEffectCall"] = false;
    }
}

void BattleRBY::calleffects(int source, int target, const QString &name)
{
    context &turn = turnMemory(source);
    if (turn.contains("Effect_" + name)) {
        turn["TurnEffectCall"] = true;
        turn["TurnEffectCalled"] = name;
        QSet<QString> &effects = *turn.value("Effect_" + name).value<QSharedPointer<QSet<QString> > >();

        foreach(QString effect, effects) {
            MechanicsFunction f = turn.value("Effect_" + name + "_" + effect).value<MechanicsFunction>();

            if (f)
                f(source, target, *this);
        }
        turn["TurnEffectCall"] = false;
    }
}

void BattleRBY::setupMove(int i, int move)
{
    RBYMoveEffect::setup(move,i,0,*this);
}

void BattleRBY::losePP(int player, int move, int loss)
{
    int PP = this->PP(player, move) - loss;

    //RBY bug: PPs loop over
    if (PP < 0) {
        PP = 63;
    }

    changePP(player, move, PP);
}

int BattleRBY::calculateDamage(int p, int t)
{
    //PokeBattle &poke = this->poke(p);

    int level = fpoke(p).level;
    int attack, def;
    bool crit = turnMem(p).contains(TM::CriticalHit);
    int ch = 1 + crit;

    int attackused = tmove(p).attack;

    int cat = tmove(p).category;
    if (cat == Move::Physical) {
        attack = crit ? this->poke(p).normalStat(Attack) : getStat(p, Attack);
        def = crit ? this->poke(t).normalStat(Defense) : getStat(t, Defense);
    } else {
        attack = crit ? this->poke(p).normalStat(SpAttack) : getStat(p, SpAttack);
        def = crit ? this->poke(t).normalStat(SpAttack) : getStat(t, SpAttack);
    }

    /* Light screen / Reflect */
    // In RBY, Reflect / Light Screen boost doesn't cap the stat at 999 or 1023
    if (!crit && pokeMemory(t).value("Barrier" + QString::number(cat) + "Count").toInt() > 0) {
        def *= 2;
    }

    // In RBY, if either stat is higher than 255, both are quartered during damage calculation
    if (def > 255 || attack > 255) {
        def = (def / 4) % 256;
        if (def == 0)
            def = 1;

        attack = (attack / 4) % 256;
        if (attack == 0)
            attack = 1;
    }

    attack = std::min(attack, 65535);

    if (attackused == Move::Explosion || attackused == Move::Selfdestruct) {
        /* explosion / selfdestruct */
        def /= 2;
        if (def == 0)
            // prevent division by zero
            def = 1;
    }

    int stab = turnMem(p).stab;
    int typemod = turnMem(p).typeMod;
    int randnum = randint(39) + 217;
    int power = tmove(p).power;

    power = std::min(power, 65535);
    int damage = ((std::min(((level * ch * 2 / 5) + 2) * power, 65535) * attack / def) / 50);
    damage = std::min(damage, 997) + 2;

    damage = (damage * stab / 2) ;
    while (typemod > 0) {
        damage *= 2;
        typemod--;
    }
    while (typemod < 0) {
        damage /= 2;
        typemod++;
    }

    // In RBY, minimum damage after randnum is applied is 1,
    // unless it became 0 during typemod calculations.
    if (damage != 1) {
        damage = (damage * randnum) / 255;
    }

    return damage;
}


void BattleRBY::changeTempMove(int player, int slot, int move)
{
    fpoke(player).moves[slot] = move;
    notify(this->player(player), ChangeTempPoke, player, quint8(TempMove), quint8(slot), quint16(move));
    changePP(player,slot,MoveInfo::PP(move, gen()));
}

int BattleRBY::getBoostedStat(int p, int stat)
{
    return poke(p).normalStat(stat) * (floor(100*getStatBoost(p, stat))/100);
}

bool BattleRBY::loseStatMod(int player, int stat, int malus, int attacker, bool tell)
{
    if (attacker != player) {
        /* Mist only works on move purely based on stat changes, not on side effects, in gen 1 */
        if (pokeMemory(this->player(player)).contains("Misted") && tmove(attacker).power == 0) {
            sendMoveMessage(86, 2, player, Pokemon::Ice, player, tmove(attacker).attack);
            return false;
        }
    }

    int boost = fpoke(player).boosts[stat];
    if (boost > -6) {
        notify(All, StatChange, player, qint8(stat), qint8(-malus), !tell);
        changeStatMod(player, stat, std::max(boost-malus, -6));
    } else {
        notify(All, CappedStat, player, qint8(stat), false);
    }

    if (stat < Accuracy) {
        fpoke(player).stats[stat] = getBoostedStat(player, stat);
    }

    if (poke(player).status() == Pokemon::Burnt) {
        fpoke(player).stats[Attack] = std::max(fpoke(player).stats[Attack] / 2, 1);
    } else if (poke(player).status() == Pokemon::Paralysed) {
        fpoke(player).stats[Speed] = std::max(fpoke(player).stats[Speed] / 4, 1);
    }

    return true;
}


bool BattleRBY::gainStatMod(int player, int stat, int bonus, int, bool tell)
{
    int boost = fpoke(player).boosts[stat];
    if (boost < 6 && (getStat(player, stat) < 999 || stat == Evasion)) {
        notify(All, StatChange, player, qint8(stat), qint8(bonus), !tell);
        changeStatMod(player, stat, std::min(boost+bonus, 6));
    } else {
        notify(All, CappedStat, player, qint8(stat), true);
    }

    if (stat < Accuracy) {
        fpoke(player).stats[stat] = getBoostedStat(player, stat);
    }

    // RBY Doubles and Triples are dumb.
    // Actually they don't exist, but this is just in case.
    for (int i = 0; i < numberOfSlots(); i++) {
        if (i != player) {
            if (poke(i).status() == Pokemon::Burnt) {
                fpoke(i).stats[Attack] = std::max(fpoke(i).stats[Attack] / 2, 1);
            } else if (poke(i).status() == Pokemon::Paralysed) {
                fpoke(i).stats[Speed] = std::max(fpoke(i).stats[Speed] / 4, 1);
            }
        }
    }

    return true;
}
