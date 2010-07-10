#include "server.h"
#include "player.h"
#include "security.h"
#include "antidos.h"
#include "waitingobject.h"
#include "tiermachine.h"
#include "tier.h"
#include "scriptengine.h"
#include "../PokemonInfo/pokemoninfo.h"

ScriptEngine::ScriptEngine(Server *s) {
    setParent(s);
    myserver = s;

    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("sys", sys);
    myengine.globalObject().setProperty("script", myscript);
    QScriptValue printfun = myengine.newFunction(nativePrint);
    printfun.setData(sys);
    myengine.globalObject().setProperty("print", printfun);


    QFile f("scripts.js");
    f.open(QIODevice::ReadOnly);

    changeScript(QString::fromUtf8(f.readAll()));
}

void ScriptEngine::changeScript(const QString &script)
{
    myscript = myengine.evaluate(script);

    if (myscript.isError()) {
        printLine("Script Error line " + QString::number(myengine.uncaughtExceptionLineNumber()) + ": " + myscript.toString());
    } else {
        printLine("Script Check: OK");
    }
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

bool ScriptEngine::beforeChatMessage(int src, const QString &message)
{
    if (!myscript.property("beforeChatMessage", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforeChatMessage").call(myscript, QScriptValueList() << src << message));

    return !endStopEvent();
}

void ScriptEngine::afterChatMessage(int src, const QString &message)
{
    if (!myscript.property("afterChatMessage", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterChatMessage").call(myscript, QScriptValueList() << src << message));
}


bool ScriptEngine::beforeNewMessage(const QString &message)
{
    if (!myscript.property("beforeNewMessage", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforeNewMessage").call(myscript, QScriptValueList() << message));

    return !endStopEvent();
}

void ScriptEngine::afterNewMessage(const QString &message)
{
    if (!myscript.property("afterNewMessage", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterNewMessage").call(myscript, QScriptValueList() << message));
}

void ScriptEngine::serverStartUp()
{
    evaluate(myscript.property("serverStartUp").call(myscript, QScriptValueList()));
}

void ScriptEngine::serverShutDown()
{
    evaluate(myscript.property("serverShutDown").call(myscript, QScriptValueList()));
}

bool ScriptEngine::beforeLogIn(int src)
{
    if (!myscript.property("beforeLogIn", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforeLogIn").call(myscript, QScriptValueList() << src));

    return !endStopEvent();
}

void ScriptEngine::afterLogIn(int src)
{
    if (!myscript.property("afterLogIn", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterLogIn").call(myscript, QScriptValueList() << src));
}

void ScriptEngine::beforeChangeTeam(int src)
{
    if (!myscript.property("beforeChangeTeam", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("beforeChangeTeam").call(myscript, QScriptValueList() << src));
}

void ScriptEngine::afterChangeTeam(int src)
{
    if (!myscript.property("afterChangeTeam", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterChangeTeam").call(myscript, QScriptValueList() << src));
}

bool ScriptEngine::beforeChangeTier(int src, const QString &oldTier, const QString &newTier)
{
    if (!myscript.property("beforeChangeTier", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforeChangeTier").call(myscript, QScriptValueList() << src << oldTier << newTier));

    return !endStopEvent();
}

void ScriptEngine::afterChangeTier(int src, const QString &oldTier, const QString &newTier)
{
    if (!myscript.property("afterChangeTier", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterChangeTier").call(myscript, QScriptValueList() << src << oldTier << newTier));
}

bool ScriptEngine::beforeChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("beforeChallengeIssued", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    QString clauses;

    for(int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses.append('0' + ((c.clauses >> i) & 0x01));
    }

    evaluate(myscript.property("beforeChallengeIssued").call(myscript, QScriptValueList() << src << dest << clauses << c.rated << c.mode));

    return !endStopEvent();
}

void ScriptEngine::afterChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("afterChallengeIssued", QScriptValue::ResolveLocal).isValid())
        return;

    QString clauses;

    for(int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses.append('0' + ((c.clauses >> i) & 0x01));
    }

    evaluate(myscript.property("afterChallengeIssued").call(myscript, QScriptValueList() << src << dest << clauses << c.rated << c.mode));
}

bool ScriptEngine::beforeBattleMatchup(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("beforeBattleMatchup", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    QString clauses;

    for(int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses.append('0' + ((c.clauses >> i) & 0x01));
    }

    evaluate(myscript.property("beforeBattleMatchup").call(myscript, QScriptValueList() << src << dest << clauses << c.rated << c.mode));

    return !endStopEvent();
}

void ScriptEngine::afterBattleMatchup(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("afterBattleMatchup", QScriptValue::ResolveLocal).isValid())
        return;

    QString clauses;

    for(int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses.append('0' + ((c.clauses >> i) & 0x01));
    }

    evaluate(myscript.property("afterBattleMatchup").call(myscript, QScriptValueList() << src << dest << clauses << c.rated << c.mode));
}


void ScriptEngine::beforeBattleStarted(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("beforeBattleStarted", QScriptValue::ResolveLocal).isValid())
        return;

    QString clauses;

    for(int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses.append('0' + ((c.clauses >> i) & 0x01));
    }

    evaluate(myscript.property("beforeBattleStarted").call(myscript, QScriptValueList() << src << dest << clauses << c.rated << c.mode));
}

void ScriptEngine::afterBattleStarted(int src, int dest, const ChallengeInfo &c)
{
    if (!myscript.property("afterBattleStarted", QScriptValue::ResolveLocal).isValid())
        return;

    QString clauses;

    for(int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses.append('0' + ((c.clauses >> i) & 0x01));
    }

    evaluate(myscript.property("afterBattleStarted").call(myscript, QScriptValueList() << src << dest << clauses << c.rated << c.mode));
}

QString battleDesc[3] = {
    "forfeit",
    "win",
    "tie"
};

void ScriptEngine::beforeBattleEnded(int src, int dest, int desc)
{
    if (!myscript.property("beforeBattleEnded", QScriptValue::ResolveLocal).isValid())
        return;
    if (desc < 0 || desc > 2)
        return;
    evaluate(myscript.property("beforeBattleEnded").call(myscript, QScriptValueList() << src << dest << battleDesc[desc]));
}

void ScriptEngine::afterBattleEnded(int src, int dest, int desc)
{
    if (!myscript.property("afterBattleEnded", QScriptValue::ResolveLocal).isValid())
        return;
    if (desc < 0 || desc > 2)
        return;
    evaluate(myscript.property("afterBattleEnded").call(myscript, QScriptValueList() << src << dest << battleDesc[desc]));
}

void ScriptEngine::beforeLogOut(int src)
{
    if (!myscript.property("beforeLogOut", QScriptValue::ResolveLocal).isValid())
        return;
    evaluate(myscript.property("beforeLogOut").call(myscript, QScriptValueList() << src));
}

void ScriptEngine::afterLogOut(int src)
{
    if (!myscript.property("afterLogOut", QScriptValue::ResolveLocal).isValid())
        return;
    evaluate(myscript.property("afterLogOut").call(myscript, QScriptValueList() << src));
    QString srcS = QString::number(src);

    foreach(QString pa, playerArrays) {
        myengine.evaluate(QString("if (typeof %1 != 'undefined') {delete %1[%2];} else { sys.unsetPA(%1);}")
                          .arg(pa, srcS));
    }
}

bool ScriptEngine::beforePlayerKick(int src, int dest)
{
    if (!myscript.property("beforePlayerKick", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforePlayerKick").call(myscript, QScriptValueList() << src << dest));

    return !endStopEvent();
}

void ScriptEngine::afterPlayerKick(int src, int dest)
{
    if (!myscript.property("afterPlayerKick", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterPlayerKick").call(myscript, QScriptValueList() << src << dest));
}

bool ScriptEngine::beforePlayerBan(int src, int dest)
{
    if (!myscript.property("beforePlayerBan", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforePlayerBan").call(myscript, QScriptValueList() << src << dest));

    return !endStopEvent();
}

void ScriptEngine::afterPlayerBan(int src, int dest)
{
    if (!myscript.property("afterPlayerBan", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterPlayerBan").call(myscript, QScriptValueList() << src << dest));
}

bool ScriptEngine::beforePlayerAway(int src, bool away)
{
    if (!myscript.property("beforePlayerAway", QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    evaluate(myscript.property("beforePlayerAway").call(myscript, QScriptValueList() << src << away));

    return !endStopEvent();
}

void ScriptEngine::afterPlayerAway(int src, bool away)
{
    if (!myscript.property("afterPlayerAway", QScriptValue::ResolveLocal).isValid())
        return;

    evaluate(myscript.property("afterPlayerAway").call(myscript, QScriptValueList() << src << away));
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

void ScriptEngine::sendMessage(int id, const QString &mess)
{
    if (!myserver->playerExist(id)) {
        printLine("Script Error in sys.sendMessage(id, mess): no such player id as " + QString::number(id));
    } else {
        myserver->sendMessage(id, mess);
    }
}

void ScriptEngine::kick(int id)
{
    if (!myserver->playerExist(id)) {
        printLine("Script Error in sys.kick(id): no such player id as " + QString::number(id));
    } else {
        myserver->silentKick(id);
    }
}

void ScriptEngine::changeAuth(int id, int auth)
{
    if (!myserver->playerLoggedIn(id)) {
        printLine("Script Error in sys.changeAuth(id, auth): no such player logged in with id " + QString::number(id));
    } else {
        myserver->changeAuth(myserver->name(id), auth);
    }
}

void ScriptEngine::changeAway(int id, bool away)
{
    if (!myserver->playerLoggedIn(id)) {
        printLine("Script Error in sys.changeAway(id, auth): no such player logged in with id " + QString::number(id));
    } else {
        myserver->player(id)->executeAwayChange(away);;
    }
}

void ScriptEngine::changeRating(const QString& name, const QString& tier, int newRating)
{
    if (!TierMachine::obj()->exists(tier))
        printLine("Script Error in sys.changeRating(name, tier, rating): no such tier as " + tier);
    else
        TierMachine::obj()->changeRating(name, tier, newRating);
}

void ScriptEngine::changeTier(int id, const QString &tier)
{
    if (!TierMachine::obj()->exists(tier))
        printLine("Script Error in sys.changeTier(id, tier): no such tier as " + tier);
    else if (!myserver->playerLoggedIn(id)) {
        printLine("Script Error in sys.changeTier(id, tier): no such player logged in with id " + QString::number(id));
    } else {
        myserver->player(id)->executeTierChange(tier);
    }
}



void ScriptEngine::changePokeItem(int id, int slot, int item)
{
    if (!myserver->player(id)->isLoggedIn())
        return;
    if (slot < 0 || slot > 5)
        return;
    if (!ItemInfo::Exist(item))
        return;
    myserver->player(id)->team().poke(slot).item() = item;
}

void ScriptEngine::changePokeNum(int id, int slot, int num)
{
    if (!myserver->player(id)->isLoggedIn())
        return;
    if (slot < 0 || slot > 5)
        return;
    if (!PokemonInfo::Exist(num))
        return;
    myserver->player(id)->team().poke(slot).num() = num;
}

void ScriptEngine::changePokeLevel(int id, int slot, int level)
{
    if (!myserver->player(id)->isLoggedIn())
        return;
    if (slot < 0 || slot > 5)
        return;
    if (level < 1 || level > 100)
        return;
    myserver->player(id)->team().poke(slot).level() = level;
    myserver->player(id)->team().poke(slot).updateStats();
}

void ScriptEngine::changePokeMove(int id, int pslot, int mslot, int move)
{
    if (!myserver->player(id)->isLoggedIn())
        return;
    if (pslot < 0 || pslot > 5)
        return;
    if (mslot < 0 || mslot > 4)
        return;
    if (!MoveInfo::Exist(move))
        return;
    myserver->player(id)->team().poke(pslot).move(mslot).num() = move;
    myserver->player(id)->team().poke(pslot).move(mslot).load();
}

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
    QSettings s(file);
    s.setValue("Script_"+key, val);
}

QScriptValue ScriptEngine::getVal(const QString &file, const QString &key)
{
    QSettings s(file);
    return s.value("Script_"+key).toString();
}

void ScriptEngine::removeVal(const QString &file, const QString &key)
{
    QSettings s(file);
    s.remove("Script_"+key);
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
    return SecurityManager::membersForIp(ip).join("\n");
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

int ScriptEngine::system(const QString &command)
{
    return ::system(command.toUtf8());
}

void ScriptEngine::appendToFile(const QString &fileName, const QString &content)
{
    QFile out(fileName);

    if (!out.open(QIODevice::Append)) {
        printLine("Script Error in sys.appendToFile(filename, content): error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.write(content.toUtf8());
}

void ScriptEngine::writeToFile(const QString &fileName, const QString &content)
{
    QFile out(fileName);

    if (!out.open(QIODevice::WriteOnly)) {
        printLine("Script Error in sys.writeToFile(filename, content): error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.write(content.toUtf8());
}

void ScriptEngine::clearChat()
{
    //((QTextEdit*)myserver->mainchat())->clear();
}

/**
 * Function will perform a GET-Request server side
 * @param urlstring web-url
 * @author Remco vd Zon
 */
void ScriptEngine::webCall(const QString &urlstring, const QString &expr)
{
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	QNetworkRequest request;
	
	request.setUrl(QUrl(urlstring));
	request.setRawHeader("User-Agent", "Pokemon-Online serverscript");
	
	connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
	QNetworkReply *reply = manager->get(request);
	webCallEvents[reply] = expr;
}

/**
 * Function will perform a POST-Request server side
 * @param urlstring web-url
 * @param params_array javascript array [key]=>value.
 * @author Remco vd Zon
 */
void ScriptEngine::webCall(const QString &urlstring, const QString &expr, const QScriptValue &params_array)
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

	connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
	QNetworkReply *reply = manager->post(request, postData);
	webCallEvents[reply] = expr;
}

void ScriptEngine::webCall_replyFinished(QNetworkReply* reply){
	//escape reply before sending it to the javascript evaluator
	QString x = reply->readAll();
	x = x.replace(QString("'"), QString("\\'"));
	x = x.replace(QString("\n"), QString("\\n"));

	//put reply in a var "resp", can be used in expr
	// i.e. expr = 'print("The resp was: "+resp);'
	eval( "var resp = '"+x+"';"+webCallEvents[reply] );
	webCallEvents.remove( reply );
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
	return sync_data;
}

void ScriptEngine::synchronousWebCall_replyFinished(QNetworkReply* reply) {
        sync_data = reply->readAll();
        sync_data = sync_data.replace(QString("'"), QString("\\'"));
        sync_data = sync_data.replace(QString("\n"), QString("\\n"));
        sync_loop.exit();
}

void ScriptEngine::callLater(const QString &expr, int delay)
{
    if (delay <= 0) {
        return;
    }

    QTimer *t = new QTimer(this);

    timerEvents[t] = expr;
    t->setSingleShot(true);
    t->start(delay*1000);
    connect(t, SIGNAL(timeout()), SLOT(timer()));
}

void ScriptEngine::timer()
{
    QTimer *t = (QTimer*) sender();

    eval(timerEvents[t]);

    timerEvents.remove(t);
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

QScriptValue ScriptEngine::dbIp(const QString &name)
{
    if (!SecurityManager::exist(name)) {
        return myengine.undefinedValue();
    } else {
        return QString(SecurityManager::member(name).ip);
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

QScriptValue ScriptEngine::ranking(const QString &name, const QString &tier)
{
    if (!TierMachine::obj()->existsPlayer(tier, name)) {
        return myengine.undefinedValue();
    }
    return TierMachine::obj()->ranking(name, tier);
}

QScriptValue ScriptEngine::totalPlayersByTier(const QString &tier)
{
    return TierMachine::obj()->count(tier);
}

QScriptValue ScriptEngine::ladderRating(int id)
{
    if (!myserver->playerLoggedIn(id)) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->rating();
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
    if (num < 0 || num >= PokemonInfo::NumberOfPokemons()) {
        return myengine.undefinedValue();
    } else {
        return PokemonInfo::Name(num);
    }
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
            if (copy[i] == '-')
                up = true;
            copy[i] = copy[i].toLower();
        }
    }
    int num = PokemonInfo::Number(copy);
    if (num == 0) {
        return myengine.undefinedValue();
    } else {
        return num;
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
    if (ItemInfo::Exist(num)) {
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

QScriptValue ScriptEngine::teamPoke(int id, int index)
{
    if (!loggedIn(id) || index < 0 || index >= 6) {
        return myengine.undefinedValue();
    } else {
        return myserver->player(id)->team().poke(index).num();
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
    if (!loggedIn(id)) {
        printLine("Script Error in sys.hasTeamPoke(id, pokenum): no such player logged in with id " + QString::number(id));
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
        printLine("Script Error in sys.indexOfTeamPoke(id, pokenum): no such player logged in with id " + QString::number(id));
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
        printLine("Script Error in sys.hasTeamMove(id, pokenum): no such player logged in with id " + QString::number(id));
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
        printLine("Script Error in sys.hasTeamPoke(id, pokenum): no such player logged in with id " + QString::number(id));
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
    return  TierMachine::obj()->tierNames().join("\n");
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

QScriptValue ScriptEngine::getFileContent(const QString &fileName)
{
    QFile out(fileName);

    if (!out.open(QIODevice::ReadOnly)) {
        printLine("Script Error in sys.getFileContent(filename): error when opening " + fileName + ": " + out.errorString());
        return myengine.undefinedValue();
    }

    return QString::fromUtf8(out.readAll());
}

int ScriptEngine::rand(int min, int max)
{
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

void ScriptEngine::setPA(const QString &name)
{
    playerArrays.insert(name);
    myengine.evaluate(QString("%1 = [];").arg(name));
}

void ScriptEngine::unsetPA(const QString &name)
{
    playerArrays.remove(name);
    myengine.evaluate("delete " + name);
}

void ScriptEngine::printLine(const QString &s)
{
    myserver->printLine(s);
}

void ScriptEngine::stopEvent()
{
    if (stopevents.size() == 0) {
        myserver->printLine("Script Warning: calling sys.stopEvent() in an unstoppable event.");
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

ScriptWindow::ScriptWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Scripts"));

    QGridLayout *l = new QGridLayout(this);
    myedit = new QTextEdit();
    l->addWidget(myedit, 0, 0, 1, 4);

    QPushButton *ok = new QPushButton(tr("&Ok"));
    QPushButton *cancel = new QPushButton(tr("&Cancel"));

    l->addWidget(cancel,1,2);
    l->addWidget(ok,1,3);

    connect(ok, SIGNAL(clicked()), SLOT(okPressed()));
    connect(cancel, SIGNAL(clicked()), SLOT(deleteLater()));

    QFile f("scripts.js");
    f.open(QIODevice::ReadOnly);

    myedit->setText(QString::fromUtf8(f.readAll()));
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
