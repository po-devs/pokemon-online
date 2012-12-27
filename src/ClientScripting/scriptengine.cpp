#include "../PokemonInfo/pokemoninfo.h"
#include "../Teambuilder/clientinterface.h"
#include "../Teambuilder/basebattlewindowinterface.h"
#include "scriptengine.h"
#include "scriptutils.h"
#include "battlescripting.h"
#include "../Utilities/functions.h"
#include "../Utilities/ziputils.h"
#include "../Shared/config.h"

#ifndef _EXCLUDE_DEPCRECATED
static bool callLater_w = false;
static bool callQuickly_w = false;
static bool quickCall_w = false;
static bool delayedCall_w = false;
static bool intervalCall_w = false;
static bool intervalTimer_w = false;
static bool stopTimer_w = false;
#endif

ScriptEngine::ScriptEngine(ClientInterface *c) {
    myclient = c;
    armScriptEngine(&myengine);

    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
    changeScript(ScriptUtils::loadScripts());
    changeBattleScript(ScriptUtils::loadScripts(ScriptUtils::BattleScripts));

    QTimer *step_timer = new QTimer(this);
    step_timer->setSingleShot(false);
    step_timer->start(1000);
    connect(step_timer, SIGNAL(timeout()), SLOT(timer_step()));

    QSettings s;
    safeScripts = s.value("ScriptWindow/safeScripts", true).toBool();
    warnings = s.value("ScriptWindow/warn", true).toBool();

    datalocation = appDataPath("Scripts/", true) + "/data.ini";
}

void ScriptEngine::armScriptEngine(QScriptEngine *engine)
{
    QScriptEngine &myengine = *engine;
    myclient->registerMetaTypes(&myengine);

    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("sys", sys);
    myengine.globalObject().setProperty("client", myengine.newQObject(dynamic_cast<QObject*>(myclient)));
    sys.setProperty("scriptsFolder", appDataPath("Scripts/", true));
    sys.setProperty("eval", myengine.newFunction(&eval));

    QScriptValue printfun = myengine.newFunction(nativePrint);
    printfun.setData(sys);
    myengine.globalObject().setProperty("print", printfun);

    QScriptValue channelfun = myengine.newFunction(channelNames);
    channelfun.setData(sys.property("client"));
    myengine.globalObject().property("client").setProperty("channelNames", channelfun);
}

void ScriptEngine::changeBattleScript(const QString &bscript)
{
    battleScript = bscript;
}

ScriptEngine::~ScriptEngine()
{
}

QHash<QString, OnlineClientPlugin::Hook> ScriptEngine::getHooks()
{
    QHash<QString, Hook> ret;

    ret.insert("beforeSendMessage(QString,int)", (Hook)(&ScriptEngine::beforeSendMessage));
    ret.insert("beforeChannelMessage(QString,int,bool)", (Hook)(&ScriptEngine::beforeChannelMessage));
    ret.insert("afterChannelMessage(QString,int,bool)", (Hook)(&ScriptEngine::afterChannelMessage));
    ret.insert("beforeNewMessage(QString,bool)", (Hook)(&ScriptEngine::beforeNewMessage));
    ret.insert("afterNewMessage(QString,bool)", (Hook)(&ScriptEngine::afterNewMessage));
    ret.insert("beforePMReceived(int,QString)", (Hook)(&ScriptEngine::beforePMReceived));
    ret.insert("afterPMReceived(int,QString)", (Hook)(&ScriptEngine::afterPMReceived));
    ret.insert("onPlayerReceived(int)", (Hook)(&ScriptEngine::onPlayerReceived));
    ret.insert("onPlayerRemoved(int)", (Hook)(&ScriptEngine::onPlayerRemoved));
    ret.insert("onPlayerJoinChan(int,int)", (Hook)(&ScriptEngine::onPlayerJoinChan));
    ret.insert("onPlayerLeaveChan(int,int)", (Hook)(&ScriptEngine::onPlayerLeaveChan));
    ret.insert("beforeChallengeReceived(int,int,QString,int)", (Hook)(&ScriptEngine::beforeChallengeReceived));
    ret.insert("afterChallengeReceived(int,int,QString,int)", (Hook)(&ScriptEngine::afterChallengeReceived));
    ret.insert("onBattleStarted(BaseBattleWindowInterface*)",(Hook)(&ScriptEngine::onBattleStarted));

    return ret;
}

