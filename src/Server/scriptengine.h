#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore>
#include <QTextEdit>

#include <QtScript>

#include <QScriptValueIterator>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHostInfo>

#include "../PokemonInfo/pokemonstructs.h"
#include "sessiondatafactory.h"

class Server;
class ChallengeInfo;

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    ScriptEngine(Server *s);
    ~ScriptEngine();

    /* Events */

    bool beforeSpectateBattle(int src, int p1, int p2);
    void afterSpectateBattle(int src, int p1, int p2);
    bool attemptToSpectateBattle(int src, int p1, int p2);
    void stepEvent();

    bool beforeChatMessage(int src, const QString &message, int channel);
    void afterChatMessage(int src, const QString &message, int channel);
    bool beforeNewMessage(const QString &message);
    void afterNewMessage(const QString &message);
    void serverStartUp();
    void serverShutDown();
    void beforeLogOut(int src);
    void afterLogOut(int src);
    bool beforeLogIn(int src);
    void afterLogIn(int src);
    bool beforeChannelCreated(int channelid, const QString &channelname, int playerid);
    void afterChannelCreated(int channelid, const QString &channelname, int playerid);
    bool beforeChannelDestroyed(int channelid);
    void afterChannelDestroyed(int channelid);
    bool beforeChannelJoin(int src, int channelid);
    void afterChannelJoin(int src, int channelid);
    void beforeChannelLeave(int src, int channelid);
    void afterChannelLeave(int src, int channelid);
    void beforeChangeTeam(int src);
    void afterChangeTeam(int src);
    bool beforeChangeTier(int src, const QString& oldTier, const QString &newTier);
    void afterChangeTier(int src, const QString& oldTier, const QString &newTier);
    bool beforeChallengeIssued(int src, int dest, const ChallengeInfo &desc);
    void afterChallengeIssued(int src, int dest, const ChallengeInfo &desc);
    bool beforeBattleMatchup(int src, int dest, const ChallengeInfo &desc);
    void afterBattleMatchup(int src, int dest, const ChallengeInfo &desc);
    void beforeBattleStarted(int src, int dest, const ChallengeInfo &desc, int battleid);
    void afterBattleStarted(int winner, int loser, const ChallengeInfo &desc, int battleid);
    void beforeBattleEnded(int winner, int loser, int desc, int battleid);
    void afterBattleEnded(int winner, int loser, int desc, int battleid);
    bool beforePlayerAway(int src, bool away);
    void afterPlayerAway(int src, bool away);
    bool beforePlayerKick(int src, int dest);
    void afterPlayerKick(int src, int dest);
    bool beforePlayerBan(int src, int dest);
    void afterPlayerBan(int src, int dest);
    void battleSetup(int src, int dest, int battleId);
    bool beforeFindBattle(int src);
    void afterFindBattle(int src);

    /* Imports a module with a given name */
    Q_INVOKABLE QScriptValue import(const QString &fileName);
    /* Functions called in scripts */
    Q_INVOKABLE void sendAll(const QString &mess);
    Q_INVOKABLE void sendHtmlAll(const QString &mess);
    Q_INVOKABLE void sendAll(const QString &mess, int channel);
    Q_INVOKABLE void sendHtmlAll(const QString &mess, int channel);
    Q_INVOKABLE void kick(int id);
    Q_INVOKABLE void kick(int playerid, int chanid);
    /* If you edited his team, updates it for the rest of the world */
    Q_INVOKABLE void updatePlayer(int playerid);
    Q_INVOKABLE void putInChannel(int playerid, int chanid);
    Q_INVOKABLE QScriptValue createChannel(const QString &channame);
    Q_INVOKABLE QScriptValue getAnnouncement();
    Q_INVOKABLE QScriptValue getColor(int id);
    Q_INVOKABLE void setAnnouncement(const QString &html, int id);
    Q_INVOKABLE void setAnnouncement(const QString &html);
    Q_INVOKABLE void changeAnnouncement(const QString &html);
    Q_INVOKABLE void makeServerPublic(bool isPublic);

    // Q_INVOKABLE void setTimer(int ms); // Causes crash

    /* Prevents the event from happening.
       For exemple, if called in 'beforeChatMessage', the message won't appear.
       If called in 'beforeChallengeIssued', the challenge won't be issued.
       */
    Q_INVOKABLE void stopEvent();

    Q_INVOKABLE void shutDown();
    Q_INVOKABLE void sendMessage(int id, const QString &mess);
    Q_INVOKABLE void sendMessage(int id, const QString &mess, int channel);
    Q_INVOKABLE void sendHtmlMessage(int id, const QString &mess);
    Q_INVOKABLE void sendHtmlMessage(int id, const QString &mess, int channel);
    /* Print on the server. Useful for debug purposes */
    Q_INVOKABLE void print(QScriptContext *context, QScriptEngine *engine);
    Q_INVOKABLE void clearPass(const QString &name);
    Q_INVOKABLE void changeAuth(int id, int auth);
    Q_INVOKABLE void changeDbAuth(const QString &name, int auth);
    Q_INVOKABLE void changeAway(int id, bool away);
    Q_INVOKABLE void changeRating(const QString& name, const QString& tier, int newRating);
    Q_INVOKABLE void changePokeLevel(int id, int slot, int level);
    Q_INVOKABLE void changePokeNum(int id, int slot, int num);
    Q_INVOKABLE void changePokeItem(int id, int slot, int item);
    Q_INVOKABLE void changePokeMove(int id, int pokeslot, int moveslot, int move);
    Q_INVOKABLE void changePokeGender(int id, int pokeslot, int gender);
    Q_INVOKABLE void changePokeName(int id, int pokeslot, const QString &name);
    Q_INVOKABLE void changeTier(int id, const QString &tier);
    Q_INVOKABLE void reloadTiers();
    /* Export the SQL databases to old style txt files */
    Q_INVOKABLE void exportMemberDatabase();
    Q_INVOKABLE void exportTierDatabase();
    /* Updates the rankings. Very time consuming, be aware... ! */
    Q_INVOKABLE void updateRatings();
    /* Resets a tier's ladders */
    Q_INVOKABLE void resetLadder(const QString &tier);
    Q_INVOKABLE void synchronizeTierWithSQL(const QString &tier);

    Q_INVOKABLE void clearChat();
    /* Accepts string as 1st parameter. */
    Q_INVOKABLE void callLater(const QString &s, int delay);
    Q_INVOKABLE void callQuickly(const QString &s, int delay);
    /* Accepts function as 1st parameter. */
    Q_INVOKABLE void delayedCall(const QScriptValue &func, int delay);
    /* Evaluates the script given in parameter */
    Q_INVOKABLE QScriptValue eval(const QString &script);

    Q_INVOKABLE QScriptValue channelIds();
    Q_INVOKABLE QScriptValue channel(int id);
    Q_INVOKABLE QScriptValue channelId(const QString &name);
    Q_INVOKABLE QScriptValue channelsOfPlayer(int playerid);
    Q_INVOKABLE QScriptValue playersOfChannel(int channelid);
    Q_INVOKABLE bool existChannel(const QString &channame);
    Q_INVOKABLE bool isInChannel(int playerid, int channelid);
    Q_INVOKABLE bool isInSameChannel(int player1, int player2);

    Q_INVOKABLE QScriptValue playerIds();
    Q_INVOKABLE QScriptValue name(int id);
    Q_INVOKABLE QScriptValue id(const QString& name);
    Q_INVOKABLE QScriptValue auth(int id);
    Q_INVOKABLE QScriptValue battling(int id);
    Q_INVOKABLE QScriptValue away(int id);
    Q_INVOKABLE QScriptValue ip(int id); 
    Q_INVOKABLE QScriptValue proxyIp(int id);
    Q_INVOKABLE void hostName(const QString &ip, const QScriptValue &function);
    Q_INVOKABLE QScriptValue gen(int id);
    Q_INVOKABLE QScriptValue dbAuth(const QString &name);
    Q_INVOKABLE QScriptValue dbAuths();
    Q_INVOKABLE QScriptValue dbAll();
    Q_INVOKABLE QScriptValue dbIp(const QString &name);
    Q_INVOKABLE QScriptValue dbDelete(const QString &name);
    Q_INVOKABLE QScriptValue dbLastOn(const QString &name);
    Q_INVOKABLE bool dbRegistered(const QString &name);
    Q_INVOKABLE QScriptValue tier(int id);
    Q_INVOKABLE QScriptValue ranking(int id);
    Q_INVOKABLE QScriptValue ratedBattles(int id);
    Q_INVOKABLE QScriptValue ranking(const QString &name, const QString &tier);
    Q_INVOKABLE QScriptValue ratedBattles(const QString &name, const QString &tier);
    Q_INVOKABLE int maxAuth(const QString &ip);
    Q_INVOKABLE QScriptValue aliases(const QString &ip);
    Q_INVOKABLE QScriptValue totalPlayersByTier(const QString &tier);
    Q_INVOKABLE QScriptValue ladderEnabled(int id);
    Q_INVOKABLE QScriptValue ladderRating(int id, const QString &tier = QString());
    /* returns a state of the memory, useful to check for memory leaks and memory usage */
    Q_INVOKABLE QScriptValue memoryDump();
    Q_INVOKABLE bool hasLegalTeamForTier(int id, const QString &tier);
    Q_INVOKABLE void changeName(int playerId, QString newName);
    Q_INVOKABLE void changeInfo(int playerId, QString newInfo);
    Q_INVOKABLE QScriptValue info(int playerId);
    Q_INVOKABLE void changeAvatar(int playerId, quint16 avatarId);
    Q_INVOKABLE QScriptValue avatar(int playerId);

    Q_INVOKABLE QScriptValue pokemon(int num);
    Q_INVOKABLE QScriptValue pokeNum(const QString &name);
    Q_INVOKABLE QScriptValue move(int num);
    Q_INVOKABLE QScriptValue moveNum(const QString &name);
    Q_INVOKABLE int moveType(int moveNum, int gen = GEN_MAX);
    Q_INVOKABLE QScriptValue item(int num);
    Q_INVOKABLE QScriptValue itemNum(const QString &item);
    Q_INVOKABLE QScriptValue nature(int num);
    Q_INVOKABLE QScriptValue natureNum(const QString &nature);
    Q_INVOKABLE QScriptValue ability(int num);
    Q_INVOKABLE QScriptValue abilityNum(const QString &nature);
    Q_INVOKABLE QScriptValue genderNum(QString genderName);
    Q_INVOKABLE QString gender(int genderNum);

    Q_INVOKABLE QScriptValue teamPokeLevel(int id, int slot);
    Q_INVOKABLE QScriptValue teamPoke(int id, int index);
    Q_INVOKABLE bool hasTeamPoke(int id, int pokemonnum);
    Q_INVOKABLE QScriptValue indexOfTeamPoke(int id, int pokenum);
    Q_INVOKABLE bool hasDreamWorldAbility(int id, int slot);
    Q_INVOKABLE bool compatibleAsDreamWorldEvent(int id, int slot);

    Q_INVOKABLE QScriptValue teamPokeMove(int id, int pokeindex, int moveindex);
    Q_INVOKABLE bool hasTeamPokeMove(int id, int pokeindex, int movenum);
    Q_INVOKABLE QScriptValue indexOfTeamPokeMove(int id, int pokeindex, int movenum);
    Q_INVOKABLE bool hasTeamMove(int id, int movenum);

    Q_INVOKABLE QScriptValue teamPokeItem(int id, int pokeindex);
    Q_INVOKABLE bool hasTeamItem(int id, int itemNum);

    Q_INVOKABLE QScriptValue teamPokeNature(int id, int slot);
    Q_INVOKABLE QScriptValue teamPokeEV(int id, int slot, int stat);
    Q_INVOKABLE QScriptValue teamPokeDV(int id, int slot, int stat);
    Q_INVOKABLE void setTeamPokeDV(int id, int slot, int stat, int newValue);
    Q_INVOKABLE void changeTeamPokeIV(int id, int slot, int stat, int newValue);
    Q_INVOKABLE void changeTeamPokeEV(int id, int slot, int stat, int newValue);

    Q_INVOKABLE int numPlayers();
    Q_INVOKABLE bool loggedIn(int id);

    Q_INVOKABLE int rand(int min, int max);
    Q_INVOKABLE long time();
    Q_INVOKABLE QScriptValue getTierList();

    Q_INVOKABLE void modifyTypeChart(int type_attack, int type_defend, const QString &modifier);
    Q_INVOKABLE QScriptValue type(int id);
    Q_INVOKABLE QScriptValue typeNum(const QString &typeName);

    Q_INVOKABLE int hiddenPowerType(int gen, quint8 hpdv, quint8 attdv, quint8 defdv, quint8 spddv, quint8 sattdv, quint8 sdefdv);

    Q_INVOKABLE QScriptValue getScript();

    Q_INVOKABLE int pokeType1(int id, int gen = GEN_MAX);
    Q_INVOKABLE int pokeType2(int id, int gen = GEN_MAX);

    Q_INVOKABLE void modifyMovePower(int moveNum, unsigned char power, int gen = GEN_MAX);
    Q_INVOKABLE void modifyMoveAccuracy(int moveNum, char accuracy, int gen = GEN_MAX);
    Q_INVOKABLE void modifyMovePP(int moveNum, char pp, int gen = GEN_MAX);
    Q_INVOKABLE void modifyMovePriority(int moveNum, qint8 priority, int gen = GEN_MAX);
   
    Q_INVOKABLE QScriptValue banList();
    Q_INVOKABLE void ban(QString name);
    Q_INVOKABLE void unban(QString name);

    Q_INVOKABLE void prepareWeather(int battleId, int weatherId);
    Q_INVOKABLE QScriptValue weatherNum(const QString &weatherName);
    Q_INVOKABLE QScriptValue weather(int weatherId);

    Q_INVOKABLE int teamPokeAbility(int id, int slot);
    Q_INVOKABLE void modifyPokeAbility(int id, int slot, int ability, int gen = GEN_MAX);
    Q_INVOKABLE void changePokeAbility(int id, int slot, int ability);
    Q_INVOKABLE QScriptValue pokeAbility(int poke, int slot, int gen = GEN_MAX);
    Q_INVOKABLE void changePokeHappiness(int id, int slot, int value);
    Q_INVOKABLE void changePokeShine(int id, int slot, bool value);
    Q_INVOKABLE QScriptValue teamPokeShine(int id, int slot);
    Q_INVOKABLE void changePokeNature(int id, int pokeslot, int nature);
    Q_INVOKABLE QScriptValue teamPokeGender(int id, int slot);

    Q_INVOKABLE QScriptValue teamPokeNick(int id, int pokeslot);

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);

    Q_INVOKABLE void inflictStatus(int battleId, bool toFirstPlayer, int slot, int status);
    Q_INVOKABLE void modifyPokeStat(int poke, int stat, quint8 value);

    Q_INVOKABLE void forceBattle(int player1, int player2, int clauses, int mode, bool is_rated = false);
    Q_INVOKABLE int getClauses(const QString &tier);
    Q_INVOKABLE QString serverVersion();
    Q_INVOKABLE bool isServerPrivate();

    /* Internal use only */
    Q_INVOKABLE void sendNetworkCommand(int id, int command);
    
