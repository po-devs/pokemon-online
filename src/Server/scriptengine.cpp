#include "server.h"
#include "player.h"
#include "security.h"
#include "antidos.h"
#include "waitingobject.h"
#include "tiermachine.h"
#include "tier.h"
#include "scriptengine.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/movesetchecker.h"
#include "battle.h"
#include <QRegExp>
#include "analyze.h"
#include "../Shared/config.h"

ScriptEngine::ScriptEngine(Server *s) {
    setParent(s);
    myserver = s;
    mySessionDataFactory = new SessionDataFactory(&myengine);

    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("sys", sys);
    QScriptValue printfun = myengine.newFunction(nativePrint);
    printfun.setData(sys);
    myengine.globalObject().setProperty("print", printfun);
    myengine.globalObject().setProperty(
        "SESSION",
        myengine.newQObject(mySessionDataFactory),
        QScriptValue::ReadOnly | QScriptValue::Undeletable
    );

#ifndef PO_SCRIPT_SAFE_ONLY
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
#endif

    QFile f("scripts.js");
    f.open(QIODevice::ReadOnly);

    changeScript(QString::fromUtf8(f.readAll()));

    QTimer *step_timer = new QTimer(this);
    step_timer->setSingleShot(false);
    step_timer->start(1000);
    connect(step_timer, SIGNAL(timeout()), SLOT(timer_step()));
}

ScriptEngine::~ScriptEngine()
{
    delete mySessionDataFactory;
}

void ScriptEngine::changeScript(const QString &script, const bool triggerStartUp)
{
    mySessionDataFactory->disableAll();
    myscript = myengine.evaluate(script);
    myengine.globalObject().setProperty("script", myscript);

    if (myscript.isError()) {
        printLine("Fatal Script Error line " + QString::number(myengine.uncaughtExceptionLineNumber()) + ": " + myscript.toString());
    } else {
        printLine("Script Check: OK");
        if(triggerStartUp) {
            serverStartUp();
        }
    }

    mySessionDataFactory->handleInitialState();
    if (mySessionDataFactory->isRefillNeeded()) {
        // Refill player session info if session data is no longer valid.
        QList<int> keys = myserver->myplayers.keys();
        for (int i = 0; i < keys.size(); i++) {
            mySessionDataFactory->handleUserLogIn(keys[i]);
        }
        // Refill channels as well.
        keys = myserver->channels.keys();
        for (int i = 0; i < keys.size(); i++) {
            int current_channel = keys[i];
            // Default channel is already there.
            if (current_channel != 0) {
                mySessionDataFactory->handleChannelCreate(current_channel);
            }
        }

        mySessionDataFactory->refillDone();
    }
    // Error check?
}

QScriptValue ScriptEngine::import(const QString &fileName) {
    QString url = "scripts/"+fileName;
    QFile in(url);

    if (!in.open(QIODevice::ReadOnly)) {
        warn("sys.import", "The file scripts/" + fileName + " is not readable.");
        return QScriptValue();
    }

    QScriptValue import = myengine.evaluate(QString::fromUtf8(in.readAll()));
    evaluate(import);
    return import;
}

QScriptValue ScriptEngine::nativePrint(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    ScriptEngine *obj = qobject_cast<ScriptEngine*>(calleeData.toQObject());
    obj->printLine(result);

    return engine->undefinedValue();
}

void ScriptEngine::print(QScriptContext *context, QScriptEngine *)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }

    printLine(result);
}

bool ScriptEngine::testChannel(const QString &function, int id)
{
    if (!myserver->channelExist(id)) {
        if (function.length() > 0)
            warn(function, QString("No channel numbered %1 existing").arg(id));
        return false;
    }

    return true;
}

bool ScriptEngine::testPlayer(const QString &function, int id)
{
    if (!loggedIn(id)) {
        if (function.length() > 0)
            warn(function, QString("No player numbered %1 existing").arg(id));
        return false;
    }

    return true;
}

bool ScriptEngine::testPlayerInChannel(const QString &function, int id, int chan)
{
    if (!myserver->player(id)->getChannels().contains(chan)) {
        if (function.length() > 0)
            warn(function, QString("Player number %1 is not in channel number %2").arg(id).arg(chan));
        return false;
    }

    return true;
}

bool ScriptEngine::testRange(const QString &function, int val, int min, int max)
{
    if (val < min || val > max) {
        if (function.length() > 0)
            warn(function, QString("%1 is out of the range [%2, %3]").arg(val).arg(min).arg(max));
        return false;
    }

    return true;
}

void ScriptEngine::warn(const QString &function, const QString &message)
{
    printLine(QString("Script Warning in %1: %2").arg(function, message));
}

bool ScriptEngine::beforeChatMessage(int src, const QString &message, int channel)
{
    return makeSEvent("beforeChatMessage", src, message, channel);
}

void ScriptEngine::afterChatMessage(int src, const QString &message, int channel)
{
    makeEvent("afterChatMessage", src, message, channel);
}

bool ScriptEngine::beforeSpectateBattle(int src, int p1, int p2)
{
    return makeSEvent("beforeSpectateBattle", src, p1, p2);
}

void ScriptEngine::afterSpectateBattle(int src, int p1, int p2)
{
    makeEvent("beforeSpectateBattle", src, p1, p2);
}


bool ScriptEngine::beforeNewMessage(const QString &message)
{
    return makeSEvent("beforeNewMessage", message);
}

void ScriptEngine::afterNewMessage(const QString &message)
{
    makeEvent("afterNewMessage", message);
}

void ScriptEngine::serverStartUp()
{
    evaluate(myscript.property("serverStartUp").call(myscript, QScriptValueList()));
}

void ScriptEngine::stepEvent()
{
    evaluate(myscript.property("step").call(myscript, QScriptValueList()));
}

void ScriptEngine::serverShutDown()
{
    evaluate(myscript.property("serverShutDown").call(myscript, QScriptValueList()));
}

bool ScriptEngine::beforeLogIn(int src)
{
    return makeSEvent("beforeLogIn", src);
}

void ScriptEngine::afterLogIn(int src)
{
    mySessionDataFactory->handleUserLogIn(src);
    makeEvent("afterLogIn", src);
}

bool ScriptEngine::beforeChannelCreated(int channelid, const QString &channelname, int playerid)
{
    return makeSEvent("beforeChannelCreated", channelid, channelname, playerid);
}

void ScriptEngine::afterChannelCreated(int channelid, const QString &channelname, int playerid)
{
    mySessionDataFactory->handleChannelCreate(channelid);
    makeEvent("afterChannelCreated", channelid, channelname, playerid);
}

bool ScriptEngine::beforeChannelDestroyed(int channelid)
{
    return makeSEvent("beforeChannelDestroyed", channelid);
}

void ScriptEngine::afterChannelDestroyed(int channelid)
{
    makeEvent("afterChannelDestroyed", channelid);
    mySessionDataFactory->handleChannelDestroy(channelid);
}

bool ScriptEngine::beforeChannelJoin(int playerid, int channelid)
{
    return makeSEvent("beforeChannelJoin", playerid, channelid);
}

void ScriptEngine::afterChannelJoin(int playerid, int channelid)
{
    makeEvent("afterChannelJoin", playerid, channelid);
}

void ScriptEngine::beforeChannelLeave(int playerid, int channelid)
{
    makeEvent("beforeChannelLeave", playerid, channelid);
}

void ScriptEngine::afterChannelLeave(int playerid, int channelid)
{
    makeEvent("afterChannelLeave", playerid, channelid);
}

void ScriptEngine::beforeChangeTeam(int src)
{
    makeEvent("beforeChangeTeam", src);
}

void ScriptEngine::afterChangeTeam(int src)
{
    makeEvent("afterChangeTeam", src);
}

