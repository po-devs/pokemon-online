#include "scriptengine.h"
#include "scriptutils.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Teambuilder/clientinterface.h"

ScriptEngine::ScriptEngine(ClientInterface *c) {
    c->registerMetaTypes(&myengine);

    myclient = c;
    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("sys", sys);
    myengine.globalObject().setProperty("client",myengine.newQObject(dynamic_cast<QObject*>(c)));
    QScriptValue printfun = myengine.newFunction(nativePrint);
    printfun.setData(sys);
    myengine.globalObject().setProperty("print", printfun);

#ifndef PO_SCRIPT_SAFE_ONLY
    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
#endif

    changeScript(ScriptUtils::loadScripts());

    QTimer *step_timer = new QTimer(this);
    step_timer->setSingleShot(false);
    step_timer->start(1000);
    connect(step_timer, SIGNAL(timeout()), SLOT(timer_step()));
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
    ret.insert("beforePMReceived(int,QString)", (Hook)(&ScriptEngine::beforePMReceived));
    ret.insert("afterPMReceived(int,QString)", (Hook)(&ScriptEngine::afterPMReceived));
    ret.insert("playerLogin(int)", (Hook)(&ScriptEngine::playerLogIn));
    ret.insert("playerLogout(int)", (Hook)(&ScriptEngine::playerLogOut));

    return ret;
}

void ScriptEngine::changeScript(const QString &script, const bool triggerStartUp)
{
    myscript = myengine.evaluate(script);
    myengine.globalObject().setProperty("script", myscript);

    if (myscript.isError()) {
        printLine("Fatal Script Error line " + QString::number(myengine.uncaughtExceptionLineNumber()) + ": " + myscript.toString());
    } else {
        //printLine("Script Check: OK");
        if(triggerStartUp) {
            clientStartUp();
        }
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

void ScriptEngine::warn(const QString &function, const QString &message)
{
    printLine(QString("Script Warning in %1: %2").arg(function, message));
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

int ScriptEngine::beforePMReceived(int id, const QString &message)
{
    return makeSEvent("beforePMReceived", id, message);
}

int ScriptEngine::afterPMReceived(int id, const QString &message)
{
    makeEvent("afterPMReceived", id, message);
    return true;
}

int ScriptEngine::playerLogIn(int id)
{
    makeEvent("playerLogIn", id);
    return true;
}

int ScriptEngine::playerLogOut(int id)
{
    makeEvent("playerLogOut", id);
    return true;
}

void ScriptEngine::stepEvent()
{
    evaluate(myscript.property("step").call(myscript, QScriptValueList()));
}

void ScriptEngine::clientShutDown()
{
    evaluate(myscript.property("clientShutDown").call(myscript, QScriptValueList()));
}

void ScriptEngine::evaluate(const QScriptValue &expr)
{
    if (expr.isError()) {
        printLine(QString("Script Error line %1: %2").arg(myengine.uncaughtExceptionLineNumber()).arg(expr.toString()));
    }
}

void ScriptEngine::clearChat()
{
    //emit clearTheChat();
}

bool ScriptEngine::validColor(const QString &color)
{
    QColor colorName = QColor(color);
    return colorName.isValid() && colorName.lightness() <= 140 && colorName.green() <= 180;
}

void ScriptEngine::callLater(const QString &expr, int delay)
{
    if (delay <= 0) {
        return;
    }

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
    QTimer *t = (QTimer*) sender();
    eval(timerEvents[t]);

    timerEvents.remove(t);
    t->deleteLater();
}

void ScriptEngine::timer_step()
{
    this->stepEvent();
}

void ScriptEngine::quickCall(const QScriptValue &func, int delay)
{
    if (delay <= 0) return;
    if (func.isFunction()) {
        QTimer *t = new QTimer(this);
        timerEventsFunc[t] = func;
        t->setSingleShot(true);
        t->start(delay);
        connect(t, SIGNAL(timeout()), SLOT(timerFunc()));
    }
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
        printLine("Script Warning: calling sys.stopEvent() in an unstoppable event.");
    } else {
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

#ifndef PO_SCRIPT_SAFE_ONLY
void ScriptEngine::saveSetting(const QString &key, const QVariant &val)
{
    QSettings s;
    s.setValue(key, val);
}

QScriptValue ScriptEngine::getSetting(const QString &key)
{
    QSettings s;
    return s.value(key).toString();
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

/**
 * Function will perform a GET-Request client side
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
    if (!callback.isString() && !callback.isFunction()) {
        printLine("Script Warning in sys.webCall(urlstring, callback, params_array): callback is not a string or a function.");
        return;
    }

    QNetworkRequest request;
    QByteArray postData;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online clientscript");

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
 * Function will perform a GET-Request client side, synchronously
 * @param urlstring web-url
 * @author Remco cd Zon and Toni Fadjukoff
 */
QScriptValue ScriptEngine::synchronousWebCall(const QString &urlstring) {
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
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    QByteArray postData;

    request.setUrl(QUrl(urlstring));
    request.setRawHeader("User-Agent", "Pokemon-Online clientscript");

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
#endif

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
