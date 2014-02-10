#ifndef BATTLECLIENTLOG_H
#define BATTLECLIENTLOG_H

#include "battlecommandmanager.h"
#include "battledata.h"

struct BattleDynamicInfo;
class BattleDefaultTheme;
class DataContainer;

class BattleClientLog : public QObject, public BattleCommandManager<BattleClientLog>
{
    Q_OBJECT
public:
    BattleClientLog(BattleData<DataContainer> *data, BattleDefaultTheme *theme, bool logNames=true);

    void onKo(int spot);
    void onSendOut(int spot, int player, ShallowBattlePoke* pokemon, bool silent);
    void onSendBack(int spot, bool silent);
    void onUseAttack(int spot, int attack, bool silent);
    void onBeginTurn(int turn);
    void onHpChange(int spot, int newHp);
    void onHitCount(int spot, int count);
    void onEffectiveness(int spot, int effectiveness);
    void onCriticalHit(int spot);
    void onMiss(int spot);
    void onAvoid(int spot);
    void onStatBoost(int spot, int stat, int boost, bool silent);
    void onCappedStat(int spot, int stat, bool maxi);
    void onMajorStatusChange(int spot, int status, bool multipleTurns, bool silent);
    void onPokeballStatusChanged(int player, int poke, int status);
    void onStatusAlreadyThere(int spot, int status);
    void onStatusNotification(int spot, int status);
    void onStatusDamage(int spot, int status);
    void onStatusOver(int spot, int status);
    void onAttackFailing(int spot, bool silent);
    void onPlayerMessage(int spot, const QString &message);
    void onSpectatorJoin(int id, const QString &name);
    void onSpectatorLeave(int id);
    void onSpectatorChat(int id, const QString &message);
    void onMoveMessage(int spot, int move, int part, int type, int foe, int other, const QString &data);
    void onNoTarget(int spot);
    void onItemMessage(int spot, int item, int part, int foe, int berry, int other);
    void onFlinch(int spot);
    void onRecoil(int spot);
    void onDrained(int spot);
    void onStartWeather(int spot, int weather, bool ability);
    void onContinueWeather(int weather);
    void onEndWeather(int weather);
    void onHurtWeather(int spot, int weather);
    void onDamageDone(int spot, int damage);
    void onAbilityMessage(int spot, int ab, int part, int type, int foe, int other);
    void onSubstituteStatus(int spot, bool substitute);
    void onBlankMessage();
    void onClauseActivated(int clause);
    void onRatedNotification(bool rated);
    void onTierNotification(const QString &tier);
//    void onDynamicInfo(int spot, BattleDynamicInfo info);
//    void onPokemonVanish(int spot);
//    void onPokemonReappear(int spot);
//    void onSpriteChange(int spot, int newSprite);
//    void onDefiniteFormeChange(int spot, int poke, int newPoke);
//    void onCosmeticFormeChange(int spot, int subforme);
//    void onClockStart(int player, int time);
//    void onClockStop(int player, int time);
    void onShiftSpots(int player, int spot1, int spot2, bool silent);
    void onBattleEnd(int res, int winner);
    void onVariation(int player, int bonus, int malus);
    void onRearrangeTeam(int player, const ShallowShownTeam& team);
    void onPrintHtml(const QString &data);
    void onReconnect(int player);
    /* Disconnection of us */
    void onDisconnection();
    /* Disconnection of a certain player */
    void onDisconnect(int player);
    void onUseItem(int spot, int item);

    QString nick(int spot);
    QString rnick(int spot);

    void setTheme(BattleDefaultTheme *);

    /* Logs, but doesnt request a print */
    void pushHtml(const QString&);
    /* Requests a print, unless silent, in which case it calls pushHtml and add comments */
    void printHtml(const QString &, const QString&);
    void printLine(const QString &, const QString&, bool silent =false);
    void printSilent(const QString&);

    QStringList getLog();
    void emitAll();
signals:
    void lineToBePrinted(const QString &);
protected:
    BattleData<DataContainer> *mData;
    BattleData<DataContainer> *data();
    BattleDefaultTheme *mTheme;
    BattleDefaultTheme *theme();

    QStringList log;
    bool hasLoggedTeams;
    QHash<int, QString> spectators;
    bool blankMessage;
    bool mLogNames;
};

#endif // BATTLECLIENTLOG_H