bool ScriptEngine::beforeChangeTier(int src, const QString &oldTier, const QString &newTier)
{
    return makeSEvent("beforeChangeTier", src, oldTier, newTier);
}

void ScriptEngine::afterChangeTier(int src, const QString &oldTier, const QString &newTier)
{
    makeEvent("afterChangeTier", src, oldTier, newTier);
}

bool ScriptEngine::beforeChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("beforeChallengeIssued", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforeChallengeIssued").call(myscript, QScriptValueList() << src << dest << c.clauses << c.rated << c.mode));

    return !endStopEvent();
}

void ScriptEngine::afterChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("afterChallengeIssued", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterChallengeIssued").call(myscript, QScriptValueList() << src << dest << c.clauses << c.rated << c.mode));
}

bool ScriptEngine::beforeBattleMatchup(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("beforeBattleMatchup", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforeBattleMatchup").call(myscript, QScriptValueList() << src << dest << c.clauses << c.rated << c.mode));

    return !endStopEvent();
}

void ScriptEngine::afterBattleMatchup(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("afterBattleMatchup", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterBattleMatchup").call(myscript, QScriptValueList() << src << dest << c.clauses << c.rated << c.mode));
}


void ScriptEngine::beforeBattleStarted(int src, int dest, const ChallengeInfo &c, int id)
{
    if (!myscript.property("beforeBattleStarted", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("beforeBattleStarted").call(myscript, QScriptValueList() << src << dest << c.clauses << c.rated << c.mode << id));
}

void ScriptEngine::afterBattleStarted(int src, int dest, const ChallengeInfo &c, int id)
{
    if (!myscript.property("afterBattleStarted", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterBattleStarted").call(myscript, QScriptValueList() << src << dest << c.clauses << c.rated << c.mode << id));
}

QString battleDesc[3] = {
    "forfeit",
    "win",
    "tie"
};

void ScriptEngine::beforeBattleEnded(int src, int dest, int desc, int battleid)
{
    if (!myscript.property("beforeBattleEnded", QScriptValue::ResolveLocal).isValid())
        return;
    if (desc < 0 || desc > 2)
        return;
    evaluate(myscript.property("beforeBattleEnded").call(myscript, QScriptValueList() << src << dest << battleDesc[desc] << battleid));
}

void ScriptEngine::afterBattleEnded(int src, int dest, int desc, int battleid)
{
    if (!myscript.property("afterBattleEnded", QScriptValue::ResolveLocal).isValid())
        return;
    if (desc < 0 || desc > 2)
        return;
    evaluate(myscript.property("afterBattleEnded").call(myscript, QScriptValueList() << src << dest << battleDesc[desc] << battleid));
}

bool ScriptEngine::beforeFindBattle(int src) {
    return makeSEvent("beforeFindBattle", src);
}

void ScriptEngine::afterFindBattle(int src) {
    makeEvent("afterFindBattle", src);
}

void ScriptEngine::beforeLogOut(int src)
{
    makeEvent("beforeLogOut", src);
}

void ScriptEngine::afterLogOut(int src)
{
    makeEvent("afterLogOut", src);

    mySessionDataFactory->handleUserLogOut(src);
}

bool ScriptEngine::beforePlayerKick(int src, int dest)
{
    return makeSEvent("beforePlayerKick", src, dest);
}

void ScriptEngine::afterPlayerKick(int src, int dest)
{
    makeEvent("afterPlayerKick", src, dest);
}

bool ScriptEngine::beforePlayerBan(int src, int dest)
{
    return makeSEvent("beforePlayerBan", src, dest);
}

void ScriptEngine::afterPlayerBan(int src, int dest)
{
    makeEvent("afterPlayerBan", src, dest);
}

bool ScriptEngine::beforePlayerAway(int src, bool away)
{
    return makeSEvent("beforePlayerAway", src, away);
}

void ScriptEngine::afterPlayerAway(int src, bool away)
{
    makeEvent("afterPlayerAway", src, away);
}

void ScriptEngine::evaluate(const QScriptValue &expr)
{
    if (expr.isError()) {
        printLine(QString("Script Error line %1: %2").arg(myengine.uncaughtExceptionLineNumber()).arg(expr.toString()));
    }
}

void ScriptEngine::sendAll(const QString &mess)
{
    myserver->sendAll(mess);
}

void ScriptEngine::sendAll(const QString &mess, int channel)
{
    if (testChannel("sendAll(mess, channel)", channel)) {
        myserver->sendChannelMessage(channel, mess);
    }
}

void ScriptEngine::sendMessage(int id, const QString &mess)
{
    if (testPlayer("sendMessage(id, mess)", id)) {
        myserver->sendMessage(id, mess);
    }
}

void ScriptEngine::sendMessage(int id, const QString &mess, int channel)
{
    if (testChannel("sendMessage(id, mess, channel)", channel) && testPlayer("sendMessage(id, mess, channel)", id) &&
        testPlayerInChannel("sendMessage(id, mess, channel)", id, channel))
    {
        myserver->sendChannelMessage(id, channel, mess);
    }
}

void ScriptEngine::sendHtmlAll(const QString &mess)
{
    myserver->sendAll(mess, false, true);
}

void ScriptEngine::sendHtmlAll(const QString &mess, int channel)
{
    if (testChannel("sendAll(mess, channel)", channel)) {
        myserver->sendChannelMessage(channel, mess, false, true);
    }
}

void ScriptEngine::sendHtmlMessage(int id, const QString &mess)
{
    if (testPlayer("sendMessage(id, mess)", id)) {
        myserver->sendMessage(id, mess, true);
    }
}

void ScriptEngine::sendHtmlMessage(int id, const QString &mess, int channel)
{
    if (testChannel("sendMessage(id, mess, channel)", channel) && testPlayer("sendMessage(id, mess, channel)", id) &&
        testPlayerInChannel("sendMessage(id, mess, channel)", id, channel))
    {
        myserver->sendChannelMessage(id, channel, mess, true);
    }
}

void ScriptEngine::kick(int id)
{
    if (testPlayer("kick(id)", id)) {
        myserver->silentKick(id);
    }
}

void ScriptEngine::kick(int id, int chanid)
{
    if (testPlayer("kick(id, channel)", id) && testChannel("kick(id, channel)", chanid)
        && testPlayerInChannel("kick(id, channel)", id, chanid))
    {
        myserver->leaveRequest(id, chanid);
    }
}

void ScriptEngine::updatePlayer(int playerid)
{
    /* Updates all the info of the player to the other players
       (mainly if you changed their team and want it to show in the challenge window) */
    if (testPlayer("updatePlayer(playerid)", playerid)) {
        myserver->sendPlayer(playerid);
    }
}

void ScriptEngine::putInChannel(int id, int chanid)
{
    if (!testPlayer("putInChannel(id, chanid)", id) || !testChannel("putInChannel(id, chanid)", chanid)) {
        return;
    }
    if (myserver->player(id)->getChannels().contains(chanid)){
        printLine(QString("Script Warning in sys.putInChannel(id, chan): player %1 is already in channel %2").arg(id).arg(chanid));
    } else {
        myserver->joinChannel(id, chanid);
    }
}

QScriptValue ScriptEngine::createChannel(const QString &channame)
{
    if (myserver->channelExist(channame)) {
        return myengine.undefinedValue();
    } else {
        return myserver->addChannel(channame);
    }
}

bool ScriptEngine::existChannel(const QString &channame)
{
    return myserver->channelExist(channame);
}

void ScriptEngine::clearPass(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        printLine("Script Warning in sys.clearPass(name): no such player name as " + name);
    }
    SecurityManager::clearPass(name);
}

void ScriptEngine::changeAuth(int id, int auth)
{
    if (testPlayer("changeAuth(id, auth)", id)) {
        if (myserver->isSafeScripts() && ((myserver->auth(id) > 2) || (auth > 2))) {
            warn("changeAuth", "Safe scripts option is on. Unable to change auth to/from 3 and above.");
        } else {
            myserver->changeAuth(myserver->name(id), auth);
        }
    }
}

void ScriptEngine::changeDbAuth(const QString &name, int auth)
{
    if (myserver->isSafeScripts()) {
        if (!SecurityManager::exist(name)) return;
        if ((SecurityManager::member(name).auth > 2) || (auth > 2)) {
            warn("changeDbAuth", "Safe scripts option is on. Unable to change auth to/from 3 and above.");
            return;
        }
    }
    SecurityManager::setAuth(name, auth);
}

void ScriptEngine::changeAway(int id, bool away)
{
    if (testPlayer("changeAway(id, away)", id)) {
        myserver->player(id)->executeAwayChange(away);
    }
}

void ScriptEngine::changeRating(const QString& name, const QString& tier, int newRating)
{
    if (!TierMachine::obj()->exists(tier))
        printLine("Script Warning in sys.changeRating(name, tier, rating): no such tier as " + tier);
    else
        TierMachine::obj()->changeRating(name, tier, newRating);
}

void ScriptEngine::changeTier(int id, const QString &tier)
{
    if (!TierMachine::obj()->exists(tier))
        printLine("Script Warning in sys.changeTier(id, tier): no such tier as " + tier);
    else if (testPlayer("changeTier(id, tier)", id)) {
        myserver->player(id)->executeTierChange(tier);
    }
}

void ScriptEngine::reloadTiers()
{
    TierMachine::obj()->load();
}

void ScriptEngine::changePokeItem(int id, int slot, int item)
{
    if (!testPlayer("changePokeItem(id, slot, item)", id) || !testRange("changePokeItem(id, slot, item)", slot, 0, 5))
        return;
    if (!ItemInfo::Exists(item))
        return;
    myserver->player(id)->team().poke(slot).item() = item;
}

void ScriptEngine::changePokeNum(int id, int slot, int num)
{
    if (!testPlayer("changePokeNum(id, slot, item)", id) || !testRange("changePokeNum(id, slot, num)", slot, 0, 5))
        return;
    if (!PokemonInfo::Exists(num, myserver->player(id)->gen()))
        return;
    myserver->player(id)->team().poke(slot).num() = num;
}

void ScriptEngine::changePokeLevel(int id, int slot, int level)
{
    if (!testPlayer("changePokeLevel(id, slot, level)", id) || !testRange("changePokeLevel(id, slot, level)", slot, 0, 5) || !testRange("changePokeLevel(id, slot, level)", level, 1, 100))
        return;
    Player *p = myserver->player(id);
    p->team().poke(slot).level() = level;
    p->team().poke(slot).updateStats(p->gen());
}

void ScriptEngine::changePokeMove(int id, int pslot, int mslot, int move)
{
    if (!testPlayer("changePokeLevel(id, pokeslot, moveslot, move)", id) || !testRange("changePokeLevel(id, pokeslot, moveslot, move)", pslot, 0, 5) || !testRange("changePokeLevel(id, pokeslot, moveslot, move)", mslot, 0, 3))
        return;
    if (!MoveInfo::Exists(move, GEN_MAX))
        return;
    Player *p = myserver->player(id);
    p->team().poke(pslot).move(mslot).num() = move;
    p->team().poke(pslot).move(mslot).load(p->gen());
}

void ScriptEngine::changePokeGender(int id, int pokeslot, int gender)
{
    if (!testPlayer("changePokeGender(id, pokeslot, gender)", id) || !testRange("changePokeGender(id, pokeslot, gender)", pokeslot, 0, 5) || !testRange("changePokeGender(id, pokeslot, gender)", gender, 0, 2))
        return;
    Player *p = myserver->player(id);
    p->team().poke(pokeslot).gender() = gender;
}

void ScriptEngine::changePokeName(int id, int pokeslot, const QString &name)
{
    if (!testPlayer("changePokeName(id, pokeslot, name)", id)|| !testRange("changePokeName(id, pokeslot, name)", pokeslot, 0, 5))
        return;
    Player *p = myserver->player(id);
    p->team().poke(pokeslot).nick() = name;
}

bool ScriptEngine::hasLegalTeamForTier(int id, const QString &tier)
{
    if (!TierMachine::obj()->exists(tier))
        return false;
    return TierMachine::obj()->isValid(myserver->player(id)->team(),tier);
}

int ScriptEngine::maxAuth(const QString &ip)
{
    return SecurityManager::maxAuth(ip);
}

QScriptValue ScriptEngine::aliases(const QString &ip)
{
    QStringList mip = SecurityManager::membersForIp(ip);

    QScriptValue ret = myengine.newArray(mip.count());

    for(int i = 0; i < mip.size(); i++) {
        ret.setProperty(i, mip[i]);
    }

    return ret;
}

QScriptValue ScriptEngine::memoryDump()
{
    QString ret;

    ret += QString("Members\n\tCached in memory> %1\n\tCached as non-existing> %2\n").arg(SecurityManager::holder.cachedMembersCount()).arg(SecurityManager::holder.cachedNonExistingCount());
    ret += QString("Waiting Objects\n\tFree Objects> %1\n\tTotal Objects> %2\n").arg(WaitingObjects::freeObjects.count()).arg(WaitingObjects::objectCount);
    ret += QString("Battles\n\tActive> %1\n\tRated Battles History> %2\n").arg(myserver->mybattles.count()).arg(myserver->lastRatedIps.count());
    ret += QString("Antidos\n\tConnections Per IP> %1\n\tLogins per IP> %2\n\tTransfers Per Id> %3\n\tSize of Transfers> %4\n\tKicks per IP> %5\n").arg(AntiDos::obj()->connectionsPerIp.count()).arg(
            AntiDos::obj()->loginsPerIp.count()).arg(AntiDos::obj()->transfersPerId.count()).arg(AntiDos::obj()->sizeOfTransfers.count())
           .arg(AntiDos::obj()->kicksPerIp.count());
    ret += QString("-------------------------\n-------------------------\n");

    foreach (QString tier, TierMachine::obj()->tierList().split('\n')) {
        const Tier &t = TierMachine::obj()->tier(tier);
        ret += QString("Tier %1\n\tCached in memory> %2\n\tCached as non-existing> %3\n").arg(tier).arg(t.holder.cachedMembersCount()).arg(t.holder.cachedNonExistingCount());
    }

    return ret;
}

void ScriptEngine::exportMemberDatabase()
{
    SecurityManager::exportDatabase();
}

void ScriptEngine::exportTierDatabase()
{
    TierMachine::obj()->exportDatabase();
}

void ScriptEngine::clearChat()
{
    emit clearTheChat();
}

bool ScriptEngine::dbRegistered(const QString &name)
{
    return SecurityManager::member(name).isProtected();
}

void ScriptEngine::callLater(const QString &expr, int delay)
{
    if (delay <= 0) {
        return;
    }
	//qDebug() << "Call Later in " << delay << expr;
    QTimer *t = new QTimer();

    timerEvents[t] = expr;
    t->setSingleShot(true);
    t->start(delay*1000);
    connect(t, SIGNAL(timeout()), SLOT(timer()), Qt::DirectConnection);
}

void ScriptEngine::callQuickly(const QString &expr, int delay)
{
    if (delay <= 0) {
        return;
    }

    QTimer *t = new QTimer(this);

    timerEvents[t] = expr;
    t->setSingleShot(true);
    t->start(delay);
    connect(t, SIGNAL(timeout()), SLOT(timer()));
}

void ScriptEngine::timer()
{
	//qDebug() << "timer()";
    QTimer *t = (QTimer*) sender();
	//qDebug() << timerEvents[t];
    eval(timerEvents[t]);

    timerEvents.remove(t);
    t->deleteLater();
}

void ScriptEngine::timer_step()
{
    this->stepEvent();
}

void ScriptEngine::delayedCall(const QScriptValue &func, int delay)
{
    if (delay <= 0) return;
    if (func.isFunction()) {
        QTimer *t = new QTimer(this);
        timerEventsFunc[t] = func;
        t->setSingleShot(true);
        t->start(delay*1000);
        connect(t, SIGNAL(timeout()), SLOT(timerFunc()));
    }
}

void ScriptEngine::timerFunc()
{
    QTimer *t = (QTimer*) sender();
    timerEventsFunc[t].call();
    timerEventsFunc.remove(t);
    t->deleteLater();
}


QScriptValue ScriptEngine::eval(const QString &script)
{
    return myengine.evaluate(script);
}

QScriptValue ScriptEngine::auth(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->auth(id);
    }
}

QScriptValue ScriptEngine::dbAuth(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        return SecurityManager::member(name).auth;
    }
}

QScriptValue ScriptEngine::dbAuths()
{
    QStringList sl = SecurityManager::authList();

    QScriptValue ret = myengine.newArray(sl.count());

    for (int i = 0; i < sl.size(); i++) {
        ret.setProperty(i, sl[i]);
    }

    return ret;
}

QScriptValue ScriptEngine::dbAll()
{
    QStringList sl = SecurityManager::userList();

    QScriptValue ret = myengine.newArray(sl.count());

    for (int i = 0; i < sl.size(); i++) {
        ret.setProperty(i, sl[i]);
    }

    return ret;
}

QScriptValue ScriptEngine::dbIp(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        return QString(SecurityManager::member(name).ip);
    }
}

QScriptValue ScriptEngine::dbDelete(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        SecurityManager::deleteUser(name);
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::dbLastOn(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        return QString(SecurityManager::member(name).date);
    }
}


QScriptValue ScriptEngine::battling(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->battling();
    }
}

