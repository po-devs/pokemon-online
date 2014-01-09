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

#include <PokemonInfo/geninfo.h>
#include <Utilities/functions.h>

class Server;
class ChallengeInfo;
class SessionDataFactory;

class ScriptEngine : public QObject
{
    Q_OBJECT
    friend class ScriptEngineAgent;

public:
    static QScriptValue enableStrict(QScriptContext *, QScriptEngine *e);

#ifndef PO_SCRIPT_SAFE_ONLY
    static QScriptValue cwd(QScriptContext *c, QScriptEngine *e);
    static QScriptValue exists(QScriptContext *c, QScriptEngine *e);

    static QScriptValue writeConcat(QScriptContext *c, QScriptEngine *e);
    static QScriptValue write(QScriptContext *c, QScriptEngine *e);
    static QScriptValue read(QScriptContext *c, QScriptEngine *e);

    static QScriptValue rm(QScriptContext *c, QScriptEngine *e);

    static QScriptValue mkdir(QScriptContext *c, QScriptEngine *);
    static QScriptValue rmdir(QScriptContext *c, QScriptEngine *);

    static QScriptValue writeObject(QScriptContext *c, QScriptEngine *e);
    static QScriptValue readObject(QScriptContext *c, QScriptEngine *e);

    static QScriptValue writeFlat(QScriptContext *c, QScriptEngine *e);
    static QScriptValue readFlat(QScriptContext *c, QScriptEngine *e);

    static QScriptValue exec(QScriptContext *c, QScriptEngine *e);
#endif
    static QScriptValue backtrace(QScriptContext *c, QScriptEngine *);

    static QScriptValue sendAll(QScriptContext *c, QScriptEngine *);
    static QScriptValue sendMessage(QScriptContext *c, QScriptEngine *);
    static QScriptValue broadcast(QScriptContext *c, QScriptEngine *);

    static QScriptValue lighter(QScriptContext *c, QScriptEngine *);
    static QScriptValue darker(QScriptContext *c, QScriptEngine *);
    static QScriptValue lightness(QScriptContext *c, QScriptEngine *);
    static QScriptValue tint(QScriptContext *c, QScriptEngine *);
public:
    ScriptEngine(Server *s);
    ~ScriptEngine();

    /* Events */
    bool beforeSpectateBattle(int src, int p1, int p2);
    void afterSpectateBattle(int src, int p1, int p2);
    bool attemptToSpectateBattle(int src, int p1, int p2);
    void stepEvent();

    void serverStartUp();
    void serverShutDown();

    bool beforePlayerRegister(int src);

    bool beforeServerMessage(const QString &message);
    void afterServerMessage(const QString &message);

    bool beforeChatMessage(int src, const QString &message, int channel);
    void afterChatMessage(int src, const QString &message, int channel);

    bool beforeNewMessage(const QString &message);
    void afterNewMessage(const QString &message);

    /* When a PM window is opened the first by src time towards another player */
    bool beforeNewPM(int src);

    void beforeLogOut(int src);
    void afterLogOut(int src);

    bool beforeIPConnected(const QString &ip);

    bool beforeLogIn(int src, const QString &defaultChan);
    void afterLogIn(int src, const QString &defaultChan);

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

    bool beforeChangeTier(int src, int teamSlot, const QString& oldTier, const QString &newTier);
    void afterChangeTier(int src, int teamSlot, const QString& oldTier, const QString &newTier);

    bool beforeChallengeIssued(int src, int dest, const ChallengeInfo &desc);
    void afterChallengeIssued(int src, int dest, const ChallengeInfo &desc);

    bool beforeBattleMatchup(int src, int dest, const ChallengeInfo &desc);
    void afterBattleMatchup(int src, int dest, const ChallengeInfo &desc);

    void beforeBattleStarted(int src, int dest, const ChallengeInfo &desc, int battleid, int team1, int team2);
    void afterBattleStarted(int winner, int loser, const ChallengeInfo &desc, int battleid, int team1, int team2);

