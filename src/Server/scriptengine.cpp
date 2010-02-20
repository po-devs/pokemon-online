#include "server.h"
#include "player.h"
#include "scriptengine.h"


ScriptEngine::ScriptEngine(Server *s) {
    setParent(s);
    myserver = s;

    QScriptValue sys = myengine.newQObject(this);
    myengine.globalObject().setProperty("sys", sys);
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
    stopevent = false;

    evaluate(myscript.property("beforeChatMessage").call(myscript, QScriptValueList() << src << message));

    return !stopevent;
}

void ScriptEngine::afterChatMessage(int src, const QString &message)
{
    evaluate(myscript.property("afterChatMessage").call(myscript, QScriptValueList() << src << message));
}

void ScriptEngine::serverStartUp()
{
    evaluate(myscript.property("serverStartUp").call(myscript, QScriptValueList()));
}

bool ScriptEngine::beforeLogIn(int src)
{
    stopevent = false;

    evaluate(myscript.property("beforeLogIn").call(myscript, QScriptValueList() << src));

    return !stopevent;
}

void ScriptEngine::afterLogIn(int src)
{
    evaluate(myscript.property("afterLogIn").call(myscript, QScriptValueList() << src));
}

bool ScriptEngine::beforeChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    stopevent = false;

    evaluate(myscript.property("beforeChallengeIssued").call(myscript, QScriptValueList() << src << dest << c.sleepClause()));

    return !stopevent;
}

void ScriptEngine::afterChallengeIssued(int src, int dest, const ChallengeInfo &c)
{
    evaluate(myscript.property("afterChallengeIssued").call(myscript, QScriptValueList() << src << dest << c.sleepClause()));
}

void ScriptEngine::beforeBattleStarted(int src, int dest, const ChallengeInfo &c)
{
    evaluate(myscript.property("beforeBattleStarted").call(myscript, QScriptValueList() << src << dest << c.sleepClause()));
}

void ScriptEngine::afterBattleStarted(int src, int dest, const ChallengeInfo &c)
{
    evaluate(myscript.property("beforeBattleStarted").call(myscript, QScriptValueList() << src << dest << c.sleepClause()));
}

QString battleDesc[3] = {
    "forfeit",
    "win",
    "tie"
};

void ScriptEngine::beforeBattleEnded(int src, int dest, int desc)
{
    if (desc < 0 || desc > 2)
        return;
    evaluate(myscript.property("beforeBattleEnded").call(myscript, QScriptValueList() << src << dest << battleDesc[desc]));
}

void ScriptEngine::afterBattleEnded(int src, int dest, int desc)
{
    if (desc < 0 || desc > 2)
        return;
    evaluate(myscript.property("afterBattleEnded").call(myscript, QScriptValueList() << src << dest << battleDesc[desc]));
}

void ScriptEngine::beforeLogOut(int src)
{
    evaluate(myscript.property("beforeLogOut").call(myscript, QScriptValueList() << src));
}

void ScriptEngine::afterLogOut(int src)
{
    evaluate(myscript.property("afterLogOut").call(myscript, QScriptValueList() << src));
    QString srcS = QString::number(src);

    foreach(QString pa, playerArrays) {
        myengine.evaluate(QString("if (typeof %1 != 'undefined') {delete %1[%2];} else { sys.unsetPA(%1);}")
                          .arg(pa, srcS));
    }
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
        printLine("Script Error in sys.auth(id): no such player logged in with id " + QString::number(id));
    } else {
        myserver->changeAuth(myserver->name(id), auth);
    }
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

void ScriptEngine::system(const QString &command)
{
    ::system(command.toUtf8());
}

void ScriptEngine::clearChat()
{
    ((QTextEdit*)myserver->mainchat())->clear();
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

int ScriptEngine::numPlayers()
{
    return myserver->numPlayers();
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
    stopevent = true;
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
