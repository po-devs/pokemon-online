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

    myengine->globalObject().setProperty("battle", myengine->newQObject(dynamic_cast<QObject*>(pdata)));
    myengine->globalObject().property("battle").setProperty("me", QScriptValue(me));
    myengine->globalObject().property("battle").setProperty("opp", QScriptValue(opp));

    interface->addOutput(this);
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