QScriptValue ScriptEngine::away(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->away();
    }
}

QScriptValue ScriptEngine::getColor(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->color().name();
    }
}

QScriptValue ScriptEngine::tier(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->tier();
    }
}

QScriptValue ScriptEngine::ranking(int id)
{
    Player *p = myserver->player(id);
    return ranking(p->name(), p->tier());
}

QScriptValue ScriptEngine::ratedBattles(int id)
{
    Player *p = myserver->player(id);
    return ratedBattles(p->name(), p->tier());
}

QScriptValue ScriptEngine::ranking(const QString &name, const QString &tier)
{
    if (!TierMachine::obj()->existsPlayer(tier, name)) {
        return myengine.undefinedValue();
    }
    return TierMachine::obj()->ranking(name, tier);
}

QScriptValue ScriptEngine::ratedBattles(const QString &name, const QString &tier)
{
    if (!TierMachine::obj()->existsPlayer(tier, name)) {
        return 0;
    }
    return TierMachine::obj()->tier(tier).ratedBattles(name);
}

QScriptValue ScriptEngine::totalPlayersByTier(const QString &tier)
{
    return TierMachine::obj()->count(tier);
}

QScriptValue ScriptEngine::ladderRating(int id, const QString &tier)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        if (tier.isEmpty()) {
            return myserver->player(id)->rating();
        } else {
            return TierMachine::obj()->rating(myserver->player(id)->name(), tier);
        }
    }
}