    void beforeBattleEnded(int winner, int loser, int desc, int battleid);
    void afterBattleEnded(int winner, int loser, int desc, int battleid);

    bool beforePlayerAway(int src, bool away);
    void afterPlayerAway(int src, bool away);

    bool beforePlayerKick(int src, int dest);
    void afterPlayerKick(int src, int dest);

    bool beforePlayerBan(int src, int dest, int time);
    void afterPlayerBan(int src, int dest, int time);

    bool beforeReconnect(int dest, int src);
    void afterReconnect(int src);

    void battleSetup(int src, int dest, int battleId);

    bool beforeFindBattle(int src);
    void afterFindBattle(int src);

    /* Imports a module with a given name */
    Q_INVOKABLE QScriptValue import(const QString &fileName);
    /* Functions called in scripts */

    // TODO: convert these crappy functions:
    Q_INVOKABLE void sendHtmlAll(const QString &mess);
    Q_INVOKABLE void sendHtmlAll(const QString &mess, int channel);
    Q_INVOKABLE void sendHtmlMessage(int id, const QString &mess);
    Q_INVOKABLE void sendHtmlMessage(int id, const QString &mess, int channel);

    Q_INVOKABLE void kick(int id);
    Q_INVOKABLE void kick(int playerid, int chanid);

    Q_INVOKABLE void disconnect(int id); //Disconnects a player. (He can reconnect with all his data)
    /* If you edited his info, updates it for the rest of the world */
    Q_INVOKABLE void updatePlayer(int playerid);
    Q_INVOKABLE void putInChannel(int playerid, int chanid);
    Q_INVOKABLE QScriptValue createChannel(const QString &channame);
    Q_INVOKABLE QScriptValue getAnnouncement();
    Q_INVOKABLE void changeColor(int id, const QString &color);
    Q_INVOKABLE QScriptValue getColor(int id);

    Q_INVOKABLE void setAnnouncement(const QString &html, int id);
    Q_INVOKABLE void setAnnouncement(const QString &html);
    Q_INVOKABLE void changeAnnouncement(const QString &html);

    Q_INVOKABLE QString getDescription();
    Q_INVOKABLE void changeDescription(const QString &html);

    Q_INVOKABLE void makeServerPublic(bool isPublic);

    /* Prevents the event from happening.
       For exemple, if called in 'beforeChatMessage', the message won't appear.
       If called in 'beforeChallengeIssued', the challenge won't be issued.
       */
    Q_INVOKABLE void stopEvent();

    Q_INVOKABLE void shutDown();

    /* Print on the server. Useful for debug purposes */
    Q_INVOKABLE void print(QScriptContext *context, QScriptEngine *engine);
    Q_INVOKABLE void clearPass(const QString &name);
    Q_INVOKABLE void changeAuth(int id, int auth);
    Q_INVOKABLE void changeDbAuth(const QString &name, int auth);
    Q_INVOKABLE void changeAway(int id, bool away);

    Q_INVOKABLE void changeRating(const QString& name, const QString& tier, int newRating);
    Q_INVOKABLE void changePokeLevel(int id, int team, int slot, int level);
    Q_INVOKABLE void changePokeNum(int id, int team, int slot, int num);
    Q_INVOKABLE void changePokeItem(int id, int team, int slot, int item);
    Q_INVOKABLE void changePokeMove(int id, int team, int pokeslot, int moveslot, int move);
    Q_INVOKABLE void changePokeGender(int id, int team, int pokeslot, int gender);
    Q_INVOKABLE void changePokeName(int id, int team, int pokeslot, const QString &name);
    Q_INVOKABLE void changePokeHp(int id, int team, int slot, int hp);
    Q_INVOKABLE void changePokeStatus(int id, int team, int slot, int status);
    Q_INVOKABLE void changePokePP(int id, int team, int slot, int moveslot, int PP);

