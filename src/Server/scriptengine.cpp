#include <stdexcept>

#include <QInputDialog>
#include <QRegExp>

#include "../Shared/config.h"
#include <Utilities/antidos.h>
#include <Utilities/ziputils.h>
#include <PokemonInfo/pokemoninfo.h>
#include <PokemonInfo/movesetchecker.h>

#include "server.h"
#include "player.h"
#include "security.h"
#include "waitingobject.h"
#include "tiermachine.h"
#include "tier.h"
#include "pluginmanager.h"
#include "analyze.h"
#include "battlecommunicator.h"

#include "sessiondatafactory.h"
#include "scriptengineagent.h"
#include "scriptengine.h"

#ifndef _EXCLUDE_DEPRECATED
#define DEPRECATED(x) x
#else
#define DEPRECATED(x)
#endif

DEPRECATED(
    static bool callLater_w = false;
    static bool callQuickly_w = false;
    static bool quickCall_w = false;
    static bool delayedCall_w = false;
    static bool intervalCall_w = false;
    static bool intervalTimer_w = false;
    static bool stopTimer_w = false;
)

QScriptValue ScriptEngine::lighter(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() != 1 && ctxt->argumentCount() != 2)
        return ctxt->throwError(QLatin1String("Qt.lighter(): Invalid arguments"));
    QColor color = QColor(ctxt->argument(0).toString());
    if (!color.isValid()) {
        return engine->nullValue();
    }
    qsreal factor = 1.5;
    if (ctxt->argumentCount() == 2)
        factor = ctxt->argument(1).toNumber();
    color = color.lighter(int(qRound(factor*100.)));
    return color.name();
}

QScriptValue ScriptEngine::darker(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() != 1 && ctxt->argumentCount() != 2)
        return ctxt->throwError(QLatin1String("Qt.darker(): Invalid arguments"));
    QColor color = QColor(ctxt->argument(0).toString());
    if (!color.isValid()) {
        return engine->nullValue();
    }
    qsreal factor = 2.0;
    if (ctxt->argumentCount() == 2)
        factor = ctxt->argument(1).toNumber();
    color = color.darker(int(qRound(factor*100.)));
    return color.name();
}

/* Returns lightness of a color */
QScriptValue ScriptEngine::lightness(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() != 1)
        return ctxt->throwError(QLatin1String("Qt.lightness(): Invalid arguments"));
    QColor color = QColor(ctxt->argument(0).toString());
    if (!color.isValid()) {
        return engine->nullValue();
    }
    return color.lightnessF();
}

QScriptValue ScriptEngine::tint(QScriptContext *ctxt, QScriptEngine *engine)
{
    if(ctxt->argumentCount() != 2)
        return ctxt->throwError(QLatin1String("Qt.tint(): Invalid arguments"));
    //get color
    QColor color = QColor(ctxt->argument(0).toString());
    QColor tintColor = QColor(ctxt->argument(1).toString());

    if (!color.isValid() || !tintColor.isValid()) {
        return engine->nullValue();
    }

    //tint
    QColor finalColor;
    int a = tintColor.alpha();
    if (a == 0xFF)
        finalColor = tintColor;
    else if (a == 0x00)
        finalColor = color;
    else {
        qreal a = tintColor.alphaF();
        qreal inv_a = 1.0 - a;

        finalColor.setRgbF(tintColor.redF() * a + color.redF() * inv_a,
                           tintColor.greenF() * a + color.greenF() * inv_a,
                           tintColor.blueF() * a + color.blueF() * inv_a,
                           a + inv_a * color.alphaF());
    }

    return finalColor.name();
}


ScriptEngine::ScriptEngine(Server *s) {
    setParent(s);
    myserver = s;
    performanceTimer.start();
    resetPerfs = false;

    myengine.setParent(this);

    parse = myengine.globalObject().property("JSON").property("parse");
    stringify = myengine.globalObject().property("JSON").property("stringify");

    ScriptEngineAgent *b = new ScriptEngineAgent(&myengine);
    QScriptEngineAgent *bt = b;

    myengine.setAgent(bt);

    mySessionDataFactory = new SessionDataFactory(this);

    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("global", myengine.globalObject());
    myengine.globalObject().setProperty("sys", sys);
    QScriptValue printfun = myengine.newFunction(nativePrint);
    printfun.setData(sys);
    myengine.globalObject().setProperty("print", printfun);
    myengine.globalObject().setProperty(
                "SESSION",
                myengine.newQObject(mySessionDataFactory),
                QScriptValue::ReadOnly | QScriptValue::Undeletable
                );

    QScriptValue qtObject = myengine.newObject();
    qtObject.setProperty("lighter", myengine.newFunction(&lighter, 1));
    qtObject.setProperty("darker", myengine.newFunction(&darker, 1));
    qtObject.setProperty("lightness", myengine.newFunction(&lightness, 1));
    qtObject.setProperty("tint", myengine.newFunction(&tint, 2));
    myengine.globalObject().setProperty("Qt", qtObject);


    sys.setProperty( "enableStrict" , myengine.newFunction(enableStrict));

#ifndef PO_SCRIPT_SAFE_ONLY
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
    QScriptValue writeFunc = myengine.newFunction(write);
    sys.setProperty( "write" , writeFunc);
    sys.setProperty( "writeToFile" , writeFunc);
    QScriptValue readFunc = myengine.newFunction(read);
    sys.setProperty( "read" , readFunc);
    sys.setProperty( "getFileContent" , readFunc);
    QScriptValue mkdirf = myengine.newFunction(mkdir);
    sys.setProperty( "mkdir" , mkdirf);
    sys.setProperty( "makeDir" , mkdirf);
    QScriptValue rmF = myengine.newFunction(rm);
    sys.setProperty( "deleteFile" , rmF);
    sys.setProperty( "rm" , rmF);
    QScriptValue wroF = myengine.newFunction(writeObject);
    sys.setProperty( "writeObject" , wroF);
    QScriptValue rdoF = myengine.newFunction(readObject);
    sys.setProperty( "readObject" , rdoF);
    QScriptValue cwdf = myengine.newFunction(cwd);
    sys.setProperty( "cwd" , cwdf);
    sys.setProperty( "getCurrentDir" , cwdf);
    QScriptValue rmdF = myengine.newFunction(rmdir);
    sys.setProperty( "removeDir" , rmdF);
    sys.setProperty( "rmdir" , rmdF);
    QScriptValue apf = myengine.newFunction(writeConcat);
    sys.setProperty( "append" , apf);
    sys.setProperty( "appendToFile" , apf);

    sys.setProperty( "fileExists" , myengine.newFunction(exists));
    sys.setProperty( "fexists" , myengine.newFunction(exists));

    sys.setProperty( "exec" , myengine.newFunction(exec));

#endif
    sys.setProperty( "sendAll" , myengine.newFunction(sendAll));
    sys.setProperty( "sendMessage" , myengine.newFunction(sendMessage));
    sys.setProperty( "broadcast" , myengine.newFunction(broadcast));

    sys.setProperty( "backtrace" , myengine.newFunction(backtrace));

    QTimer *step_timer = new QTimer(this);
    step_timer->setSingleShot(false);
    step_timer->start(1000);
    connect(step_timer, SIGNAL(timeout()), SLOT(timer_step()));
}

