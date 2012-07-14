#include <QScriptEngine>
#include "../Utilities/otherwidgets.h"
#include "../BattleManager/advancedbattledata.h"
#include "../BattleManager/battledataaccessor.h"
#include "../BattleManager/pokemoninfoaccessor.h"
#include "../BattleManager/proxydatacontainer.h"
#include "../BattleManager/battleinput.h"
#include "../Teambuilder/basebattlewindowinterface.h"
#include "battlescripting.h"

#define objectConverter(className) \
    Q_DECLARE_METATYPE(className*) \
    typedef className* T##className; \
    \
    static QScriptValue to##className(QScriptEngine *e, const T##className&r) {\
    return e->newQObject(r); \
    }\
    \
    static void from##className(const QScriptValue &s, T##className&r) {\
    r = dynamic_cast<T##className>(s.toQObject());\
    }


objectConverter(ProxyDataContainer)
objectConverter(TeamProxy)
objectConverter(PokeProxy)
objectConverter(AuxPokeDataProxy)
objectConverter(FieldProxy)

#define registerObject(className) qScriptRegisterMetaType<className*>(engine, &to##className, &from##className)

BattleScripting::BattleScripting(QScriptEngine *engine, BaseBattleWindowInterface *interface)
{
    myinterface = interface;
    myengine = engine;
    myengine->setParent(this);
    setParent(interface);

    registerObject(ProxyDataContainer);
    registerObject(TeamProxy);
    registerObject(PokeProxy);
    registerObject(AuxPokeDataProxy);
    registerObject(FieldProxy);

    advbattledata_proxy *data = interface->getBattleData();
    ProxyDataContainer *pdata = data->exposedData();
    int me = data->isPlayer(1) ? 1 : 0;
    int opp = !me;

    QScriptValue battle = myengine->newQObject(interface);
    myengine->globalObject().setProperty("battle", battle);
    battle.setProperty("data", myengine->newQObject(dynamic_cast<QObject*>(pdata)));
    battle.setProperty("me", QScriptValue(me));
    battle.setProperty("opp", QScriptValue(opp));
    battle.setProperty("id", interface->battleId());

    /* Removes clientscripting' print function and add ours. Client scripting's print function
      can still be accessed with sys.print() */
    QScriptValue printfun = myengine->newFunction(nativePrint);
    printfun.setData(myengine->newQObject(this));
    myengine->globalObject().setProperty("print", printfun);

    interface->addOutput(this);
}

QScriptValue BattleScripting::nativePrint(QScriptContext *context, QScriptEngine *engine)
{
    QString result;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(" ");
        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    BattleScripting *obj = qobject_cast<BattleScripting*>(calleeData.toQObject());
    obj->printLine(result);

    return engine->undefinedValue();
}

void BattleScripting::onSendOut(int spot, int previndex, ShallowBattlePoke *pokemon, bool silent)
{
    PokeProxy *p = new PokeProxy(pokemon);
    makeEvent("onSendOut", spot, previndex, qScriptValueFromValue<PokeProxy*>(myengine, p), silent);
    delete p;
}

void BattleScripting::evaluate(const QScriptValue &expr)
{
    if (expr.isError()) {
        printLine(QString("Script Error line %1: %2").arg(myengine->uncaughtExceptionLineNumber()).arg(expr.toString()));
    }
}

void BattleScripting::printLine(const QString &string)
{
    myinterface->getLogWidget()->insertPlainText(string+"\n");
//    auto p = std::shared_ptr<QString>(new QString(string));
//    myinterface->getInput()->entryPoint(BattleEnum::PrintHtml, &p);
}