    Q_INVOKABLE void changeTier(int id, int team, const QString &tier);
    Q_INVOKABLE void reloadTiers();
    /* Export the SQL databases to old style txt files */
    Q_INVOKABLE void exportMemberDatabase();
    Q_INVOKABLE void exportTierDatabase();
    /* Updates the rankings. Very time consuming, be aware... ! */
    Q_INVOKABLE void updateRatings();
    /* Updates the database and delete inactive players. Very time consuming, be aware... ! */
    Q_INVOKABLE void updateDatabase();
    /* Resets a tier's ladders */
    Q_INVOKABLE void resetLadder(const QString &tier);

    Q_INVOKABLE void clearChat();

    /* 2 timer functions to replace the other 7 */
    Q_INVOKABLE int setTimer(const QScriptValue &v, int delay, bool repeats);
    Q_INVOKABLE bool unsetTimer(int timerId);
    Q_INVOKABLE int unsetAllTimers();

    /* Accepts string as 1st parameter. */
    Q_INVOKABLE int callLater(const QString &s, int delay); // DEPRECATED
    Q_INVOKABLE int callQuickly(const QString &s, int delay); // DEPRECATED


    /* Accepts function as 1st parameter. */
    Q_INVOKABLE int quickCall(const QScriptValue &func, int delay); // DEPRECATED
    Q_INVOKABLE int delayedCall(const QScriptValue &func, int delay); // DEPRECATED

    /* Interval timers. */

    Q_INVOKABLE int intervalTimer(const QString &expr, int delay); // DEPRECATED
    Q_INVOKABLE int intervalCall(const QScriptValue &func, int delay); // DEPRECATED

    /* Stops a timer. */
    Q_INVOKABLE bool stopTimer(int timerId); // RENAMED/DEPRECATED

    /* Evaluates the script given in parameter */
    Q_INVOKABLE QScriptValue eval(const QString &script);
    Q_INVOKABLE QScriptValue eval(const QString &script, const QString &fname);

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
    Q_INVOKABLE QScriptValue battlingIds();
    Q_INVOKABLE QScriptValue away(int id);
    Q_INVOKABLE QScriptValue ip(int id); 
    Q_INVOKABLE QScriptValue proxyIp(int id);
    Q_INVOKABLE void hostName(const QString &ip, const QScriptValue &function);
    Q_INVOKABLE QScriptValue gen(int id, int team);
    Q_INVOKABLE QScriptValue subgen(int id, int team);
    Q_INVOKABLE QScriptValue teamCount(int id);
    Q_INVOKABLE QScriptValue generation(int genNum, int subNum);
    Q_INVOKABLE QScriptValue dbAuth(const QString &name);
    Q_INVOKABLE QScriptValue dbAuths();
    Q_INVOKABLE QScriptValue dbAll();
    Q_INVOKABLE QScriptValue dbIp(const QString &name);
    Q_INVOKABLE QScriptValue dbDelete(const QString &name);
    Q_INVOKABLE bool dbLoaded(const QString &name);
    Q_INVOKABLE bool dbExists(const QString &name);
    Q_INVOKABLE void dbClearCache();
    Q_INVOKABLE QScriptValue dbLastOn(const QString &name);
    Q_INVOKABLE QScriptValue dbExpire(const QString &name);
    Q_INVOKABLE QScriptValue dbTempBanTime(const QString &name);
    Q_INVOKABLE int dbExpiration();
    Q_INVOKABLE bool dbRegistered(const QString &name);
    Q_INVOKABLE QScriptValue tier(int id, int team);
    Q_INVOKABLE bool hasTier(int id, const QString &tier);
    Q_INVOKABLE QScriptValue ranking(int id, int team);
    Q_INVOKABLE QScriptValue ratedBattles(int id, int team);
    Q_INVOKABLE QScriptValue ranking(const QString &name, const QString &tier);
    Q_INVOKABLE QScriptValue ratedBattles(const QString &name, const QString &tier);
    Q_INVOKABLE int maxAuth(const QString &ip);
    Q_INVOKABLE QScriptValue aliases(const QString &ip);