void ScriptEngine::init()
{
    QFile f("scripts.js");
    f.open(QIODevice::ReadOnly);

    changeScript(QString::fromUtf8(f.readAll()));
}

ScriptEngine::~ScriptEngine()
{
    delete mySessionDataFactory;
}

void ScriptEngine::changeScript(const QString &script, const bool triggerStartUp)
{
    QScriptValue newscript;
    QScriptValue oldscript;
    oldscript = myscript;

    makeEvent("unloadScript");
    // allow the script to write to file and etc.
    // clean up after itself.

    callLater_w = false;
    callQuickly_w = false;
    quickCall_w = false;
    delayedCall_w = false;
    intervalCall_w = false;
    intervalTimer_w = false;
    stopTimer_w = false;

    strict = false;
    wfatal = false;

    newscript = myengine.evaluate(script, "scripts.js");

    if (newscript.isError()) {
        strict = false;
        wfatal = false;
        makeEvent("switchError", newscript);
        printLine("Script Check: Fatal script error on line " + QString::number(myengine.uncaughtExceptionLineNumber()) + ": " + newscript.toString() + "\n" +myengine.uncaughtException().property("backtracetext").toString());

    } else {
        myscript = newscript;
        myengine.globalObject().setProperty("script", myscript);

        if (!makeSEvent("loadScript")) {
            myscript = oldscript;
            myengine.globalObject().setProperty("script", myscript);
            strict = false;
            wfatal = false;
            makeEvent("switchError", newscript);
            printLine("Script Check: Script rejected server. Maybe it requires a newer version?");
            return;
        }

        printLine("Script Check: OK");

        if(triggerStartUp) {
            serverStartUp();
        }

        mySessionDataFactory->refill(SessionDataFactory::ConstructGlobal);
    }
}

QScriptValue ScriptEngine::backtrace(QScriptContext *c, QScriptEngine *)
{
    return c->backtrace().join("\n");
}

QScriptValue ScriptEngine::exec(QScriptContext *c, QScriptEngine *e)
{
    ScriptEngine *po = dynamic_cast<ScriptEngine*>(e->parent());
    QString url = c->argument(0).toString();
    QFile in(url);

    if (!in.open(QIODevice::ReadOnly)) {
        po->warn("exec(filename)", in.errorString(), true);
        return QScriptValue();
    }

    QScriptValue import = e->evaluate(QString::fromUtf8(in.readAll()), c->argument(0).toString());
    if (e->hasUncaughtException()) {
        import.setProperty("backtracetext", c->backtrace().join("\n"));
        c->throwValue(import);
    }
    return import;
}

QScriptValue ScriptEngine::import(const QString &fileName) {
    QString url = "scripts/"+fileName;
    QFile in(url);

    if (!in.open(QIODevice::ReadOnly)) {
        warn("import", in.errorString(), true);
        return QScriptValue();
    }

    QScriptValue import = myengine.evaluate(QString::fromUtf8(in.readAll()), url );

    if (myengine.hasUncaughtException()) {
        import.setProperty("backtracetext", myengine.currentContext()->backtrace().join("\n"));
        myengine.currentContext()->throwValue(import);
    }

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
            warn(function, QString("Invalid channel ID."), true);
        return false;
    }

    return true;
}

bool ScriptEngine::testTeamCount(const QString &function, int id, int team)
{
    if (!testPlayer(function, id)) {
        return false;
    }

    if (myserver->player(id)->teamCount() <= team) {
        if (function.length() > 0)
            warn(function, QString("Player numbered %1 only has %2 teams, so you can't access team #%3.").arg(id).arg(myserver->player(id)->teamCount()).arg(team), true);
        return false;
    }

    return true;
}

bool ScriptEngine::testPlayer(const QString &function, int id)
{
    if (!loggedIn(id)) {
        if (function.length() > 0)
            warn(function, QString("Invalid player ID."), true);
        return false;
    }

    return true;
}

bool ScriptEngine::testPlayerInChannel(const QString &function, int id, int chan)
{
    if (!myserver->player(id)->getChannels().contains(chan)) {
        if (function.length() > 0)
            warn(function, QString("Player number %1 is not in channel number %2").arg(id).arg(chan), true);
        return false;
    }

    return true;
}

bool ScriptEngine::testRange(const QString &function, int val, int min, int max)
{
    if (val < min || val > max) {
        if (function.length() > 0)
            warn(function, QString("%1 is out of the range [%2, %3]").arg(val).arg(min).arg(max), true);
        return false;
    }

    return true;
}

void ScriptEngine::warn(const QString &function, const QString &message, bool errinstrict = false)
{
    if (strict && errinstrict) {
        myengine.currentContext()->throwError(message);
        return;
    }

    QString backtrace = myengine.currentContext()->backtrace().join("\n");

    if ( makeSEvent("warning", function, message, backtrace) ) {
        printLine(QString("Script Warning in sys.%1: %2\n%3").arg(function, message, backtrace));
    }
}

quint64 ScriptEngine::startProfiling()
{
    return performanceTimer.elapsed();
}

void ScriptEngine::endProfiling(quint64 startTime, const QString &name)
{
    quint64 elapsed = performanceTimer.elapsed() - startTime;
    profiles[name].calls += 1;
    profiles[name].totalDuration += elapsed;
}

bool ScriptEngine::beforeServerMessage(const QString &message)
{
    return makeSEvent("beforeServerMessage", message);
}

void ScriptEngine::afterServerMessage(const QString &message)
{
    makeEvent("afterServerMessage", message);
}

bool ScriptEngine::beforeChatMessage(int src, const QString &message, int channel)
{
    return makeSEvent("beforeChatMessage", src, message, channel);
}

void ScriptEngine::afterChatMessage(int src, const QString &message, int channel)
{
    makeEvent("afterChatMessage", src, message, channel);
}

bool ScriptEngine::beforeNewPM(int src)
{
    return makeSEvent("beforeNewPM", src);
}

bool ScriptEngine::beforeSpectateBattle(int src, int p1, int p2)
{
    return makeSEvent("beforeSpectateBattle", src, p1, p2);
}

void ScriptEngine::afterSpectateBattle(int src, int p1, int p2)
{
    makeEvent("afterSpectateBattle", src, p1, p2);
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
    makeEvent("serverStartUp");
}

void ScriptEngine::stepEvent()
{
    makeEvent("step");
}

void ScriptEngine::serverShutDown()
{
    makeEvent("serverShutDown");
}

bool ScriptEngine::beforePlayerRegister(int src)
{
    return makeSEvent("beforePlayerRegister", src);
}

bool ScriptEngine::beforeIPConnected(const QString &ip)
{
    return makeSEvent("beforeIPConnected", ip);
}

bool ScriptEngine::beforeLogIn(int src, const QString &defaultChan)
{
    bool login = makeSEvent("beforeLogIn", src, defaultChan);

    if (login && exists(src)) {
        mySessionDataFactory->handleUserLogIn(src);
    }

    return login;
}