// Potentially unsafe functions.
#ifndef PO_SCRIPT_SAFE_ONLY
    /* Save vals using the QSettings (persistent vals, that stay after the shutdown of the server */
    Q_INVOKABLE void saveVal(const QString &key, const QVariant &val);
    Q_INVOKABLE void saveVal(const QString &file, const QString &key, const QVariant &val);
    Q_INVOKABLE void removeVal(const QString &key);
    Q_INVOKABLE void removeVal(const QString &file, const QString &key);
    Q_INVOKABLE QScriptValue getVal(const QString &key);
    Q_INVOKABLE QScriptValue getVal(const QString &file, const QString &key);
    // Returns an array of Script_* key names in config.
    Q_INVOKABLE QScriptValue getValKeys();
    Q_INVOKABLE QScriptValue getValKeys(const QString &file);
    // Direct file access.
    Q_INVOKABLE void appendToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void writeToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void deleteFile(const QString &fileName);
    Q_INVOKABLE QScriptValue getFileContent(const QString &path);
    /* GET call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QScriptValue &callback);
    /* POST call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QScriptValue &callback, const QScriptValue &params_array);
    /* synchronous GET call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring);
    /* synchronous POST call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring, const QScriptValue &params_array);
#endif // PO_SCRIPT_SAFE_ONLY

#if !defined(PO_SCRIPT_NO_SYSTEM) && !defined(PO_SCRIPT_SAFE_ONLY)
    /* Calls the underlying OS for a command */
    Q_INVOKABLE int system(const QString &command);