    /* Returns the number of connections currently online for the IP asked */
    Q_INVOKABLE int connections(const QString &ip);
    Q_INVOKABLE int numRegistered(const QString &ip);

    Q_INVOKABLE QScriptValue totalPlayersByTier(const QString &tier);
    Q_INVOKABLE QScriptValue ladderEnabled(int id);
    Q_INVOKABLE QScriptValue ladderRating(int id, const QString &tier = QString());
    /* returns a state of the memory, useful to check for memory leaks and memory usage */
    Q_INVOKABLE QScriptValue memoryDump();
    Q_INVOKABLE QString profileDump();
    Q_INVOKABLE void resetProfiling();
    Q_INVOKABLE QScriptValue dosChannel();
    Q_INVOKABLE void changeDosChannel(const QString &newChannel);
    /* Removes the history of kicks and logins for all the IPs */
    Q_INVOKABLE void clearDosData();
    Q_INVOKABLE void reloadDosSettings();
    Q_INVOKABLE QScriptValue currentMod();
    Q_INVOKABLE QScriptValue currentModPath();
    Q_INVOKABLE QScriptValue dataRepo();
    /* Counts the number of players in a disconnected state - not wholly removed yet in hope they might reconnect */
    Q_INVOKABLE int disconnectedPlayers();
    Q_INVOKABLE bool hasLegalTeamForTier(int id, int team, const QString &tier);
    Q_INVOKABLE void changeName(int playerId, QString newName);
    Q_INVOKABLE void changeInfo(int playerId, QString newInfo);
    Q_INVOKABLE QScriptValue info(int playerId);
    Q_INVOKABLE void changeAvatar(int playerId, quint16 avatarId);
    Q_INVOKABLE QScriptValue avatar(int playerId);

    // Overloaded function os.
    // First (no parameters) returns the server's os.
    // Second (with a param) returns a player's os.
    Q_INVOKABLE QScriptValue os();
    Q_INVOKABLE QScriptValue os(int playerId);