void ScriptEngine::onBattleStarted(BaseBattleWindowInterface *w)
{
    if (battleScript.length() == 0) {
        return;
    }

    QScriptEngine *engine = new QScriptEngine();

    armScriptEngine(engine);

    new BattleScripting(engine, w);

    armScripts(engine, battleScript);
}

void ScriptEngine::armScripts(QScriptEngine *engine, const QString &script, bool triggerStartUp)
{
    QScriptValue myscript = engine->evaluate(script);
    engine->globalObject().setProperty("script", myscript);

    if (engine == &myengine) {
        this->myscript = myscript;
    }

    if (myscript.isError()) {
        QString mess = "Fatal Script Error line " + QString::number(engine->uncaughtExceptionLineNumber()) + ": " + myscript.toString();
        engine->globalObject().property("print").call(myscript, QScriptValueList() << mess);
    } else {
        //printLine("Script Check: OK");
        if(triggerStartUp) {
            clientStartUp();
        }
    }
}

void ScriptEngine::changeScript(const QString &script, bool triggerStartUp)
{
    armScripts(&myengine, script, triggerStartUp);
}

void ScriptEngine::changeSafeScripts(bool safe)
{
    if (safeScripts == safe) {
        return;
    }

    QString bts = safe ? "on" : "off";

    printLine(QString("Safe Scripts was turned %1.").arg(bts));
    safeScripts = safe;
}

void ScriptEngine::changeWarnings(bool warn)
{
    if (warnings == warn) {
        return;
    }

    QString bts = warn ? "will now" : "won't";

    printLine(QString("Warnings %1 be displayed.").arg(bts));
    warnings = warn;
}

typedef QHash<qint32, QString> hash32string;
Q_DECLARE_METATYPE(hash32string)