QScriptValue ScriptEngine::ladderEnabled(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->ladder();
    }
}

QScriptValue ScriptEngine::ip(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->ip();
    }
}

QScriptValue ScriptEngine::proxyIp(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->proxyIp();
    }
}

void ScriptEngine::hostName(const QString &ip, const QScriptValue &function)
{
    myHostLookups[QHostInfo::lookupHost(ip, this, SLOT(hostInfo_Ready(QHostInfo)))] = function;
}

void ScriptEngine::hostInfo_Ready(const QHostInfo &myInfo)
{
    QScriptValue myVal = myHostLookups.take(myInfo.lookupId());
    if(myVal.isString()) {
        QString info = myInfo.hostName();
        eval("var name = '"+info+"';"+myVal.toString());
    } else {
        if(myVal.isFunction()) {
            QScriptValueList arguments;
            arguments << QString(myInfo.hostName());
            myVal.call(QScriptValue(), arguments);
        }
    }
}

QScriptValue ScriptEngine::gen(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->gen();
    }
}

QScriptValue ScriptEngine::name(int id)
{
    if (!myserver->playerExist(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->name(id);
    }
}

QScriptValue ScriptEngine::id(const QString &name)
{
    if (!myserver->nameExist(name)) {
        return myengine.undefinedValue();
    } else {
        return myserver->id(name);
    }
}

QScriptValue ScriptEngine::pokemon(int num)
{
    return PokemonInfo::Name(num);
}

QScriptValue ScriptEngine::pokeNum(const QString &name)
{
    QString copy = name;
    bool up = true;
    for (int i = 0; i < copy.length(); i++) {
        if (up) {
            copy[i] = copy[i].toUpper();
            up = false;
        } else {
            if (copy[i] == '-' || copy[i] == ' ')
                up = true;
            copy[i] = copy[i].toLower();
        }
    }
    Pokemon::uniqueId num = PokemonInfo::Number(copy);
    if (num.toPokeRef() == Pokemon::NoPoke) {
        return myengine.undefinedValue();
    } else {
        return num.toPokeRef();
    }
}

QScriptValue ScriptEngine::move(int num)
{
    if (num < 0  || num >= MoveInfo::NumberOfMoves()) {
        return myengine.undefinedValue();
    } else {
        return MoveInfo::Name(num);
    }
}

QString convertToSerebiiName(const QString input)
{
    QString truename = input;
    bool blankbefore = true;
    for (int i = 0; i < truename.length(); i++) {
        if (truename[i].isSpace()) {
            blankbefore = true;
        } else {
            if (blankbefore) {
                truename[i] = truename[i].toUpper();
                blankbefore = false;
            } else {
                truename[i] = truename[i].toLower();
            }
        }
    }
    return truename;
}

QScriptValue ScriptEngine::moveNum(const QString &name)
{
    int num = MoveInfo::Number(convertToSerebiiName(name));
    return num == 0 ? myengine.undefinedValue() : num;
}

QScriptValue ScriptEngine::item(int num)
{
    if (ItemInfo::Exists(num)) {
        return ItemInfo::Name(num);
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::itemNum(const QString &name)
{
    int num = ItemInfo::Number(convertToSerebiiName(name));
    return num == 0 ? myengine.undefinedValue() : num;
}


QScriptValue ScriptEngine::nature(int num)
{
    if (num >= 0 && num < NatureInfo::NumberOfNatures()) {
        return NatureInfo::Name(num);
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::natureNum(const QString &name)
{
    return NatureInfo::Number(convertToSerebiiName(name));
}

QScriptValue ScriptEngine::ability(int num)
{
    if (num >= 0 && num < AbilityInfo::NumberOfAbilities()) {
        return AbilityInfo::Name(num);
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::abilityNum(const QString &ability)
{
    return AbilityInfo::Number(ability);
}

QScriptValue ScriptEngine::genderNum(QString genderName)
{
    if(genderName.toLower() == "genderless") {
        return 0;
    }
    if(genderName.toLower() == "male") {
        return 1;
    }
    if(genderName.toLower() == "female") {
        return 2;
    }
    return "";
}

QString ScriptEngine::gender(int genderNum)
{
    switch(genderNum) {
        case 0:
            return "genderless";
        case 1:
            return "male";
        case 2:
            return "female";
    }
    return "";
}

QScriptValue ScriptEngine::teamPoke(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).num().toPokeRef();
    }
}

QScriptValue ScriptEngine::teamPokeLevel(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).level();
    }
}


bool ScriptEngine::hasTeamPoke(int id, int pokemonnum)
{
    if (!testPlayer("hasTeamPoke(id, poke)",id)) {
        return false;
    }
    TeamBattle &t = myserver->player(id)->team();
    for (int i = 0; i < 6; i++) {
        if (t.poke(i).num() == pokemonnum) {
            return true;
        }
    }
    return false;
}

QScriptValue ScriptEngine::indexOfTeamPoke(int id, int pokenum)
{
    if (!loggedIn(id)) {
        printLine("Script Warning in sys.indexOfTeamPoke(id, pokenum): no such player logged in with id " + QString::number(id));
        return myengine.undefinedValue();
    }
    TeamBattle &t = myserver->player(id)->team();
    for (int i = 0; i < 6; i++) {
        if (t.poke(i).num() == pokenum) {
            return i;
        }
    }
    return myengine.undefinedValue();
}

bool ScriptEngine::hasDreamWorldAbility(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return false;
    } else {
        PokeBattle &p = myserver->player(id)->team().poke(index);

        AbilityGroup ag = PokemonInfo::Abilities(p.num(), 5);

        return p.ability() != ag.ab(0) && p.ability() != ag.ab(1);
    }
}

bool ScriptEngine::compatibleAsDreamWorldEvent(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return false;
    } else {
        PokeBattle &p = myserver->player(id)->team().poke(index);

        return MoveSetChecker::isValid(p.num(),5,p.move(0).num(),p.move(1).num(),p.move(2).num(),p.move(3).num(),p.ability(),p.gender(), p.level(), true);
    }
}

QScriptValue ScriptEngine::teamPokeMove(int id, int pokeindex, int moveindex)
{
    if (!loggedIn(id) || pokeindex < 0 || moveindex < 0 || pokeindex >= 6 || moveindex >= 4) {
        return myengine.undefinedValue();
    }
    return myserver->player(id)->team().poke(pokeindex).move(moveindex).num();
}

bool ScriptEngine::hasTeamPokeMove(int id, int pokeindex, int movenum)
{
    if (!loggedIn(id) || pokeindex < 0 || pokeindex >= 6) {
        return false;
    }
    PokeBattle &poke = myserver->player(id)->team().poke(pokeindex);

    for (int i = 0; i < 4; i++) {
        if (poke.move(i).num() == movenum) {
            return true;
        }
    }
    return false;
}

QScriptValue ScriptEngine::indexOfTeamPokeMove(int id, int pokeindex, int movenum)
{
    if (!loggedIn(id) || pokeindex < 0 || pokeindex >= 6) {
        return myengine.undefinedValue();
    }
    PokeBattle &poke = myserver->player(id)->team().poke(pokeindex);

    for (int i = 0; i < 4; i++) {
        if (poke.move(i).num() == movenum) {
            return i;
        }
    }
    return myengine.undefinedValue();
}

bool ScriptEngine::hasTeamMove(int id, int movenum)
{
    if (!loggedIn(id)) {
        printLine("Script Warning in sys.hasTeamMove(id, pokenum): no such player logged in with id " + QString::number(id));
        return false;
    }
    for (int i = 0; i < 6; i++) {
        if (hasTeamPokeMove(id,i,movenum))
            return true;
    }
    return false;
}

QScriptValue ScriptEngine::teamPokeItem(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).item();
    }
}