void ScriptEngine::afterLogIn(int src, const QString &defaultChan)
{
    makeEvent("afterLogIn", src, defaultChan);
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

bool ScriptEngine::beforeChangeTier(int src, int slot, const QString &oldTier, const QString &newTier)
{
    return makeSEvent("beforeChangeTier", src, slot, oldTier, newTier);
}

void ScriptEngine::afterChangeTier(int src, int slot, const QString &oldTier, const QString &newTier)
{
    makeEvent("afterChangeTier", src, slot, oldTier, newTier);
}

bool ScriptEngine::beforeChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    return makeSEvent("beforeChallengeIssued", src, dest, c.clauses, c.rated, c.mode, c.team, c.desttier);
}

void ScriptEngine::afterChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    makeEvent("afterChallengeIssued", src, dest, c.clauses, c.rated, c.mode, c.team, c.desttier);
}

bool ScriptEngine::beforeBattleMatchup(int src, int dest, const ChallengeInfo &c)
{
    return makeSEvent("beforeBattleMatchup", src, dest, c.clauses, c.rated, c.mode);
}

void ScriptEngine::afterBattleMatchup(int src, int dest, const ChallengeInfo &c)
{
    makeEvent("afterBattleMatchup", src, dest, c.clauses, c.rated, c.mode);
}


void ScriptEngine::beforeBattleStarted(int src, int dest, const ChallengeInfo &c, int id, int team1, int team2)
{
    makeEvent("beforeBattleStarted", src, dest, c.clauses, c.rated, c.mode, id, team1, team2);
}

void ScriptEngine::afterBattleStarted(int src, int dest, const ChallengeInfo &c, int id, int team1, int team2)
{
    makeEvent("afterBattleStarted", src, dest, c.clauses, c.rated, c.mode, id, team1, team2);
}

QString battleDesc[3] = {
    "forfeit",
    "win",
    "tie"
};

void ScriptEngine::beforeBattleEnded(int src, int dest, int desc, int battleid)
{
    if (desc < 0 || desc > 2)
        return;
    makeEvent("beforeBattleEnded", src, dest, battleDesc[desc], battleid);
}