#endif // PO_SCRIPT_NO_SYSTEM

signals:
    void clearTheChat();
public slots:
    void changeScript(const QString &script, const bool triggerStartUp = false);

private slots:
    void timer();
    void timer_step();
    void timerFunc();
#ifndef PO_SCRIPT_SAFE_ONLY
    void webCall_replyFinished(QNetworkReply* reply);
    void synchronousWebCall_replyFinished(QNetworkReply* reply);
#endif
    void hostInfo_Ready(const QHostInfo &myInfo);
    
private:
    Server *myserver;
    QScriptEngine myengine;
    QScriptValue myscript;
    QTimer * step_timer;
    QVector<bool> stopevents;
    SessionDataFactory *mySessionDataFactory;

    QNetworkAccessManager manager;
    QHash<QTimer*,QString> timerEvents;
    QHash<QTimer*,QScriptValue> timerEventsFunc;
    QHash<QNetworkReply*,QScriptValue> webCallEvents;
    QHash<int,QScriptValue> myHostLookups;

    void startStopEvent() {stopevents.push_back(false);}
    bool endStopEvent() {
        bool res = stopevents.back();
        stopevents.pop_back();
        return res;
    }

    QEventLoop sync_loop;
    QString sync_data;

    void evaluate(const QScriptValue &expr);
    void printLine(const QString &s);

    bool testPlayer(const QString &function, int id);
    bool testChannel(const QString &function, int id);
    bool testPlayerInChannel(const QString &function, int id, int chan);
    bool testRange(const QString &function, int val, int min, int max);
    void warn(const QString &function, const QString &message);

    template<class T>
    void makeEvent(const QString &event, const T& param);
    template<class T, class T2>
    void makeEvent(const QString &event, const T &param, const T2 &param2);
    template<class T, class T2, class T3>
    void makeEvent(const QString &event, const T& param, const T2 &param2, const T3 &param3);
    template<class T>
    bool makeSEvent(const QString &event, const T& param);
    template<class T, class T2>
    bool makeSEvent(const QString &event, const T &param, const T2 &param2);
    template<class T, class T2, class T3>
    bool makeSEvent(const QString &event, const T& param, const T2 &param2, const T3 &param3);
};