bool ScriptEngine::hasTeamItem(int id, int itemnum)
{
    if (!loggedIn(id)) {
        printLine("Script Warning in sys.hasTeamPoke(id, pokenum): no such player logged in with id " + QString::number(id));
        return false;
    }
    TeamBattle &t = myserver->player(id)->team();
    for (int i = 0; i < 6; i++) {
        if (t.poke(i).item() == itemnum) {
            return true;
        }
    }
    return false;
}

long ScriptEngine::time()
{
    return ::time(NULL);
}

QScriptValue ScriptEngine::getTierList()
{
    QStringList origin = TierMachine::obj()->tierNames();

    QScriptValue ret = myengine.newArray(origin.count());

    for(int i = 0;i < origin.size(); i++) {
        ret.setProperty(i, origin[i]);
    }

    return  ret;
}

QScriptValue ScriptEngine::playerIds()
{
    QList<int> keys = myserver->myplayers.keys();

    QScriptValue ret = myengine.newArray(keys.count());

    for (int i = 0; i < keys.size(); i++) {
        ret.setProperty(i, keys[i]);
    }

    return ret;
}

QScriptValue ScriptEngine::channelIds()
{
    QList<int> keys = myserver->channels.keys();

    QScriptValue ret = myengine.newArray(keys.count());

    for (int i = 0; i < keys.size(); i++) {
        ret.setProperty(i, keys[i]);
    }

    return ret;
}

QScriptValue ScriptEngine::channel(int id)
{
    if (!myserver->channelExist(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->channel(id).name;
    }
}

QScriptValue ScriptEngine::channelId(const QString &name)
{
    if (!myserver->channelExist(name)) {
        return myengine.undefinedValue();
    } else {
        return myserver->channelids.value(name.toLower());
    }
}

bool ScriptEngine::isInChannel(int playerid, int channelid)
{
    if (!loggedIn(playerid))
        return false;
    return (myserver->player(playerid)->getChannels().contains(channelid));
}

bool ScriptEngine::isInSameChannel(int playerid, int player2)
{
    if (!loggedIn(playerid) || !loggedIn(player2))
        return false;
    return (myserver->player(playerid)->isInSameChannel(myserver->player(player2)));
}

QScriptValue ScriptEngine::channelsOfPlayer(int playerid)
{
    if (!myserver->playerExist(playerid)) {
        return myengine.undefinedValue();
    } else {
        Player *p = myserver->player(playerid);

        QSet<int> chans = p->getChannels();
        int i = 0;
        QScriptValue ret = myengine.newArray(chans.count());

        foreach(int chan, chans) {
            ret.setProperty(i, chan);
            i += 1;
        }

        return ret;
    }
}

QScriptValue ScriptEngine::playersOfChannel(int channelid)
{
    if (!myserver->channelExist(channelid)) {
        return myengine.undefinedValue();
    } else {
        Channel &c = myserver->channel(channelid);

        int i = 0;
        QScriptValue ret = myengine.newArray(c.players.count());

        foreach(Player *p, c.players) {
            ret.setProperty(i, p->id());
            i += 1;
        }

        return ret;
    }
}

QScriptValue ScriptEngine::teamPokeNature(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).nature();
    }
}

QScriptValue ScriptEngine::teamPokeEV(int id, int index, int stat)
{
    if (!loggedIn(id) || index < 0 || index >= 6 || stat < 0 || stat >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).evs()[stat];
    }
}

QScriptValue ScriptEngine::teamPokeDV(int id, int index, int stat)
{
    if (!loggedIn(id) || index < 0 || index >= 6 || stat < 0 || stat >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).dvs()[stat];
    }
}

void ScriptEngine::setTeamPokeDV(int id, int slot, int stat, int newValue)
{
    if(loggedIn(id) && slot >=0 && slot <=5 && stat >=0 && stat <= 5 && newValue >= 0 && newValue <= 31) {
        myserver->player(id)->team().poke(slot).dvs()[stat] = newValue;
    }
}

void ScriptEngine::changeTeamPokeIV(int id, int slot, int stat, int newValue)
{
    return this->setTeamPokeDV(id, slot, stat, newValue);
}

void ScriptEngine::changeTeamPokeEV(int id, int slot, int stat, int newValue)
{
    if(loggedIn(id) && slot >=0 && slot <6 && stat >=0 && stat <6 && newValue >= 0 && newValue <= 255) {
        int total = 0;
        for (int i=0; i<6; i++) {
            if (i == stat)
                total += newValue;
            else
                total += myserver->player(id)->team().poke(slot).evs()[i];
        }
        if (total <= 510)
            myserver->player(id)->team().poke(slot).evs()[stat] = newValue;
    }
}