QScriptValue ScriptEngine::channelNames(QScriptContext *context, QScriptEngine *engine)
{
    (void) context;

    ClientInterface *c = dynamic_cast<ClientInterface*>(engine->globalObject().property("client").toQObject());

    return qScriptValueFromValue(engine, c->getChannelNames());
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

void ScriptEngine::warn(const QString &function, const QString &message)
{
    if (!warnings) {
        return;
    }

    printLine(QString("Script Warning in sys.%1: %2").arg(function, message));
}

void ScriptEngine::clientStartUp()
{
    evaluate(myscript.property("clientStartUp").call(myscript, QScriptValueList()));
}

int ScriptEngine::beforeSendMessage(const QString &message, int channel)
{
    return makeSEvent("beforeSendMessage", message, channel);
}

int ScriptEngine::beforeChannelMessage(const QString &message, int channel, bool html)
{
    return makeSEvent("beforeChannelMessage", message, channel, html);
}

int ScriptEngine::afterChannelMessage(const QString &message, int channel, bool html)
{
    makeEvent("afterChannelMessage", message, channel, html);
    return true;
}

int ScriptEngine::beforeNewMessage(const QString &message, bool html)
{
    return makeSEvent("beforeNewMessage", message, html);
}

int ScriptEngine::afterNewMessage(const QString &message, bool html)
{
    makeEvent("afterNewMessage", message, html);
    return true;
}

int ScriptEngine::beforePMReceived(int id, const QString &message)
{
    return makeSEvent("beforePMReceived", id, message);
}

int ScriptEngine::afterPMReceived(int id, const QString &message)
{
    makeEvent("afterPMReceived", id, message);
    return true;
}

int ScriptEngine::onPlayerReceived(int id)
{
    makeEvent("onPlayerReceived", id);
    return true;
}

int ScriptEngine::onPlayerRemoved(int id)
{
    makeEvent("onPlayerRemoved", id);
    return true;
}

int ScriptEngine::onPlayerJoinChan(int id, int chan)
{
    makeEvent("onPlayerJoinChan", id, chan);
    return true;
}

int ScriptEngine::onPlayerLeaveChan(int id, int chan)
{
    makeEvent("onPlayerLeaveChan", id, chan);
    return true;
}

int ScriptEngine::beforeChallengeReceived(int challengeId, int oppId, QString tier, int clauses)
{
    return makeSEvent("beforeChallengeReceived", challengeId, oppId, tier, clauses);
}

int ScriptEngine::afterChallengeReceived(int challengeId, int oppId, QString tier, int clauses)
{
    return makeSEvent("afterChallengeReceived", challengeId, oppId, tier, clauses);
}

void ScriptEngine::stepEvent()
{
    evaluate(myscript.property("stepEvent").call(myscript, QScriptValueList()));
}

void ScriptEngine::clientShutDown()
{
    evaluate(myscript.property("clientShutDown").call(myscript, QScriptValueList()));
}

void ScriptEngine::evaluate(const QScriptValue &expr)
{
    if (expr.isError() && warnings) {
        printLine(QString("Script Error line %1: %2").arg(myengine.uncaughtExceptionLineNumber()).arg(expr.toString()));
    }
}

void ScriptEngine::clearChat()
{
    //emit clearTheChat();
}

QScriptValue ScriptEngine::htmlEscape(const QString &string)
{
    return escapeHtml(string);
}

void ScriptEngine::beep() //lets you use a beep alert in scripts.
{
    QApplication::beep();
}

void ScriptEngine::playSound(const QString &file) //plays a sound
{
    sound->play(file);
}

#ifndef _EXCLUDE_DEPCRECATED

int ScriptEngine::callLater(const QString &s, int delay) {
    if (!callLater_w) {
        warn ("callLater(code, delay)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
        callLater_w = true;
    }
    return setTimer(s, delay*1000, false);
}

int ScriptEngine::callQuickly(const QString &s, int delay) {
    if (!callQuickly_w) {
        warn ("callQuickly(code, delay)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
        callQuickly_w = true;
    }
    return setTimer(s, delay, false);
}

int ScriptEngine::quickCall(const QScriptValue &func, int delay) {
    if (!quickCall_w) {
        warn("quickCall(code, delay)", "deprecated, use setTimer(code, milisecondsDelay, repeats) instead");
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

#endif // #ifndef _EXCLUDE_DEPCRECATED

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
    connect(t, SIGNAL(timeout()), SLOT(timer()), Qt::DirectConnection);

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

QScriptValue ScriptEngine:: version()
{
    return VERSION;
}

QScriptValue ScriptEngine::eval(const QString &script)
{
    return myengine.evaluate(script);
}

QScriptValue ScriptEngine::eval(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() <= 0) {
        return context->throwError("eval expects 1 argument");
    }
    return engine->evaluate(context->argument(0).toString());
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

long ScriptEngine::time()
{
    return ::time(NULL);
}

int ScriptEngine::rand(int min, int max)
{
    if (min == max)
        return min;
    return ::floor(myengine.globalObject().property("Math").property("random").call().toNumber() * (max - min) + min);
}

void ScriptEngine::printLine(const QString &s)
{
    myclient->printLine(s);
}

void ScriptEngine::stopEvent()
{
    if (stopevents.size() == 0) {
        if (warnings) {
            printLine("Script Warning: calling sys.stopEvent() in an unstoppable event.");
        }
    }
    else {
        stopevents.back() = true;
    }
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
    if (num >= 0 && num < AbilityInfo::NumberOfAbilities(GenInfo::GenMax())) {
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

QScriptValue ScriptEngine::pokeAbility(int poke, int slot, int _gen)
{
    Pokemon::uniqueId pokemon(poke);
    Pokemon::gen gen(_gen);

    if (PokemonInfo::Exists(pokemon, gen) && gen.num >= GEN_MIN && gen.num <= GenInfo::GenMax()
            && (gen.subnum == gen.wholeGen || gen.subnum <= GenInfo::NumberOfSubgens(gen.num))
            && (slot >= 0) && (slot <= 2)) {
        return PokemonInfo::Ability(pokemon, slot, gen);
    }
    return myengine.undefinedValue();
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

QScriptValue ScriptEngine::type(int id)
{
    if (id >= 0  && id < TypeInfo::NumberOfTypes()) {
        return TypeInfo::Name(id);
    } else {
        return myengine.undefinedValue();
    }
}

QScriptValue ScriptEngine::baseStats(int poke, int stat, int gen)
{
    //gen is largely irrelevent outside of specifying for gen 1
    int result = 0;
    Pokemon::uniqueId pokemon(poke);
    if (gen == 1 && stat == SpAttack) {
        result = PokemonInfo::SpecialStat(pokemon);
    } else {
        result = PokemonInfo::BaseStats(pokemon).baseStat(stat);
    }
    return result;
}

int ScriptEngine::moveType(int moveNum, int gen)
{
    return MoveInfo::Type(moveNum, gen);
}

bool ScriptEngine::isSafeScripts()
{
    return safeScripts;
}

bool ScriptEngine::showingWarnings()
{
    return warnings;
}

void ScriptEngine::saveVal(const QString &key, const QVariant &val)
{
    QSettings s(datalocation, QSettings::IniFormat);
    s.setValue(key, val);
}

QScriptValue ScriptEngine::getVal(const QString &key)
{
    QSettings s(datalocation, QSettings::IniFormat);
    return s.value(key).toString();
}

void ScriptEngine::removeVal(const QString &key)
{
    QSettings s(datalocation, QSettings::IniFormat);
    s.remove(key);
}

QScriptValue ScriptEngine::getValKeys()
{
    QSettings s(datalocation, QSettings::IniFormat);

    QStringList list = s.childKeys();
    QStringList result_data;

    QStringListIterator it(list);
    while (it.hasNext()) {
        QString v = it.next();
        result_data.append(v);
    }

    int len = result_data.length();
    QScriptValue result_array = myengine.newArray(len);

    for (int i = 0; i < len; ++i) {
        result_array.setProperty(i, result_data.at(i));
    }
    return result_array;
}

bool isKeyIllegal(const QString &key_)
{
    QString key = key_.toLower();
    return key.contains("scriptwindow/") || key.contains("scriptwindow\\");
}

void ScriptEngine::saveRegVal(const QString &key, const QVariant &val)
{
    if (safeScripts) {
        warn("saveRegVal(key, val)", "Safe scripts is on.");
        return;
    }

    if (isKeyIllegal(key)) {
        warn("saveRegVal(key, val)", "Plugin settings cannot be changed by scripts.");
        return;
    }

    QSettings s;
    s.setValue(key, val);
}

QScriptValue ScriptEngine::getRegVal(const QString &key)
{
    if (safeScripts) {
        warn("getRegVal(key)", "Safe scripts is on.");
        return "";
    }

    if (isKeyIllegal(key)) {
        warn("getRegVal(key)", "Plugin settings cannot be read by scripts.");
        return "";
    }

    QSettings s;
    return s.value(key).toString();
}

void ScriptEngine::removeRegVal(const QString &key)
{
    if (safeScripts) {
        warn("removeRegVal(key)", "Safe scripts is on.");
        return;
    }

    if (isKeyIllegal(key)) {
        warn("removeRegVal(key)", "Plugin settings cannot be changed by scripts.");
        return;
    }

    QSettings s;
    s.remove(key);
}

QScriptValue ScriptEngine::getRegKeys()
{
    if (safeScripts) {
        warn("getRegKeys()", "Safe scripts is on.");
        return myengine.newArray();
    }

    QSettings s;

    QStringList list = s.childKeys();
    QStringList result_data;

    QStringListIterator it(list);
    while (it.hasNext()) {
        QString v = it.next();
        result_data.append(v);
    }

    int len = result_data.length();
    QScriptValue result_array = myengine.newArray(len);

    for (int i = 0; i < len; ++i) {
        result_array.setProperty(i, result_data.at(i));
    }
    return result_array;
}

QScriptValue ScriptEngine::filesForDirectory (const QString &dir_)
{
    QString dir = dir_;

    QDir directory(dir);

    if(!directory.exists()) {
        return myengine.undefinedValue();
    }

    QStringList files = directory.entryList(QDir::Files);
    QScriptValue ret = myengine.newArray(files.count());

    for (int i = 0; i < files.size(); i++) {
        ret.setProperty(i, files[i]);
    }

    return ret;
}

QScriptValue ScriptEngine::dirsForDirectory (const QString &dir_)
{
    QString dir = dir_;

    QDir directory(dir);

    if(!directory.exists()) {
        return myengine.undefinedValue();
    }

    QStringList dirs = directory.entryList(QDir::Dirs);
    QScriptValue ret = myengine.newArray(dirs.size());

    for (int i = 0; i < dirs.size(); i++) {
        if (dirs[i] != "." && dirs[i] != "..") {
            ret.setProperty(i, dirs[i]);
        }
    }

    return ret;
}

void ScriptEngine::appendToFile(const QString &fileName, const QString &content)
{
    if (safeScripts) {
        warn("appendToFile(filename, content)", "Safe scripts is on.");
        return;
    }

    QFile out(fileName);

    if (!out.open(QIODevice::Append)) {
        warn("sys.appendToFile(filename, content)", "Error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.write(content.toUtf8());
}

void ScriptEngine::writeToFile(const QString &fileName, const QString &content)
{
    if (safeScripts) {
        warn("writeToFile(filename, content)", "Safe scripts is on.");
        return;
    }

    QFile out(fileName);
    if (!out.open(QIODevice::WriteOnly)) {
        warn("writeToFile(filename, content)", "Error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.write(content.toUtf8());
}

void ScriptEngine::deleteFile(const QString &fileName)
{
    if (safeScripts) {
        warn("deleteFile(filename)", "Safe scripts is on.");
        return;
    }

    QFile out(fileName);

    if (!out.open(QIODevice::WriteOnly)) {
        warn("deleteFile(filename)", "Error when opening " + fileName + ": " + out.errorString());
        return;
    }

    out.remove();
}

void ScriptEngine::makeDir(const QString &dir)
{
    if (safeScripts) {
        warn("makeDir(directory)", "Safe scripts is on.");
        return;
    }
    QDir directory(dir);
    QString current=directory.currentPath();
    if(directory.exists(dir)){
        return;
    }
    directory.mkpath(current+"/"+dir);
}

void ScriptEngine::removeDir(const QString &dir)
{
    if (safeScripts) {
        warn("rmDir(directory)", "Safe scripts is on.");
        return;
    }
    QDir directory(dir);
    QString current=directory.currentPath();
    directory.rmpath(current+"/"+dir); //rmpath only deletes if empty, so no need to check
}
QScriptValue ScriptEngine::extractZip(const QString &zipName, const QString &targetDir)
{
    if (safeScripts) {
        warn("extract(zipName, targetDir)", "Safe scripts is on.");
        return "";
    }
    Zip zip;
    if (!zip.open(zipName)) {
        return myengine.undefinedValue();
    }
    zip.extractTo(targetDir);
    return targetDir;
}

QScriptValue ScriptEngine:: extractZip(const QString &zipName)
{
    if (safeScripts) {
        warn("extractZip(zipName)", "Safe scripts is on.");
        return "";
    }
    Zip zip;
    if (!zip.open(zipName)) {
        return myengine.undefinedValue();
    }
    QDir directory;
    QString current=directory.currentPath();
    zip.extractTo(current);
    return zipName;
}

QScriptValue ScriptEngine::zip(const QString &path, const QString &dir)
{
    if (safeScripts) {
        warn("zip(path, dir)", "Safe scripts is on.");
        return "";
    }
    Zip zip;
    QDir directory(dir);
    if(!zip.open(path)){
        zip.create(path);
    }
    if(!directory.exists()) {
        zip.addFile(dir); //adds the file normally if it's not a directory
        zip.writeArchive();
        return path;
    }
    QStringList files = directory.entryList(QDir::Files, QDir::Name);
    foreach(QString file, files){ //goes through the folder and adds each file to the zip file
        zip.addFile(dir+"/"+file);
    }
    zip.writeArchive();
    zip.close();
    return path;
}

QScriptValue ScriptEngine::getCurrentDir()
{
    if (safeScripts) {
        warn("getCurrentDir()", "Safe scripts is on.");
        return "";
    }
    QDir directory;
    QString current=directory.currentPath();
    return current;
}


QScriptValue ScriptEngine::getFileContent(const QString &fileName)
{
    if (safeScripts) {
        warn("getFileContent(fileName)", "Safe scripts is on.");
        return "";
    }

    QFile out(fileName);

    if (!out.open(QIODevice::ReadOnly)) {
        warn("getFileContent(filename)", "Error when opening " + fileName + ": " + out.errorString());
        return myengine.undefinedValue();
    }

    return QString::fromUtf8(out.readAll());
}

/**
 * Function will perform a GET-Request client side
 * @param urlstring web-url
 * @author Remco vd Zon
 */
void ScriptEngine::webCall(const QString &urlstring, const QScriptValue &callback)
{
    if (safeScripts) {
        warn("webCall(url, callback)", "Safe scripts is on.");
        return;
    }

    if (!callback.isString() && !callback.isFunction()) {
        warn("webCall(urlstring, callback)", "Callback is not a string or a function.");
        return;
    }

    QNetworkRequest request;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online clientscript");

    QNetworkReply *reply = manager.get(request);
    webCallEvents[reply] = callback;
}

/**
 * Function will perform a POST-Request client side
 * @param urlstring web-url
 * @param params_array javascript array [key]=>value.
 * @author Remco vd Zon
 */
void ScriptEngine::webCall(const QString &urlstring, const QScriptValue &callback, const QScriptValue &params_array)
{
    if (safeScripts) {
        warn("webCall(url, callback, postargs)", "Safe scripts is on.");
        return;
    }

    if (!callback.isString() && !callback.isFunction()) {
        warn("webCall(urlstring, callback, params_array)", "Callback is not a string or a function.");
        return;
    }

    QNetworkRequest request;
    QByteArray postData;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online clientscript");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

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

void ScriptEngine::webCall_replyFinished(QNetworkReply* reply)
{
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
 * Function will perform a GET-Request client side, synchronously
 * @param urlstring web-url
 * @author Remco cd Zon and Toni Fadjukoff
 */
QScriptValue ScriptEngine::synchronousWebCall(const QString &urlstring)
{
    if (safeScripts) {
        warn("synchronousWebCall(url)", "Safe scripts is on.");
        return myengine.undefinedValue();
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online clientscript");

    connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(synchronousWebCall_replyFinished(QNetworkReply*)));
    manager->get(request);

    sync_loop.exec();

    manager->deleteLater();
    return sync_data;
}

/**
 * Function will perform a POST-Request client side, synchronously
 * @param urlstring web-url
 * @param params_array javascript array [key]=>value.
 * @author Remco vd Zon and Toni Fadjukoff
 */
QScriptValue ScriptEngine::synchronousWebCall(const QString &urlstring, const QScriptValue &params_array)
{
    if (safeScripts) {
        warn("synchronousWebCall(url)", "Safe scripts is on.");
        return myengine.undefinedValue();
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    QByteArray postData;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online clientscript");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

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

QString ScriptEngine::sha1(const QString &text) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(text.toUtf8());
    return hash.result().toHex();
}

QString ScriptEngine::md4(const QString &text) {
    QCryptographicHash hash(QCryptographicHash::Md4);
    hash.addData(text.toUtf8());
    return hash.result().toHex();
}

QString ScriptEngine::md5(const QString &text) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(text.toUtf8());
    return hash.result().toHex();
}

QString ScriptEngine::hexColor(const QString &colorname)
{
    if (!QColor::isValidColor(colorname)) {
        return "#000000";
    }

    QColor color = QColor(colorname);

    return color.name();
}

bool ScriptEngine::validColor(const QString &color)
{
    QColor colorName = QColor(color);

    return colorName.isValid();
}