class ScriptWindow : public QWidget
{
    Q_OBJECT
public:
    ScriptWindow();
signals:
    void scriptChanged(const QString &script);
public slots:
    void okPressed();
    void gotoLine();
private:
    QTextEdit *myedit;
};

template<class T>
void ScriptEngine::makeEvent(const QString &event, const T &param)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property(event).call(myscript, QScriptValueList() << param));
}

template<class T, class T2>
void ScriptEngine::makeEvent(const QString &event, const T &param, const T2 &param2)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property(event).call(myscript, QScriptValueList() << param << param2));
}

template<class T, class T2, class T3>
void ScriptEngine::makeEvent(const QString &event, const T &param, const T2 &param2, const T3 &param3)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property(event).call(myscript, QScriptValueList() << param << param2 << param3));
}

template<class T>
bool ScriptEngine::makeSEvent(const QString &event, const T &param)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property(event).call(myscript, QScriptValueList() << param));

    return !endStopEvent();
}

template<class T, class T2>
bool ScriptEngine::makeSEvent(const QString &event, const T &param, const T2 &param2)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property(event).call(myscript, QScriptValueList() << param << param2));

    return !endStopEvent();
}

template<class T, class T2, class T3>
bool ScriptEngine::makeSEvent(const QString &event, const T &param, const T2 &param2, const T3 &param3)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property(event).call(myscript, QScriptValueList() << param << param2 << param3));

    return !endStopEvent();
}

#endif // SCRIPTENGINE_H