int ScriptEngine::rand(int min, int max)
{
    if (min == max)
        return min;
    return (::rand()%(max-min)) + min;
}

int ScriptEngine::numPlayers()
{
    return myserver->numberOfPlayersLoggedIn;
}

bool ScriptEngine::loggedIn(int id)
{
    return myserver->playerLoggedIn(id);
}

void ScriptEngine::printLine(const QString &s)
{
    myserver->printLine(s, false, true);
}

void ScriptEngine::stopEvent()
{
    if (stopevents.size() == 0) {
        printLine("Script Warning: calling sys.stopEvent() in an unstoppable event.");
    } else {
        stopevents.back() = true;
    }
}

void ScriptEngine::shutDown()
{
    Server::print("Scripted server shutdown");
    serverShutDown();
    exit(0);
}

void ScriptEngine::modifyTypeChart(int type_attack, int type_defend, const QString &modifier)
{
    QString compare_to = modifier.toLower();
    QString modifiers[] = { "none", "ineffective", "normal", "effective" };
    int real_modifiers[] = { 0, 1, 2, 4 };
    int i = 0;
    while((i < 4) && (modifiers[i] != compare_to)) i++;
    if(i < 4) TypeInfo::modifyTypeChart(type_attack, type_defend, real_modifiers[i]);
}