void ScriptEngine::afterBattleEnded(int src, int dest, int desc, int battleid)
{
    if (desc < 0 || desc > 2)
        return;
    makeEvent("afterBattleEnded", src, dest, battleDesc[desc], battleid);
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

bool ScriptEngine::beforePlayerBan(int src, int dest, int time)
{
    return makeSEvent("beforePlayerBan", src, dest, time);
}

void ScriptEngine::afterPlayerBan(int src, int dest, int time)
{
    makeEvent("afterPlayerBan", src, dest, time);
}

bool ScriptEngine::beforePlayerAway(int src, bool away)
{
    return makeSEvent("beforePlayerAway", src, away);
}

void ScriptEngine::afterPlayerAway(int src, bool away)
{
    makeEvent("afterPlayerAway", src, away);
}

void ScriptEngine::battleConnectionLost()
{
    makeEvent("battleConnectionLost");
}

bool ScriptEngine::beforeReconnect(int dest, int src)
{
    return makeSEvent("beforeReconnect", dest, src);
}

void ScriptEngine::afterReconnect(int src)
{
    makeEvent("afterReconnect", src);
}

void ScriptEngine::evaluate(const QScriptValue &expr)
{
    if (expr.isError()) {
        printLine(QString("Script Error line %1: %2").arg(myengine.uncaughtExceptionLineNumber()).arg(expr.toString()) + "\n" +myengine.uncaughtException().property("backtracetext").toString() );
    }
}

QScriptValue ScriptEngine::sendAll(QScriptContext *c, QScriptEngine *e)
{
    ScriptEngine* po = dynamic_cast<ScriptEngine*>(e->parent());
    Server * s = po->myserver;

    if (po->strict ? c->argument(1).isUndefined() || c->argument(1).isNull() : c->argumentCount() <= 1 || c->argument(1).isNull()) {
        s->broadCast(c->argument(0).toString());
        return QScriptValue();
    }
    else if (!s->channelExist(c->argument(1).toInteger())) {
        po->warn("sendAll(mess, channel)","invalid channel", true);
        return QScriptValue();
    }

    s->broadCast(c->argument(0).toString(), c->argument(1).toInteger());

    return QScriptValue();
}

QScriptValue ScriptEngine::broadcast(QScriptContext *c, QScriptEngine *e)
{
    ScriptEngine *se = dynamic_cast<ScriptEngine*>(e->parent());

    Server * s = se->myserver;

    QString m = c->argument(0).toString();

    int channel = c->argument(1).toInteger();

    if (channel != Server::NoChannel && !s->channelExist(channel)) {
        se->warn("broadcast(message, channel, sender, html, target)", "Invalid channel ID.", true);
        return QScriptValue();
    }

    int sender = c->argument(2).toInteger();

    if (sender != Server::NoSender && sender != 0 && !s->playerExist(c->argument(2).toInteger())) {
        se->warn("broadcast(message, channel, sender, html, target)", "Invalid player ID (sender).", true);
        return QScriptValue();
    }
    bool html = c->argument(3).toBool();

    int target = c->argument(4).toInteger();

    if (target != Server::NoTarget && !s->playerExist(target)) {
        se->warn("broadcast(message, channel, sender, html, target)", "Invalid player ID (target).", true);
        return QScriptValue();
    }

    s->broadCast(m, channel, sender, html, target);

    return QScriptValue();


}

QScriptValue ScriptEngine::sendMessage(QScriptContext *c, QScriptEngine *e)
//void ScriptEngine::sendMessage(int id, const QString &mess)
{
    ScriptEngine *po = dynamic_cast<ScriptEngine*>(e->parent());
    Server *myserver = po->myserver;

    if (!myserver->playerExist(c->argument(0).toInteger())) {
        po->warn("sendMessage(id, message, chan)", "Invalid player ID.", true);
        return QScriptValue();
    }



    if ((po->strict && !c->argument(2).isNumber()) || (!po->strict && c->argumentCount() <= 2)) {
        myserver->broadCast(c->argument(1).toString(), Server::NoChannel, Server::NoSender, false, c->argument(0).toInteger());
        return QScriptValue();
    }
    else if ( !myserver->channelExist(c->argument(2).toInteger()) ) {
        po->warn("sendMessage(id, message, chan)", "Invalid channel ID.", true);
        return QScriptValue();
    } else {
        myserver->broadCast(c->argument(1).toString(), c->argument(2).toInteger(), Server::NoSender, false, c->argument(0).toInteger());
    }

    return QScriptValue();
}


void ScriptEngine::sendHtmlAll(const QString &mess)
{
    myserver->broadCast(mess, Server::NoChannel, Server::NoSender, true);
}

void ScriptEngine::sendHtmlAll(const QString &mess, int channel)
{
    if (testChannel("sendHtmlAll(mess, channel)", channel)) {

        myserver->broadCast(mess, channel, Server::NoSender, true);
    }
}

void ScriptEngine::sendHtmlMessage(int id, const QString &mess)
{
    if (testPlayer("sendHtmlMessage(id, mess)", id)) {
        myserver->broadCast(mess, Server::NoChannel, Server::NoSender, true, id);
    }
}

void ScriptEngine::sendHtmlMessage(int id, const QString &mess, int channel)
{
    if (testChannel("sendHtmlMessage(id, mess, channel)", channel) && testPlayer("sendMessage(id, mess, channel)", id) &&
            testPlayerInChannel("sendHtmlMessage(id, mess, channel)", id, channel))
    {
        myserver->broadCast(mess, channel, Server::NoSender, true, id);
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

void ScriptEngine::disconnect(int id)
{
    if (testPlayer("disconnect(id)", id)) {
        myserver->disconnectPlayer(id);
    }
}

void ScriptEngine::updatePlayer(int playerid)
{
    /* Updates all the info of the player to the other players */
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
        warn("putInChannel(id, chan)", QString("player %1 is already in channel %2").arg(id).arg(chanid), true);
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
        warn("clearPass(name)", "no such player name as " + name, true);
    }
    SecurityManager::clearPass(name);
}

void ScriptEngine::changeAuth(int id, int auth)
{
    if (testPlayer("changeAuth(id, auth)", id)) {
        if (myserver->isSafeScripts() && ((myserver->auth(id) > 2) || (auth > 2))) {
            warn("changeAuth(id, auth)", "Safe scripts option is on. Unable to change auth to/from 3 and above.", false);
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
            warn("changeDbAuth(name, auth)", "Safe scripts option is on. Unable to change auth to/from 3 and above.", false);
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
        warn("changeRating(name, tier, rating)", "no such tier as " + tier, true);
    else if (!TierMachine::obj()->tier(tier).exists(name)) {
        warn("changeRating(name, tier, rating)", "player doesn't exist in tier" + tier, true);
    } else {
        TierMachine::obj()->changeRating(name, tier, newRating);
    }
}

void ScriptEngine::changeTier(int id, int team, const QString &tier)
{
    if (!testPlayer("changeTier", id) || !testTeamCount("changeTier", id, team))
        return;
    if (!TierMachine::obj()->exists(tier)) {
        warn("changeTier(id, tier)", "no such tier as " + tier, true);
    } else {
        myserver->player(id)->executeTierChange(team, tier);
    }
}

void ScriptEngine::reloadTiers()
{
    TierMachine::obj()->load();
}

void ScriptEngine::changePokeItem(int id, int team, int slot, int item)
{
    if (!testPlayer("changePokeItem", id) || !testRange("changePokeItem", slot, 0, 5) || !testTeamCount("changePokeItem", id, team))
        return;
    if (!ItemInfo::Exists(item))
        return;
    myserver->player(id)->team(team).poke(slot).item() = item;
}

void ScriptEngine::changePokeNum(int id, int team, int slot, int num)
{
    if (!testPlayer("changePokeNum", id) || !testRange("changePokeNum", slot, 0, 5) || !testTeamCount("changePokeNum", id, team))
        return;
    if (!PokemonInfo::Exists(num, myserver->player(id)->gen(team)))
        return;
    myserver->player(id)->team(team).poke(slot).num() = num;
}

void ScriptEngine::changePokeLevel(int id, int team, int slot, int level)
{
    if (!testPlayer("changePokeLevel", id) || !testRange("changePokeLevel", slot, 0, 5) || !testRange("changePokeLevel", level, 1, 100) || !testTeamCount("changePokeLevel", id, team))
        return;
    Player *p = myserver->player(id);
    p->team(team).poke(slot).level() = level;
    p->team(team).poke(slot).updateStats(p->gen(team));
}

void ScriptEngine::changePokeMove(int id, int team, int pslot, int mslot, int move)
{
    if (!testPlayer("changePokeLevel", id) || !testRange("changePokeLevel", pslot, 0, 5) || !testRange("changePokeLevel", mslot, 0, 3) || !testTeamCount("changePokeLevel", id, team))
        return;
    if (!MoveInfo::Exists(move, GenInfo::GenMax()))
        return;
    Player *p = myserver->player(id);
    p->team(team).poke(pslot).move(mslot).num() = move;
    p->team(team).poke(pslot).move(mslot).load(p->gen(team));
}

void ScriptEngine::changePokeGender(int id, int team, int pokeslot, int gender)
{
    if (!testPlayer("changePokeGender", id) || !testRange("changePokeGender", pokeslot, 0, 5) || !testRange("changePokeGender", gender, 0, 2) || !testTeamCount("changePokeGender", id, team))
        return;
    Player *p = myserver->player(id);
    p->team(team).poke(pokeslot).gender() = gender;
}

void ScriptEngine::changePokeName(int id, int team, int pokeslot, const QString &name)
{
    if (!testPlayer("changePokeName", id)|| !testTeamCount("changePokeName", id, team) || !testRange("changePokeName", pokeslot, 0, 5))
        return;
    Player *p = myserver->player(id);
    p->team(team).poke(pokeslot).nick() = name;
}

void ScriptEngine::changePokeHp(int id, int team, int slot, int hp)
{
    if (!testPlayer("changePokeHp", id)|| !testTeamCount("changePokeHp", id, team) || !testRange("changePokeHp", slot, 0, 5))
        return;

    PokeBattle &poke = myserver->player(id)->team(team).poke(slot);

    if (!testRange("changePokeHp", hp, 0, poke.totalLifePoints()))
        return;

    poke.lifePoints() = hp;

    if (hp > 0 && poke.status() == Pokemon::Koed) {
        poke.changeStatus(Pokemon::Fine);
    } else if (hp == 0) {
        poke.changeStatus(Pokemon::Koed);
    }
}

void ScriptEngine::changePokeStatus(int id, int team, int slot, int status)
{
    if (!testPlayer("changePokeStatus", id)|| !testTeamCount("changePokeStatus", id, team) || !testRange("changePokeStatus", slot, 0, 5))
        return;

    if (status != Pokemon::Koed && !testRange("changePokeStatus", status, Pokemon::Fine, Pokemon::Poisoned))
        return;

    PokeBattle &poke = myserver->player(id)->team(team).poke(slot);
    poke.changeStatus(status);
    poke.oriStatusCount() = poke.statusCount() = 0; //Clearing toxic & sleep turns, to use them introduce new script functions

    if (status == Pokemon::Koed) {
        poke.lifePoints() = 0;
    } else {
        poke.lifePoints() = std::max(int(poke.lifePoints()), 1);
    }
}

void ScriptEngine::changePokePP(int id, int team, int slot, int moveslot, int PP)
{
    if (!testPlayer("changePokePP", id)|| !testTeamCount("changePokePP", id, team) || !testRange("changePokePP", slot, 0, 5)
            || !testRange("changePokePP", moveslot, 0, 4))
        return;

    PokeBattle &poke = myserver->player(id)->team(team).poke(slot);

    if (!testRange("changePokePP", PP, 0, poke.move(moveslot).totalPP()))
        return;

    poke.move(moveslot).PP() = PP;
}

bool ScriptEngine::hasLegalTeamForTier(int id, int team, const QString &tier)
{
    if(!testPlayer("hasLegalTeamForTier", id) || !testTeamCount("hasLegalTeamForTier", id, team)) {
        return false;
    }
    if (!TierMachine::obj()->exists(tier))
        return false;
    return TierMachine::obj()->isValid(myserver->player(id)->team(team),tier);
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

int ScriptEngine::connections(const QString &ip)
{
    return AntiDos::obj()->connections(ip);
}

int ScriptEngine::numRegistered(const QString &ip)
{
    return SecurityManager::numRegistered(ip);
}

QScriptValue ScriptEngine::memoryDump()
{
    QString ret;

    ret += QString("Members\n\tCached in memory> %1\n\tCached as non-existing> %2\n").arg(SecurityManager::holder.cachedMembersCount()).arg(SecurityManager::holder.cachedNonExistingCount());
    ret += QString("Waiting Objects\n\tFree Objects> %1\n\tTotal Objects> %2\n").arg(WaitingObjects::freeObjects.count()).arg(WaitingObjects::objectCount);
    ret += QString("Battles\n\tActive> %1\n\tRated Battles History> %2\n").arg(myserver->battles->count()).arg(myserver->lastRatedIps.count());
    ret += AntiDos::obj()->dump();
    ret += QString("-------------------------\n-------------------------\n");

    foreach (QString tier, TierMachine::obj()->tierList().split('\n')) {
        const Tier &t = TierMachine::obj()->tier(tier);
        ret += QString("Tier %1\n\tCached in memory> %2\n\tCached as non-existing> %3\n").arg(tier).arg(t.holder.cachedMembersCount()).arg(t.holder.cachedNonExistingCount());
    }

    return ret;
}

QString ScriptEngine::profileDump()
{
    QString ret;

    QHash<QString, Profile>::iterator i;
    quint64 total = 0;
    for (i = profiles.begin(); i != profiles.end(); ++i) {
        Profile profile = i.value();
        int average = 0;

        // prevent divide by zero
        if (profile.totalDuration != 0 && profile.calls != 0) {
            average = profile.totalDuration / profile.calls;
        }

        ret += QString("%1: Called %2 times, took %3 ms in total (avg %4 ms per call)\n").arg(
                    i.key(),
                    QString::number(profile.calls),
                    QString::number(profile.totalDuration),
                    QString::number(average)
                    );

        total += profile.totalDuration;
    }
    return QString("time since last reset: %1ms, time taken by events: %2ms\n").arg(performanceTimer.elapsed()).arg(total) + ret;
}

void ScriptEngine::resetProfiling()
{
    resetPerfs = true;
}

QScriptValue ScriptEngine::dosChannel()
{
    return AntiDos::obj()->notificationsChannel;
}

void ScriptEngine::changeDosChannel(const QString &str)
{
    AntiDos::obj()->notificationsChannel = str;

    QSettings s("config", QSettings::IniFormat);
    s.setValue("AntiDOS/NotificationsChannel", str);
}

void ScriptEngine::clearDosData()
{
    AntiDos::obj()->clearData();
}

void ScriptEngine::reloadDosSettings()
{
    QSettings s("config", QSettings::IniFormat);
    AntiDos::obj()->init(s);
}

QScriptValue ScriptEngine::currentMod()
{
    return PokemonInfoConfig::currentMod();
}

QScriptValue ScriptEngine::currentModPath()
{
    return PokemonInfoConfig::currentModPath();
}

QScriptValue ScriptEngine::dataRepo()
{
    return PokemonInfoConfig::dataRepo();
}

int ScriptEngine::disconnectedPlayers()
{
    return myserver->mynames.size() - myserver->numberOfPlayersLoggedIn;
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
    if (!SecurityManager::exist(name)) {
        return false;
    }
    return SecurityManager::registered(name);
}

#ifndef _EXCLUDE_DEPRECATED

int ScriptEngine::callLater(const QString &s, int delay) {
    if (!callLater_w) {
        warn ("callLater(code, delay)", "Deprecated, use setTimer(code, milisecondsDelay, repeats) instead", true);
        callLater_w = true;
    }
    return setTimer(s, delay*1000, false);
}

int ScriptEngine::callQuickly(const QString &s, int delay) {
    if (!callQuickly_w) {
        warn ("callQuickly(code, delay)", "Deprecated, use setTimer(code, milisecondsDelay, repeats) instead", true);
        callQuickly_w = true;
    }
    return setTimer(s, delay, false);
}

int ScriptEngine::quickCall(const QScriptValue &func, int delay) {
    if (!quickCall_w) {
        warn("quickCall(code, delay)", "Deprecated, use setTimer(code, milisecondsDelay, repeats) instead.", true);
        quickCall_w = true;
    }
    return setTimer(func, delay, false);
}

int ScriptEngine::delayedCall(const QScriptValue &func, int delay)
{
    if (!delayedCall_w) {
        warn ("quickCall(code, delay)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
        delayedCall_w = true;
    }
    return setTimer(func, delay*1000, false);
}

int ScriptEngine::intervalCall(const QScriptValue &func, int delay)
{
    if (!intervalCall_w) {
        warn ("intervalCall(func, delay)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
        intervalCall_w = true;
    }
    return setTimer(func, delay*1000, true);
}

int ScriptEngine::intervalTimer(const QString &expr, int delay) {
    if (!intervalTimer_w) {
        warn ("intervalTimer(code, delay)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
        intervalTimer_w = true;
    }
    return setTimer(expr, delay*1000, true);
}

bool ScriptEngine::stopTimer(int timerId) {
    if (!stopTimer_w) {
        warn ("stopTimer(timerId)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
        stopTimer_w = true;
    }
    return unsetTimer(timerId);
}

#endif // #ifndef _EXCLUDE_DEPRECATED

int ScriptEngine::setTimer(const QScriptValue &code, int delay, bool repeats)
{
    if (delay <= 0) {
        return -1;
    }
    if ((!code.isFunction()) && (!code.isString())) {
        warn("setTimer(code, delay, repeats)", "code must be string or function.");
        return -1;
    }

    QTimer *t = new QTimer();

    timerEvents[t] = code;
    t->setSingleShot(!repeats);
    t->start(delay);
    connect(t, SIGNAL(timeout()), SLOT(timer()));

    return t->timerId();
}

void ScriptEngine::timer()
{
    QTimer *t = (QTimer*) sender();
    if (timerEvents[t].isFunction()) {
        timerEvents[t].call();
    } else if (timerEvents[t].isString()) {
        eval(timerEvents[t].toString());
    } else {
        warn("ScriptEngine::timer", "this is a bug, report it. code is not string or function");
        return;
    }

    if (t->isSingleShot()) {
        timerEvents.remove(t);
        t->deleteLater();
    }
}

bool ScriptEngine::unsetTimer(int timerId)
{
    QHashIterator <QTimer*, QScriptValue> it (timerEvents);
    while (it.hasNext()) {
        it.next();
        QTimer *timer = it.key();

        if (timer->timerId() == timerId) {
            timer->stop();
            timer->blockSignals(true);

            timerEvents.remove(timer);
            timer->deleteLater();
            return true; // Timer found.
        }
    }
    warn ("unsetTimer(timerId)", "no timer with that id");
    return false; // No timer found.
}

int ScriptEngine::unsetAllTimers()
{
    int i = 0;
    QHashIterator <QTimer*, QScriptValue> it (timerEvents);
    while (it.hasNext()) {
        it.next();
        QTimer *timer = it.key();

        timer->stop();
        timer->blockSignals(true);

        timerEvents.remove(timer);
        timer->deleteLater();
        i++;
    }

    return i;
}

void ScriptEngine::timer_step()
{
    this->stepEvent();
}

QScriptValue ScriptEngine::eval(const QString &script)
{
    return myengine.evaluate(script);
}

QScriptValue ScriptEngine::eval(const QString &script, const QString &fname )
{
    return myengine.evaluate(script, fname);
}


QScriptValue ScriptEngine::auth(int id)
{
    if (!exists(id)) {
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
        return SecurityManager::auth(name);
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

bool ScriptEngine::dbLoaded(const QString &name)
{
    return SecurityManager::holder.isInMemory(name);
}

bool ScriptEngine::dbExists(const QString &name)
{
    return SecurityManager::exist(name);
}

void ScriptEngine::dbClearCache()
{
    SecurityManager::holder.clearCache();
}

QScriptValue ScriptEngine::dbLastOn(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        return SecurityManager::member(name).date;
    }
}

QScriptValue ScriptEngine::dbExpire(const QString &name)
{
    if(!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        QDate tempDate;
        tempDate = QDate::fromString(SecurityManager::member(name).date, Qt::ISODate);
        return int(myserver->playerDeleteDays() - tempDate.daysTo(QDate::currentDate()));
    }
}

QScriptValue ScriptEngine::dbTempBanTime(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        return SecurityManager::member(name).ban_expire_time - QDateTime::currentDateTimeUtc().toTime_t();
    }
}

int ScriptEngine::dbExpiration()
{
    return myserver->playerDeleteDays();
}

QScriptValue ScriptEngine::battling(int id)
{
    if (!exists(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->battling();
    }
}

QScriptValue ScriptEngine::away(int id)
{
    if (!exists(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->away();
    }
}

void ScriptEngine::changeColor(int id, const QString &color)
{
    if (!testPlayer("changeColor", id)) {
        return;
    }

    QColor playerColor(color);
    if (!playerColor.isValid()) {
        return;
    }

    myserver->player(id)->color() = playerColor;
    myserver->sendPlayer(id);
}

QScriptValue ScriptEngine::getColor(int id)
{
    if (!exists(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->color().name();
    }
}

QScriptValue ScriptEngine::tier(int id, int team)
{
    if(!testPlayer("tier", id) || !testTeamCount("tier", id, team)) {
        return myengine.undefinedValue();
    }
    return myserver->player(id)->team(team).tier;
}

bool ScriptEngine::hasTier(int id, const QString &tier)
{
    if (!testPlayer("hasTier", id)) {
        return false;
    }

    return myserver->player(id)->hasTier(tier);
}

QScriptValue ScriptEngine::ranking(int id, int team)
{
    if(!testPlayer("ranking", id) || !testTeamCount("ranking", id, team)) {
        return myengine.undefinedValue();
    }
    Player *p = myserver->player(id);
    return ranking(p->name(), p->team(team).tier);
}

QScriptValue ScriptEngine::ratedBattles(int id, int team)
{
    if(!testPlayer("ratedBattles", id) || !testTeamCount("ratedBattles", id, team)) {
        return myengine.undefinedValue();
    }
    Player *p = myserver->player(id);
    return ratedBattles(p->name(), p->team(team).tier);
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
    if (!exists(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->rating(tier);
    }
}

QScriptValue ScriptEngine::ladderEnabled(int id)
{
    if (!exists(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->ladder();
    }
}

QScriptValue ScriptEngine::ip(int id)
{
    if (!exists(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->ip();
    }
}

QScriptValue ScriptEngine::proxyIp(int id)
{
    if (!exists(id)) {
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

QScriptValue ScriptEngine::gen(int id, int team)
{
    if (!testTeamCount("gen(id, team)", id, team)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->gen(team).num;
    }
}

QScriptValue ScriptEngine::subgen(int id, int team)
{
    if (!testTeamCount("subgen(id, team)", id, team)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->gen(team).subnum;
    }
}

QScriptValue ScriptEngine::teamCount(int id)
{
    if (!testPlayer("teamCount(id)", id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->teamCount();
    }
}

QScriptValue ScriptEngine::generation(int genNum, int subNum)
{
    if(testRange("generation(genNum, subNum)", genNum, GEN_MIN, GenInfo::GenMax()) && testRange("generation(genNum, subNum)", subNum, 0, GenInfo::NumberOfSubgens(genNum) - 1)) {
        return GenInfo::Version(Pokemon::gen(genNum, subNum));
    }
    return myengine.undefinedValue();
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
        int id = myserver->id(name);
        if (!loggedIn(id)) {
            return myengine.undefinedValue();
        } else {
            return id;
        }
    }
}

QScriptValue ScriptEngine::pokemon(int num)
{
    return PokemonInfo::Name(num);
}

QScriptValue ScriptEngine::pokeNum(const QString &name)
{
    Pokemon::uniqueId num = PokemonInfo::Number(name);
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
    if (num >= 0 && num < AbilityInfo::NumberOfAbilities(GenInfo::GenMax())) {
        return AbilityInfo::Name(num);
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::abilityNum(const QString &ability)
{
    return AbilityInfo::Number(convertToSerebiiName(ability));
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

QScriptValue ScriptEngine::teamPoke(int id, int team, int index)
{
    if(!testPlayer("teamPoke", id) || !testTeamCount("teamPoke", id, team)) {
        return myengine.undefinedValue();
    }
    if (!testRange("teamPoke", index, 0, 5)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).num().toPokeRef();
    }
}

QScriptValue ScriptEngine::teamPokeName(int id, int team, int index)
{
    if (!testPlayer("teamPokeName", id) || !testTeamCount("teamPokeName", id, team)) {
        return myengine.undefinedValue();
    }
    if (!testRange("teamPokeName", index, 0, 5)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).nick();
    }
}

QScriptValue ScriptEngine::teamPokeLevel(int id, int team, int index)
{
    if(!testPlayer("teamPokeLevel", id) || !testTeamCount("teamPokeLevel", id, team) || !testRange("teamPokeLevel", index, 0, 5)) {
        return myengine.undefinedValue();
    }

    return myserver->player(id)->team(team).poke(index).level();
}

QScriptValue ScriptEngine::teamPokeStat(int id, int team, int slot, int stat)
{
    if(!testPlayer("teamPokeStat", id) || !testTeamCount("teamPokeStat", id, team) || !testRange("teamPokeStat", slot, 0, 5)
            || !testRange("teamPokeStat", stat, 0, 5)) {
        return myengine.undefinedValue();
    }

    if (stat == Hp) {
        return myserver->player(id)->team(team).poke(slot).totalLifePoints();
    } else {
        return myserver->player(id)->team(team).poke(slot).normalStat(stat);
    }
}

QScriptValue ScriptEngine::teamPokeHp(int id, int team, int slot)
{
    if(!testPlayer("teamPokeHp", id) || !testTeamCount("teamPokeHp", id, team) || !testRange("teamPokeHp", slot, 0, 5)) {
        return myengine.undefinedValue();
    }

    return myserver->player(id)->team(team).poke(slot).lifePoints();
}

QScriptValue ScriptEngine::teamPokeStatus(int id, int team, int slot)
{
    if(!testPlayer("teamPokeStatus", id) || !testTeamCount("teamPokeStatus", id, team) || !testRange("teamPokeStatus", slot, 0, 5)) {
        return myengine.undefinedValue();
    }

    return myserver->player(id)->team(team).poke(slot).status();
}

QScriptValue ScriptEngine::teamPokePP(int id, int team, int slot, int moveslot)
{
    if(!testPlayer("teamPokePP", id) || !testTeamCount("teamPokePP", id, team) || !testRange("teamPokePP", slot, 0, 5)
            || !testRange("teamPokePP", moveslot, 0, 3)) {
        return myengine.undefinedValue();
    }

    return myserver->player(id)->team(team).poke(slot).move(moveslot).PP();
}

bool ScriptEngine::hasTeamPoke(int id, int team, int pokemonnum)
{
    if(!testPlayer("hasTeamItem", id) || !testTeamCount("hasTeamItem", id, team)) {
        TeamBattle &t = myserver->player(id)->team(team);
        for (int i = 0; i < 6; i++) {
            if (t.poke(i).num() == pokemonnum) {
                return true;
            }
        }
        return false;
    }
    return false;
}

QScriptValue ScriptEngine::indexOfTeamPoke(int id, int team, int pokenum)
{
    if(!testPlayer("indexOfTeamPoke", id) || !testTeamCount("indexOfTeamPoke", id, team)) {
        return myengine.undefinedValue();
    }
    TeamBattle &t = myserver->player(id)->team(team);
    for (int i = 0; i < 6; i++) {
        if (t.poke(i).num() == pokenum) {
            return i;
        }
    }
    return myengine.undefinedValue();
}

bool ScriptEngine::hasDreamWorldAbility(int id, int team, int index, int gen)
{
    if(testPlayer("hasDreamWorldAbility", id) && testTeamCount("hasDreamWorldAbility", id, team) && gen > 4) {
        if (index < 0 || index >= 6) {
            return false;
        } else {
            PokeBattle &p = myserver->player(id)->team(team).poke(index);

            AbilityGroup ag = PokemonInfo::Abilities(p.num(), gen);

            return p.ability() != ag.ab(0) && p.ability() != ag.ab(1);
        }
    }
    return false;
}

bool ScriptEngine::compatibleAsDreamWorldEvent(int id, int team, int index)
{
    if(testPlayer("compatibleAsDreamWorldEvent", id) && testTeamCount("compatibleAsDreamWorldEvent", id, team)) {
        if (index < 0 || index >= 6) {
            return false;
        } else {
            PokeBattle &p = myserver->player(id)->team(team).poke(index);

            return MoveSetChecker::isValid(p.num(),myserver->player(id)->team(team).gen,
                                           p.move(0).num(),p.move(1).num(),p.move(2).num(),p.move(3).num(),p.ability(),p.gender(), p.level(), true);
        }
        return false;
    }
    return false;
}

QScriptValue ScriptEngine::teamPokeMove(int id, int team, int pokeindex, int moveindex)
{
    if(!testPlayer("teamPokeMove", id) || !testTeamCount("teamPokeMove", id, team)) {
        return myengine.undefinedValue();
    }
    if (pokeindex < 0 || moveindex < 0 || pokeindex >= 6 || moveindex >= 4) {
        return myengine.undefinedValue();
    }
    return myserver->player(id)->team(team).poke(pokeindex).move(moveindex).num();
}

bool ScriptEngine::hasTeamPokeMove(int id, int team, int pokeindex, int movenum)
{
    if(testPlayer("hasTeamPokeMove", id) || !testTeamCount("hasTeamPokeMove", id, team)) {
        if (pokeindex < 0 || pokeindex >= 6) {
            return false;
        }
        PokeBattle &poke = myserver->player(id)->team(team).poke(pokeindex);

        for (int i = 0; i < 4; i++) {
            if (poke.move(i).num() == movenum) {
                return true;
            }
        }
        return false;
    }
    return false;
}

QScriptValue ScriptEngine::indexOfTeamPokeMove(int id, int team, int pokeindex, int movenum)
{
    if(!testPlayer("indexOfTeamPokeMove", id) || !testTeamCount("indexOfTeamPokeMove", id, team)) {
        return myengine.undefinedValue();
    }
    if (pokeindex < 0 || pokeindex >= 6) {
        return myengine.undefinedValue();
    }
    PokeBattle &poke = myserver->player(id)->team(team).poke(pokeindex);

    for (int i = 0; i < 4; i++) {
        if (poke.move(i).num() == movenum) {
            return i;
        }
    }
    return myengine.undefinedValue();
}

bool ScriptEngine::hasTeamMove(int id, int team, int movenum)
{
    if(testPlayer("hasTeamMove", id) || testTeamCount("hasTeamMove", id, team)) {
        for (int i = 0; i < 6; i++) {
            if (hasTeamPokeMove(id,team,i,movenum))
                return true;
        }
        return false;
    }
    return false;
}

QScriptValue ScriptEngine::teamPokeItem(int id, int team, int index)
{
    if(!testPlayer("teamPokeItem", id) || !testTeamCount("teamPokeItem", id, team)) {
        return myengine.undefinedValue();
    }
    if (index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).item();
    }
}

bool ScriptEngine::hasTeamItem(int id, int team, int itemnum)
{
    if(testPlayer("hasTeamItem", id) && testTeamCount("hasTeamItem", id, team)) {
        TeamBattle &t = myserver->player(id)->team(team);
        for (int i = 0; i < 6; i++) {
            if (t.poke(i).item() == itemnum) {
                return true;
            }
        }
        return false;
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

QScriptValue ScriptEngine::battlingIds()
{
    QList<int> keys = myserver->myplayers.keys();

    QScriptValue ret = myengine.newArray();
    int x = 0;
    for (int i = 0; i < keys.size(); i++) {
        if (myserver->player(keys[i])->battling()) {
            ret.setProperty(x++, keys[i]);
        }
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
        return myserver->channel(id).name();
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
        QScriptValue ret = myengine.newArray(c.count());

        foreach(int id, c.players) {
            ret.setProperty(i, id);
            i += 1;
        }

        return ret;
    }
}

QScriptValue ScriptEngine::teamPokeHappiness(int id, int team, int index)
{
    if(!testPlayer("teamPokeHappiness", id) || !testTeamCount("teamPokeHappiness", id, team)) {
        return myengine.undefinedValue();
    }
    if (index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).happiness();
    }
}

QScriptValue ScriptEngine::teamPokeNature(int id, int team, int index)
{
    if(!testPlayer("teamPokeNature", id) || !testTeamCount("teamPokeNature", id, team)) {
        return myengine.undefinedValue();
    }
    if (index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).nature();
    }
}

QScriptValue ScriptEngine::teamPokeEV(int id, int team, int index, int stat)
{
    if (!testPlayer("teamPokeEV", id) || !testTeamCount("teamPokeEV", id, team)) {
        return myengine.undefinedValue();
    }
    if (index < 0 || index >= 6 || stat < 0 || stat >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).evs()[stat];
    }
}

QScriptValue ScriptEngine::teamPokeDV(int id, int team, int index, int stat)
{
    if (!testPlayer("teamPokeDV", id) || !testTeamCount("teamPokeDV", id, team)) {
        return myengine.undefinedValue();
    }
    if (index < 0 || index >= 6 || stat < 0 || stat >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team(team).poke(index).dvs()[stat];
    }
}

void ScriptEngine::changeTeamPokeDV(int id, int team, int slot, int stat, int newValue)
{
    if(testPlayer("changeTeamPokeDV", id) && testTeamCount("changeTeamPokeDV", id, team) && (slot >=0 && slot <=5 && stat >=0 && stat <= 5 && newValue >= 0 && newValue <= 31)) {
        myserver->player(id)->team(team).poke(slot).dvs()[stat] = newValue;
    }
}

void ScriptEngine::changeTeamPokeEV(int id, int team, int slot, int stat, int newValue)
{
    if(testPlayer("changeTeamPokeEV", id) && testTeamCount("changeTeamPokeEV", id, team) && (slot >=0 && slot <6 && stat >=0 && stat <6 && newValue >= 0 && newValue <= 255)) {
        /* int total = 0;
        for (int i=0; i<6; i++) {
            if (i == stat)
                total += newValue;
            else
                total += myserver->player(id)->team(team).poke(slot).evs()[i];
        }
        if (total <= 510) */
            myserver->player(id)->team(team).poke(slot).evs()[stat] = newValue;
    }
}

int ScriptEngine::rand(int min, int max)
{
    if (min == max)
        return min;
    return ::floor(myengine.globalObject().property("Math").property("random").call().toNumber() * (max - min) + min);
}

int ScriptEngine::numPlayers()
{
    return myserver->numberOfPlayersLoggedIn;
}

int ScriptEngine::playersInMemory()
{
    return myserver->myplayers.size();
}

bool ScriptEngine::exists(int id)
{
    return myserver->playerExist(id);
}

bool ScriptEngine::loggedIn(int id)
{
    return myserver->playerLoggedIn(id);
}

void ScriptEngine::printLine(const QString &s)
{
    myserver->forcePrint(s);
}

void ScriptEngine::stopEvent()
{
    if (stopevents.size() == 0) {
        warn("sys.stopEvent()",  "Unstoppable event.", true);
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
#ifdef QT5
    int line = QInputDialog::getInt(myedit, tr("Line Number"), tr("To what line do you want to go?"), 1, 1, myedit->document()->lineCount());
#else
    int line = QInputDialog::getInteger(myedit, tr("Line Number"), tr("To what line do you want to go?"), 1, 1, myedit->document()->lineCount());
#endif
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
    if((gen >= GEN_MIN) && (gen <= GenInfo::GenMax())) {
        result = PokemonInfo::Type1(Pokemon::uniqueId(id), gen);
    }else{
        warn("pokeType1", "generation is not supported.");
    }
    return result;
}

int ScriptEngine::pokeType2(int id, int gen)
{
    int result = Pokemon::Curse;
    if((gen >= GEN_MIN) && (gen <= GenInfo::GenMax())) {
        result = PokemonInfo::Type2(Pokemon::uniqueId(id), gen);
    }else{
        warn("pokeType2", "generation is not supported.");
    }
    return result;
}

QScriptValue ScriptEngine::baseStats(int poke, int stat, int gen)
{
    if (!PokemonInfo::Exists(Pokemon::uniqueId(poke))) {
        warn("baseStats(poke, stat, gen)", QString("Pokemon %1 doesn't exist.").arg(QString::number(poke)));
        return -1;
    }

    if (!testRange("baseStats(poke, stat, gen)", stat, 0, 6)) {
        return -1;
    }

    if((gen < GEN_MIN) || (gen > GenInfo::GenMax())) {
        warn("baseStats(poke, stat, gen)", QString("Gen %1 unsupported.").arg(QString::number(gen)));
        return -1;
    }

    return PokemonInfo::BaseStats(poke,gen).baseStat(stat);
}

QScriptValue ScriptEngine::pokeBaseStats(int id, int gen)
{
    QScriptValue ret;
    if((gen >= GEN_MIN) && (gen <= GenInfo::GenMax())) {
        if(PokemonInfo::Exists(Pokemon::uniqueId(id))) {
            ret = myengine.newArray(6);
            PokeBaseStats bs = PokemonInfo::BaseStats(Pokemon::uniqueId(id), gen);

            for (int i = 0; i < 6; i++) {
                ret.setProperty(i, bs.baseStat(i));
            }
        }else{
            warn("pokeBaseStats", "Pokemon doesn't exist.");
        }
    } else {
        warn("pokeBaseStats", "generation is not supported.");
    }
    return ret;
}

QScriptValue ScriptEngine::pokeGenders(int poke)
{
    QScriptValue ret = myengine.newObject();
    int gender = PokemonInfo::Gender(poke);

    if (gender == Pokemon::MaleAvail) {
        ret.setProperty("male", 100);
    } else if (gender == Pokemon::FemaleAvail) {
        ret.setProperty("female", 100);
    } else if (gender == Pokemon::NeutralAvail) {
        ret.setProperty("neutral", 100);
    } else {
        int rate = PokemonInfo::GenderRate(poke);

        ret.setProperty("female", 100.0*(8-rate)/8);
        ret.setProperty("male", 100.0*rate/8);
    }

    return ret;
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
    if(loggedIn(myserver->id(name))) {
        myserver->silentKick(myserver->id(name));
    }
}

void ScriptEngine::tempBan(QString name, int time)
{
    if(time < 0) {
        return;
    }
    SecurityManager::ban(name, time);
    if(loggedIn(myserver->id(name))) {
        myserver->silentKick(myserver->id(name));
    }
}

void ScriptEngine::unban(QString name)
{
    SecurityManager::unban(name);
}

bool ScriptEngine::banned(const QString &ip) {
    return SecurityManager::bannedIP(ip);
}

void ScriptEngine::battleSetup(int src, int dest, int battleId)
{
    makeEvent("battleSetup", src, dest, battleId);
}

#if 0
void ScriptEngine::prepareWeather(int battleId, int weatherId)
{
    if((weatherId >= 0) && (weatherId <= 4)) {
        BattleBase * battle = myserver->getBattle(battleId);
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

void ScriptEngine::prepareItems(int battleId, int playerSlot, QScriptValue items)
{
    if(testRange("prepareItems", playerSlot, 0, 1)) {
        BattleBase * battle = myserver->getBattle(battleId);
        if (battle) {
            QHash<quint16,quint16> itemh;

            QScriptValueIterator it(items);
            while (it.hasNext()) {
            