    Q_INVOKABLE QScriptValue pokemon(int num);
    Q_INVOKABLE QScriptValue pokeNum(const QString &name);
    Q_INVOKABLE QScriptValue move(int num);
    Q_INVOKABLE QScriptValue moveNum(const QString &name);
    Q_INVOKABLE int moveType(int moveNum, int gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue item(int num);
    Q_INVOKABLE QScriptValue itemNum(const QString &item);
    Q_INVOKABLE QScriptValue nature(int num);
    Q_INVOKABLE QScriptValue natureNum(const QString &nature);
    Q_INVOKABLE QScriptValue ability(int num);
    Q_INVOKABLE QScriptValue abilityNum(const QString &nature);
    Q_INVOKABLE QScriptValue genderNum(QString genderName);
    Q_INVOKABLE QString gender(int genderNum);

    Q_INVOKABLE QScriptValue teamPokeLevel(int id, int team, int slot);
    Q_INVOKABLE QScriptValue teamPokeStat(int id, int team, int slot, int stat);
    Q_INVOKABLE QScriptValue teamPokeHp(int id, int team, int slot); //Stat would return total hp
    Q_INVOKABLE QScriptValue teamPokeStatus(int id, int team, int slot);
    Q_INVOKABLE QScriptValue teamPokePP(int id, int team, int slot, int moveslot);
    Q_INVOKABLE QScriptValue teamPoke(int id, int team, int index);
    Q_INVOKABLE QScriptValue teamPokeName(int id, int team, int pokemonnum);
    Q_INVOKABLE bool hasTeamPoke(int id, int team, int pokemonnum);
    Q_INVOKABLE QScriptValue indexOfTeamPoke(int id, int team, int pokenum);
    Q_INVOKABLE bool hasDreamWorldAbility(int id, int team, int slot, int gen = 5);
    Q_INVOKABLE bool compatibleAsDreamWorldEvent(int id, int team, int slot);

    Q_INVOKABLE QScriptValue teamPokeMove(int id, int team, int pokeindex, int moveindex);
    Q_INVOKABLE bool hasTeamPokeMove(int id, int team, int pokeindex, int movenum);
    Q_INVOKABLE QScriptValue indexOfTeamPokeMove(int id, int team, int pokeindex, int movenum);
    Q_INVOKABLE bool hasTeamMove(int id, int team, int movenum);

    Q_INVOKABLE QScriptValue teamPokeItem(int id, int team, int pokeindex);
    Q_INVOKABLE bool hasTeamItem(int id, int team, int itemNum);

    Q_INVOKABLE QScriptValue teamPokeHappiness(int id, int team, int slot);
    Q_INVOKABLE QScriptValue teamPokeNature(int id, int team, int slot);
    Q_INVOKABLE QScriptValue teamPokeEV(int id, int team, int slot, int stat);
    Q_INVOKABLE QScriptValue teamPokeDV(int id, int team, int slot, int stat);
    Q_INVOKABLE void changeTeamPokeDV(int id, int team, int slot, int stat, int newValue);
    Q_INVOKABLE void changeTeamPokeEV(int id, int team, int slot, int stat, int newValue);

    Q_INVOKABLE int numPlayers();
    Q_INVOKABLE int playersInMemory();
    Q_INVOKABLE bool exists(int id);
    Q_INVOKABLE bool loggedIn(int id);

    Q_INVOKABLE int rand(int min, int max);
    Q_INVOKABLE long time();
    Q_INVOKABLE QScriptValue getTierList();

    Q_INVOKABLE QScriptValue type(int id);
    Q_INVOKABLE QScriptValue typeNum(const QString &typeName);

    Q_INVOKABLE int hiddenPowerType(int gen, quint8 hpdv, quint8 attdv, quint8 defdv, quint8 spddv, quint8 sattdv, quint8 sdefdv);

    Q_INVOKABLE QScriptValue getScript();

    Q_INVOKABLE int pokeType1(int id, int gen = GenInfo::GenMax());
    Q_INVOKABLE int pokeType2(int id, int gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue baseStats(int poke, int stat, int gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue pokeBaseStats(int id, int gen = GenInfo::GenMax());

    /* Returns an array of "male", "female", "neutral" with percentages associated */
    Q_INVOKABLE QScriptValue pokeGenders(int poke);
   
    Q_INVOKABLE QScriptValue banList();
    Q_INVOKABLE void ban(QString name);
    Q_INVOKABLE void tempBan(QString name, int time);
    Q_INVOKABLE void unban(QString name);
    Q_INVOKABLE bool banned(const QString &ip);

#if 0
    Q_INVOKABLE void prepareWeather(int battleId, int weatherId);
    Q_INVOKABLE QScriptValue weatherNum(const QString &weatherName);
    Q_INVOKABLE QScriptValue weather(int weatherId);

    Q_INVOKABLE void prepareItems(int battleId, int playerSlot, QScriptValue items);

    /* Only do that in beforeBattleEnded. Will set your team to what it was at the end of the battle */
    Q_INVOKABLE void setTeamToBattleTeam(int pid, int teamSlot, int battleId);
#endif

    Q_INVOKABLE void swapPokemons(int pid, int teamSlot, int slot1, int slot2);

    Q_INVOKABLE int teamPokeAbility(int id, int team, int slot);
    Q_INVOKABLE void changePokeAbility(int id, int team, int slot, int ability);
    Q_INVOKABLE QScriptValue pokeAbility(int poke, int slot, int gen = GenInfo::GenMax());
    Q_INVOKABLE void changePokeHappiness(int id, int team, int slot, int value);
    Q_INVOKABLE void changePokeShine(int id, int team, int slot, bool value);
    Q_INVOKABLE QScriptValue teamPokeShine(int id, int team, int slot);
    Q_INVOKABLE void changePokeNature(int id, int team, int pokeslot, int nature);
    Q_INVOKABLE QScriptValue teamPokeGender(int id, int team, int slot);

    Q_INVOKABLE QScriptValue teamPokeNick(int id, int team, int pokeslot);

    /* Changes mod. Also reloads db even if same mod */
    Q_INVOKABLE void changeMod(const QString &val);

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);

    //Q_INVOKABLE void inflictStatus(int battleId, bool toFirstPlayer, int slot, int status);

    Q_INVOKABLE void forceBattle(int player1, int player2, int team1, int team2, int clauses, int mode, bool is_rated = false);
    Q_INVOKABLE int getClauses(const QString &tier);
    Q_INVOKABLE QString serverVersion();
    Q_INVOKABLE QString protocolVersion(int id);
    Q_INVOKABLE bool isServerPrivate();

    /* Internal use only */
    Q_INVOKABLE void sendNetworkCommand(int id, int command);
    
    Q_INVOKABLE QString sha1(const QString &text);
    Q_INVOKABLE QString md4(const QString &text);
    Q_INVOKABLE QString md5(const QString &text);

    Q_INVOKABLE bool validColor(const QString &color);
    Q_INVOKABLE QString hexColor(const QString &colorname);

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

    Q_INVOKABLE QScriptValue filesForDirectory (const QString &dir);
    Q_INVOKABLE QScriptValue dirsForDirectory (const QString &dir);

    // Direct file access.


    Q_INVOKABLE QScriptValue zip(const QString &path, const QString &directory);
    Q_INVOKABLE QScriptValue extractZip(const QString &zipName, const QString &targetDir);
    Q_INVOKABLE QScriptValue extractZip(const QString &zipName);

    //Q_INVOKABLE void writeCompressed(const QString &fileName, const QString &content, int ziplvl);
    //Q_INVOKABLE QScriptValue readCompressed(const QString &path);
    // Implement in the future


    /* GET call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QScriptValue &callback);
    /* POST call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QScriptValue &callback, const QScriptValue &params_array);

    /* synchronous GET call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring);
    /* synchronous POST call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring, const QScriptValue &params_array);

    // Server plugin management from scripts
    Q_INVOKABLE QScriptValue getServerPlugins();
    Q_INVOKABLE bool loadServerPlugin(const QString &path);
    Q_INVOKABLE bool unloadServerPlugin(const QString &plugin);
    Q_INVOKABLE void loadBattlePlugin(const QString &path);
    Q_INVOKABLE void unloadBattlePlugin(const QString &plugin);
#endif // PO_SCRIPT_SAFE_ONLY

#if !defined(PO_SCRIPT_NO_SYSTEM) && !defined(PO_SCRIPT_SAFE_ONLY)
    /* Calls the underlying OS for a command */
    Q_INVOKABLE int system(const QString &command);

    /* Better version of system, also captures the output */
    Q_INVOKABLE QScriptValue get_output(const QString &command, const QScriptValue &callback, const QScriptValue &errback);
    Q_INVOKABLE QScriptValue list_processes();
    Q_INVOKABLE QScriptValue kill_processes();
    Q_INVOKABLE QScriptValue write_process(double pid, const QString &data);
    inline quint64 getProcessID(const QProcess* proc)
    {
        #ifdef Q_OS_WIN
            struct _PROCESS_INFORMATION* procinfo = proc->pid();
            return procinfo->dwProcessId;
        #else // Linux
            return proc->pid();
        #endif // Q_WS_WIN
    }
    Server* getServer();
    QScriptEngine* getEngine();
    void printLine(const QString &s);
signals:
    void stdoutReceived(quint64 pid, const QString &procStdout);
    void stderrReceived(quint64 pid, const QString &procStderr);
private slots:
    void process_finished(int exitcode, QProcess::ExitStatus exitStatus);
    void process_error(QProcess::ProcessError error);
    void read_standard_output();
    void read_standard_error();
private:
    QScriptValue parse;
    QScriptValue stringify;
    Server *myserver;
    QScriptEngine myengine;
    QScriptValue myscript;
    struct ProcessData {
        QScriptValue callback;
        QScriptValue errback;
        QByteArray out;
        QByteArray err;
        QString command;
        quint64 pid;
    };
    QHash<QProcess*, ProcessData> processes;
public:
    Q_INVOKABLE bool addPlugin(const QString &path);
    Q_INVOKABLE void removePlugin(int index);
    Q_INVOKABLE QStringList listPlugins();
#endif // PO_SCRIPT_NO_SYSTEM

signals:
    void clearTheChat();
public slots:
    void changeScript(const QString &script, const bool triggerStartUp = false);
    void battleConnectionLost();

private slots:
    void timer();
    void timer_step();
#ifndef PO_SCRIPT_SAFE_ONLY
    void webCall_replyFinished(QNetworkReply* reply);
    void synchronousWebCall_replyFinished(QNetworkReply* reply);
#endif
    void hostInfo_Ready(const QHostInfo &myInfo);
    
private:
    bool strict;
    bool wfatal;
    QTimer * step_timer;
    QVector<bool> stopevents;
    SessionDataFactory *mySessionDataFactory;

    QNetworkAccessManager manager;
    QHash<QTimer*,QScriptValue> timerEvents;
    QHash<QNetworkReply*,QScriptValue> webCallEvents;
    QHash<int,QScriptValue> myHostLookups;

    void startStopEvent() {
        stopevents.push_back(false);
    }

    bool endStopEvent() {
        bool res = stopevents.back();
        stopevents.pop_back();
        return res;
    }

    QEventLoop sync_loop;
    QString sync_data;

    void evaluate(const QScriptValue &expr);

    bool testPlayer(const QString &function, int id);
    bool testTeamCount(const QString &function, int id, int team);
    bool testChannel(const QString &function, int id);
    bool testPlayerInChannel(const QString &function, int id, int chan);
    bool testRange(const QString &function, int val, int min, int max);
    void warn(const QString &function, const QString &message, bool errinstrict);

    struct Profile {
        int calls;
        int totalDuration;
        Profile() {
            calls = 0;
            totalDuration = 0;
        }
    };

    QHash<QString, Profile> profiles;
    QElapsedTimer performanceTimer;
    bool resetPerfs;
    quint64 startProfiling();
    void endProfiling(quint64 startTime, const QString &name);
    template <typename ...Params>
    void makeEvent(const QString &event, Params&&... params);
    template <typename ...Params>
    bool makeSEvent(const QString &event, Params&&... params);
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

template<typename ...Params>
void ScriptEngine::makeEvent(const QString &event, Params &&... params)
{
    if (!this || !myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return;

    QScriptValueList l;
    auto startTime = startProfiling();
    evaluate(myscript.property(event).call(myscript, pack(l, params...)));
    endProfiling(startTime, "script." + event);

    if (resetPerfs) {
        resetPerfs = false;

        profiles.clear();
        performanceTimer.restart();
    }
}

template<typename ...Params>
bool ScriptEngine::makeSEvent(const QString &event, Params &&... params)
{
    /* !this happens on first run of the server for example when channels are created outside events,
     * when script is first evaluated */
    if (!this || !myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    QScriptValueList l;
    auto startTime = startProfiling();
    evaluate(myscript.property(event).call(myscript, pack(l, params...)));
    endProfiling(startTime, "script." + event);

    if (resetPerfs) {
        resetPerfs = false;

        profiles.clear();
        performanceTimer.restart();
    }

    return !endStopEvent();
}

#endif // SCRIPTENGINE_H
