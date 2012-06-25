#include "scriptengine.h"
#include "scriptutils.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Teambuilder/clientinterface.h"

ScriptEngine::ScriptEngine(ClientInterface *c) {
    myclient = c;
    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("sys", sys);
    QScriptValue printfun = myengine.newFunction(nativePrint);
    printfun.setData(sys);
    myengine.globalObject().setProperty("print", printfun);

    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(webCall_replyFinished(QNetworkReply*)));
    changeScript(ScriptUtils::loadScripts());

    QTimer *step_timer = new QTimer(this);
    step_timer->setSingleShot(false);
    step_timer->start(1000);
    connect(step_timer, SIGNAL(timeout()), SLOT(timer_step()));
}

ScriptEngine::~ScriptEngine()
{
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

QString ScriptEngine::sha1(const QString &text) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(text.toUtf8());
    return hash.result().toHex();
}