QScriptValue ScriptEngine::type(int id)
{
    if (id >= 0  && id < TypeInfo::NumberOfTypes()) {
        return TypeInfo::Name(id);
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::typeNum(const QString &typeName)
{
    int num = TypeInfo::Number(convertToSerebiiName(typeName));
    if (num >= 0  && num < TypeInfo::NumberOfTypes()) {
        return num;
    } else {
        return myengine.undefinedValue();
    }
}

int ScriptEngine::hiddenPowerType(int gen, quint8 hpdv, quint8 attdv, quint8 defdv, quint8 spddv, quint8 sattdv, quint8 sdefdv)
{
    return HiddenPowerInfo::Type(gen, hpdv, attdv, defdv, spddv, sattdv, sdefdv);
}

ScriptWindow::ScriptWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Scripts"));

    QGridLayout *l = new QGridLayout(this);
    myedit = new QTextEdit();
    l->addWidget(myedit, 0, 0, 1, 4);

    QPushButton *ok = new QPushButton(tr("&Ok"));
    QPushButton *cancel = new QPushButton(tr("&Cancel"));
    QPushButton *gotoLine = new QPushButton(tr("&Goto Line"));

    l->addWidget(cancel,1,1);
    l->addWidget(ok,1,2);
    l->addWidget(gotoLine, 1, 3);

    connect(ok, SIGNAL(clicked()), SLOT(okPressed()));
    connect(cancel, SIGNAL(clicked()), SLOT(deleteLater()));
    connect(gotoLine, SIGNAL(clicked()), SLOT(gotoLine()));

    QFile f("scripts.js");
    f.open(QIODevice::ReadOnly);

    myedit->setPlainText(QString::fromUtf8(f.readAll()));
    myedit->setFont(QFont("Courier New", 10));
    myedit->setLineWrapMode(QTextEdit::NoWrap);
    myedit->setTabStopWidth(25);

    resize(700, 550);
}

void ScriptWindow::okPressed()
{
    QFile f("scripts.js");
    f.open(QIODevice::WriteOnly);

    QString plainText = myedit->toPlainText();
    f.write(plainText.toUtf8());

    emit scriptChanged(plainText);

    close();
}

void ScriptWindow::gotoLine()
{
    int line = QInputDialog::getInteger(myedit, tr("Line Number"), tr("To what line do you want to go?"), 1, 1, myedit->document()->lineCount());
    QTextCursor myCursor = myedit->textCursor();
    myCursor.setPosition(0);
    myCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
    myedit->setTextCursor(myCursor);
    myedit->setFocus();
}

QScriptValue ScriptEngine::getScript()
{
    return myscript;
}

int ScriptEngine::pokeType1(int id, int gen)
{
    int result = Pokemon::Curse;
    if((gen >= GEN_MIN) && (gen <= GEN_MAX)) {
        result = PokemonInfo::Type1(Pokemon::uniqueId(id), gen);
    }else{
        warn("pokeType1", "generation is not supported.");
    }
    return result;
}

int ScriptEngine::pokeType2(int id, int gen)
{
    int result = Pokemon::Curse;
    if((gen >= GEN_MIN) && (gen <= GEN_MAX)) {
        result = PokemonInfo::Type2(Pokemon::uniqueId(id), gen);
    }else{
        warn("pokeType2", "generation is not supported.");
    }
    return result;
}

void ScriptEngine::modifyMovePower(int moveNum, unsigned char power, int gen)
{
    MoveInfo::setPower(moveNum, power, gen);
}

void ScriptEngine::modifyMoveAccuracy(int moveNum, char accuracy, int gen)
{
    MoveInfo::setAccuracy(moveNum, accuracy, gen);
}

void ScriptEngine::modifyMovePP(int moveNum, char pp, int gen)
{
    MoveInfo::setPP(moveNum, pp, gen);
}

void ScriptEngine::modifyMovePriority(int moveNum, qint8 priority, int gen)
{
    MoveInfo::setPriority(moveNum, priority, gen);
}

QScriptValue ScriptEngine::banList()
{
    QList<QString> keys = SecurityManager::banList().keys();
    int size = keys.size();
    QScriptValue result = myengine.newArray(size);
    for(int i = 0; i < size; i++) {
        result.setProperty(i, keys.at(i));
    }
    return result;
}

void ScriptEngine::ban(QString name)
{
    SecurityManager::ban(name);
}

void ScriptEngine::unban(QString name)
{
    SecurityManager::unban(name);
}

void ScriptEngine::battleSetup(int src, int dest, int battleId)
{
    makeEvent("battleSetup", src, dest, battleId);
}

void ScriptEngine::prepareWeather(int battleId, int weatherId)
{
    if((weatherId >= 0) && (weatherId <= 4)) {
        BattleSituation * battle = myserver->getBattle(battleId);
        if (battle) {
            battle->setupLongWeather(weatherId);
        }else{
            warn("prepareWeather", "can't find a battle with specified id.");
        }
    }
}

QScriptValue ScriptEngine::weatherNum(const QString &weatherName)
{
    QString weatherSimplified = weatherName.toLower();
    bool found = false;
    int i = 0;
    while(i <= 4) {
        if(weatherSimplified == TypeInfo::weatherName(i)) {
            found = true;
            break;
        }
        ++i;
    }
    if (found) {
        return i;
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::weather(int weatherId)
{
    if (weatherId >= 0  && weatherId < 4) {
        return TypeInfo::weatherName(weatherId);
    } else {
        return myengine.undefinedValue();
    }
}

void ScriptEngine::setAnnouncement(const QString &html, int id) {
    if (testPlayer("setAnnouncment(html, id)", id)) {
        myserver->setAnnouncement(id, html); }
    }
void ScriptEngine::setAnnouncement(const QString &html) {
        myserver->setAllAnnouncement(html);
    }

void ScriptEngine::changeAnnouncement(const QString &html) {
        QSettings settings("config", QSettings::IniFormat);
        settings.setValue("server_announcement", html);
        myserver->announcementChanged(html);
    }

void ScriptEngine::makeServerPublic(bool isPublic)
{
    myserver->regPrivacyChanged(!isPublic);
}

QScriptValue ScriptEngine::getAnnouncement() {
    return myserver->serverAnnouncement;
}

/* Causes crash...
void ScriptEngine::setTimer(int milisec) {
    if (milisec <= 0)
        return;
    step_timer->stop();
    step_timer->start(ms);
    }
*/
int ScriptEngine::teamPokeAbility(int id, int slot)
{
    if (!loggedIn(id) || slot < 0 || slot >= 6) {
        return Ability::NoAbility;
    } else {
        return myserver->player(id)->team().poke(slot).ability();
    }
}

void ScriptEngine::changeName(int playerId, QString newName)
{
    if (!loggedIn(playerId)) return;
    myserver->player(playerId)->setName(newName);
    myserver->sendPlayer(playerId);
}

void ScriptEngine::changeInfo(int playerId, QString newInfo)
{
    if (!loggedIn(playerId)) return;
    myserver->player(playerId)->setInfo(newInfo);
    myserver->sendPlayer(playerId);
}

QScriptValue ScriptEngine::info(int playerId)
{
    if (loggedIn(playerId)) {
        return myserver->player(playerId)->team().info;
    }else{
        return myengine.undefinedValue();
    }
}

void ScriptEngine::modifyPokeAbility(int id, int slot, int ability, int gen)
{
    bool res = PokemonInfo::modifyAbility(Pokemon::uniqueId(id), slot, ability, gen);
    if (!res) {
        warn(
            "modifyPokeAbility",
            QString("slot out of range or pokemon do not exist in gen %1.").arg(QString::number(gen))
        );
    }
}

void ScriptEngine::changePokeAbility(int id, int slot, int ability)
{
    if (!testPlayer("changePokeAbility", id) || !testRange("changePokeAbility", slot, 0, 5)) {
        return;
    }
    myserver->player(id)->team().poke(slot).ability() = ability;
}

QScriptValue ScriptEngine::pokeAbility(int poke, int slot, int gen)
{
    Pokemon::uniqueId pokemon(poke);
    if (PokemonInfo::Exists(pokemon, gen)
        && (slot >= 0) && (slot <= 2)
        && (gen >= GEN_MIN) && (gen <= GEN_MAX)) {
        return PokemonInfo::Abilities(pokemon, gen).ab(slot);
    }
    return myengine.undefinedValue();
}

void ScriptEngine::changePokeHappiness(int id, int slot, int value)
{
    if (!testPlayer("changePokeHappiness", id)
        || !testRange("changePokeHappiness", slot, 0, 5)
        || !testRange("changePokeHappiness", value, 0, 255)) {
        return;
    }
    myserver->player(id)->team().poke(slot).happiness() = value;
}

void ScriptEngine::changePokeShine(int id, int slot, bool value)
{
    if (!testPlayer("changePokeShine", id) || !testRange("changePokeShine", slot, 0, 5)) {
        return;
    }
    myserver->player(id)->team().poke(slot).shiny() = value;
}

void ScriptEngine::changePokeNature(int id, int slot, int nature)
{
    if(!testPlayer("changePokeNature(id, slot, forme)", id) || !testRange("changePokeNature(id, slot, forme)",slot, 0, 15))
        return;
      // Ugly, we don't have NatureInfo::Exists(nature) or we do?
    myserver->player(id)->team().poke(slot).nature() = nature;
}

QScriptValue ScriptEngine::teamPokeGender(int id, int slot)
{
    if (!testPlayer("teamPokeGender", id) || !testRange("teamPokeGender", slot, 0, 5)) {
        return myengine.undefinedValue();
    }
    return myserver->player(id)->team().poke(slot).gender();
}

QScriptValue ScriptEngine::teamPokeNick(int id, int index)
{
    if(!loggedIn(id) || index < 0 ||index >= 6) {
        return myengine.undefinedValue();
    }else{
        return myserver->player(id)->team().poke(index).nick();
    }
}

void ScriptEngine::inflictStatus(int battleId, bool toFirstPlayer, int slot, int status)
{
    if (!testRange("inflictStatus", status, Pokemon::Fine, Pokemon::Koed)
        || !testRange("inflictStatus", slot, 0, 5)) {
        return;
    }
    BattleSituation * battle = myserver->getBattle(battleId);
    if (battle) {
        if (toFirstPlayer) {
            battle->changeStatus(0, slot, status);
        }else{
            battle->changeStatus(1, slot, status);
        }
    }else{
        warn("inflictStatus", "can't find a battle with specified id.");
    }
}

void ScriptEngine::modifyPokeStat(int poke, int stat, quint8 value)
{
    bool res = PokemonInfo::modifyBaseStat(Pokemon::uniqueId(poke), stat, value);
    if (!res) {
        warn("modifyPokeStat", "unable to modify.");
    }
}

void ScriptEngine::updateRatings()
{
    TierMachine::obj()->processDailyRun();
}

void ScriptEngine::resetLadder(const QString &tier)
{
    if (!TierMachine::obj()->exists(tier)) {
        warn("resetLadder", "tier doesn't exist");
        return;
    }

    TierMachine::obj()->tier(tier).resetLadder();

    /* Updates the rating of all the players of the tier */
    foreach(Player *p, myserver->myplayers) {
        if (p->tier() == tier)
            p->findRating();
    }
}

void ScriptEngine::synchronizeTierWithSQL(const QString &tier)
{
    if (!TierMachine::obj()->exists(tier)) {
        warn("resetLadder", "tier doesn't exist");
        return;
    }

    TierMachine::obj()->tier(tier).clearCache();

    /* Updates the rating of all the players of the tier */
    foreach(Player *p, myserver->myplayers) {
        if (p->tier() == tier)
            p->findRating();
    }
}

void ScriptEngine::forceBattle(int player1, int player2, int clauses, int mode, bool is_rated)
{
    if (!loggedIn(player1) || !loggedIn(player2)) {
        warn("forceBattle", "player is not online.");
        return;
    }
    if (player1 == player2) {
        warn("forceBattle", "player1 == player2");
        return;
    }
    if (!testRange("forceBattle", mode, ChallengeInfo::ModeFirst, ChallengeInfo::ModeLast)) {
        return;
    }

    ChallengeInfo c;
    c.clauses = clauses;
    c.mode = mode;
    c.rated = is_rated;
   
    myserver->startBattle(player1, player2, c);
}

void ScriptEngine::sendNetworkCommand(int id, int command)
{
    if (!testPlayer("sendNetworkCommand(id, command)", id))
        return;

    myserver->player(id)->relay().notify(command);
}

int ScriptEngine::getClauses(const QString &tier)
{
    return TierMachine::obj()->tier(tier).getClauses();
}

bool ScriptEngine::attemptToSpectateBattle(int src, int p1, int p2)
{
    if (!myscript.property("attemptToSpectateBattle", QScriptValue::ResolveLocal).isValid()) {
        return false;
    }
    QScriptValue res = myscript.property("attemptToSpectateBattle").call(myscript, QScriptValueList() << src << p1 << p2);
    if (res.isError()) {
        printLine(QString("Script Error line %1: %2").arg(myengine.uncaughtExceptionLineNumber()).arg(res.toString()));
    }
    return res.isString() && (res.toString().toLower() == "allow");
}

void ScriptEngine::changeAvatar(int playerId, quint16 avatarId)
{
    if (!loggedIn(playerId)) {
        warn("changeAvatar","unknown player.");
        return;
    }
    myserver->player(playerId)->avatar() = avatarId;
    myserver->sendPlayer(playerId);
}

QScriptValue ScriptEngine::avatar(int playerId)
{
    if (!loggedIn(playerId)) {
        return myengine.nullValue();
    }
    return myserver->player(playerId)->avatar();
}

// Potentially unsafe functions.
#ifndef PO_SCRIPT_SAFE_ONLY

void ScriptEngine::saveVal(const QString &key, const QVariant &val)
{
    QSettings s;
    s.setValue("Script_"+key, val);
}

QScriptValue ScriptEngine::getVal(const QString &key)
{
    QSettings s;
    return s.value("Script_"+key).toString();
}

void ScriptEngine::removeVal(const QString &key)
{
    QSettings s;
    s.remove("Script_"+key);
}

void ScriptEngine::saveVal(const QString &file, const QString &key, const QVariant &val)
{
    QSettings s(file, QSettings::IniFormat);
    s.setValue("Script_"+key, val);
}

QScriptValue ScriptEngine::getVal(const QString &file, const QString &key)
{
    QSettings s(file, QSettings::IniFormat);
    return s.value("Script_"+key).toString();
}

void ScriptEngine::removeVal(const QString &file, const QString &key)
{
    QSettings s(file, QSettings::IniFormat);
    s.remove("Script_"+key);
}

void ScriptEngine::appendToFile(const QString &fileName, const QString &content)
{
    QFile out(fileName);

    if (!out.open(QIODevice::Append)) {
        printLine("Script Warning in sys.appendToFile(filename, content): error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.write(content.toUtf8());
}

void ScriptEngine::writeToFile(const QString &fileName, const QString &content)
{
    QFile out(fileName);

    if (!out.open(QIODevice::WriteOnly)) {
        printLine("Script Warning in sys.writeToFile(filename, content): error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.write(content.toUtf8());
}

void ScriptEngine::deleteFile(const QString &fileName)
{
    QFile out(fileName);

    if (!out.open(QIODevice::WriteOnly)) {
        printLine("Script Warning in sys.deleteFile(filename): error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.remove();
}

/**
 * Function will perform a GET-Request server side
 * @param urlstring web-url
 * @author Remco vd Zon
 */
void ScriptEngine::webCall(const QString &urlstring, const QScriptValue &callback)
{
    if (!callback.isString() && !callback.isFunction()) {
        printLine("Script Warning in sys.webCall(urlstring, callback): callback is not a string or a function.");
        return;
    }

    QNetworkRequest request;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online serverscript");

    QNetworkReply *reply = manager.get(request);
    webCallEvents[reply] = callback;
}

/**
 * Function will perform a POST-Request server side
 * @param urlstring web-url
 * @param params_array javascript array [key]=>value.
 * @author Remco vd Zon
 */
void ScriptEngine::webCall(const QString &urlstring, const QScriptValue &callback, const QScriptValue &params_array)
{
    if (!callback.isString() && !callback.isFunction()) {
        printLine("Script Warning in sys.webCall(urlstring, callback, params_array): callback is not a string or a function.");
        return;
    }
    
    QNetworkRequest request;
    QByteArray postData;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online serverscript");

    //parse the POST fields
    QScriptValueIterator it(params_array);
    while (it.hasNext()) {
        it.next();
        postData.append( it.name() + "=" + it.value().toString().replace(QString("&"), QString("%26"))); //encode ampersands!
        if(it.hasNext()) postData.append("&");
    }

    QNetworkReply *reply = manager.post(request, postData);
    webCallEvents[reply] = callback;
}

void ScriptEngine::webCall_replyFinished(QNetworkReply* reply){
    QScriptValue val = webCallEvents.take(reply);
    if (val.isString()) {
        //escape reply before sending it to the javascript evaluator
        QString x = reply->readAll();
        x = x.replace(QString("\\"), QString("\\\\"));
        x = x.replace(QString("'"), QString("\\'"));
        x = x.replace(QString("\n"), QString("\\n"));
        x = x.replace(QString("\r"), QString(""));

        //put reply in a var "resp", can be used in expr
        // i.e. expr = 'print("The resp was: "+resp);'
        eval( "var resp = '"+x+"';"+val.toString());
    } else if (val.isFunction()) {
        QScriptValueList args;
        args << QString(reply->readAll());
        val.call(QScriptValue(), args); // uses globalObject as this
    }
    reply->deleteLater();
}

/**
 * Function will perform a GET-Request server side, synchronously
 * @param urlstring web-url
 * @author Remco cd Zon and Toni Fadjukoff
 */
QScriptValue ScriptEngine::synchronousWebCall(const QString &urlstring) {
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online serverscript");

    connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(synchronousWebCall_replyFinished(QNetworkReply*)));
    manager->get(request);

    sync_loop.exec();

    manager->deleteLater();
    return sync_data;
}

/**
 * Function will perform a POST-Request server side, synchronously
 * @param urlstring web-url
 * @param params_array javascript array [key]=>value.
 * @author Remco vd Zon and Toni Fadjukoff
 */
QScriptValue ScriptEngine::synchronousWebCall(const QString &urlstring, const QScriptValue &params_array)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    QByteArray postData;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online serverscript");

    //parse the POST fields
    QScriptValueIterator it(params_array);
    while (it.hasNext()) {
        it.next();
        postData.append( it.name() + "=" + it.value().toString().replace(QString("&"), QString("%26"))); //encode ampersands!
        if(it.hasNext()) postData.append("&");
    }

    connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(synchronousWebCall_replyFinished(QNetworkReply*)));
    manager->post(request, postData);

    sync_loop.exec();
    manager->deleteLater();
    return sync_data;
}

void ScriptEngine::synchronousWebCall_replyFinished(QNetworkReply* reply) {
    sync_data = reply->readAll();
    sync_loop.exit();
}

QScriptValue ScriptEngine::getValKeys()
{
    QSettings s;
    QStringList list = s.childKeys();
    QStringList result_data;
    
    QStringListIterator it(list);
    while (it.hasNext()) {
        QString v = it.next();
        if (v.startsWith("Script_")) {
            result_data.append(v.mid(7));
        }
    }
    int len = result_data.length();
    QScriptValue result_array = myengine.newArray(len);
    for (int i = 0; i < len; ++i) {
        result_array.setProperty(i, result_data.at(i));
    }
    return result_array;
}

QScriptValue ScriptEngine::getValKeys(const QString &file)
{
    QSettings s(file, QSettings::IniFormat);
    QStringList list = s.childKeys();
    QStringList result_data;
    
    QStringListIterator it(list);
    while (it.hasNext()) {
        QString v = it.next();
        if (v.startsWith("Script_")) {
            result_data.append(v.mid(7));
        }
    }
    int len = result_data.length();
    QScriptValue result_array = myengine.newArray(len);
    for (int i = 0; i < len; ++i) {
        result_array.setProperty(i, result_data.at(i));
    }
    return result_array;
}

QScriptValue ScriptEngine::getFileContent(const QString &fileName)
{
    QFile out(fileName);

    if (!out.open(QIODevice::ReadOnly)) {
        printLine("Script Warning in sys.getFileContent(filename): error when opening " + fileName + ": " + out.errorString());
        return myengine.undefinedValue();
    }

    return QString::fromUtf8(out.readAll());
}

#endif // PO_SCRIPT_SAFE_ONLY

#if !defined(PO_SCRIPT_NO_SYSTEM) && !defined(PO_SCRIPT_SAFE_ONLY)
int ScriptEngine::system(const QString &command)
{
    if (myserver->isSafeScripts()) {
        warn("system", "Safe scripts option is on. Unable to invoke system command.");
        return -1;
    } else {
        return ::system(command.toUtf8());
    }
}
#endif // PO_SCRIPT_NO_SYSTEM

QScriptValue ScriptEngine::teamPokeShine(int id, int slot)
{
    if (!testPlayer("teamPokeShine", id) || !testRange("teamPokeShine", slot, 0, 5)) {
        return myengine.undefinedValue();
    }
    return myserver->player(id)->team().poke(slot).shiny();
}

int ScriptEngine::moveType(int moveNum, int gen)
{
    return MoveInfo::Type(moveNum, gen);
}

QString ScriptEngine::serverVersion()
{
    return VERSION;
}

bool ScriptEngine::isServerPrivate()
{
    return myserver->isPrivate();
}
