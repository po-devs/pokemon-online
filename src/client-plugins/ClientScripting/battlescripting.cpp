#include <QScriptEngine>
#include <Utilities/qscrolldowntextbrowser.h>
#include <BattleManager/advancedbattledata.h>
#include <BattleManager/battledataaccessor.h>
#include <BattleManager/pokemoninfoaccessor.h>
#include <BattleManager/proxydatacontainer.h>
#include <BattleManager/battleinput.h>
#include "../Teambuilder/basebattlewindowinterface.h"
#include "battlescripting.h"

Q_DECLARE_METATYPE(BattleChoice)

static QStringList bchoices =  QStringList() << "cancel" << "attack" << "switch" << "rearrange" << "shiftcenter" << "tie" << "item";

static QScriptValue toBattleChoice(QScriptEngine *e, const BattleChoice& info) {
    QScriptValue v = e->newObject();
    v.setProperty("type", bchoices.count() > info.type ? bchoices[info.type] : "unknown");
    v.setProperty("slot", int(info.slot()));
    if (info.attackingChoice()) {
        v.setProperty("attackSlot", int(info.attackSlot()));
        v.setProperty("target", int(info.target()));
        v.setProperty("mega", info.mega());
    } else if (info.switchChoice()) {
        v.setProperty("pokeSlot", int(info.pokeSlot()));
    } else if (info.rearrangeChoice()) {
        QScriptValue v2 = e->newArray();
        for (int i = 0; i < 6; i++) {
            v2.setProperty(i, int(info.choice.rearrange.pokeIndexes[i]));
        }
        v.setProperty("neworder", v2);
    } else if (info.itemChoice()) {
        v.setProperty("item", info.item());
        v.setProperty("target", info.itemTarget());
        v.setProperty("attack", info.itemAttack());
    }
    return v;
}
static void fromBattleChoice(const QScriptValue &v, BattleChoice& info) {

    info.type = std::max(bchoices.indexOf(v.property("type").toString()), 0);
    info.playerSlot = v.property("slot").toInt32();

    if (info.type == AttackType) {
        info.choice.attack.attackSlot = v.property("attackSlot").toInt32();
        if (v.property("target").isValid()) {
            info.choice.attack.attackTarget = v.property("target").toInt32();
        } else {
            info.choice.attack.attackTarget = !info.playerSlot;
        }
        info.choice.attack.mega = v.property("mega").toBool();
    } else if (info.type == SwitchType) {
        info.choice.switching.pokeSlot = v.property("pokeSlot").toInt32();
    } else if (info.type == RearrangeType) {
        if (v.property("neworder").isArray() && v.property("neworder").property("length").toInt32() >= 6) {
            for (int i = 0; i < 6; i++) {
                info.choice.rearrange.pokeIndexes[i] = v.property("neworder").property(i).toInt32();
            }
        } else {
            for (int i = 0; i < 6; i++) {
                info.choice.rearrange.pokeIndexes[i] = i;
            }
        }
    } else if (info.type == ItemType) {
        info.choice.item.item = v.property("item").toInt32();
        info.choice.item.target = v.property("target").toInt32();
        info.choice.item.attack = v.property("attack").toInt32();
    }
}

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
objectConverter(MoveProxy)
objectConverter(AuxPokeDataProxy)
objectConverter(FieldProxy)

#define registerObject(className) qScriptRegisterMetaType<className*>(engine, &to##className, &from##className)

BattleScripting::BattleScripting(QScriptEngine *engine, BaseBattleWindowInterface *_interface)
{
    myinterface = _interface;
    myengine = engine;
    myengine->setParent(this);
    setParent(_interface);

    registerObject(ProxyDataContainer);
    registerObject(TeamProxy);
    registerObject(PokeProxy);
    registerObject(MoveProxy);
    registerObject(AuxPokeDataProxy);
    registerObject(FieldProxy);
    qScriptRegisterMetaType<BattleChoice>(engine, &toBattleChoice, &fromBattleChoice);

    advbattledata_proxy *data = _interface->getBattleData();
    ProxyDataContainer *pdata = data->exposedData();
    int me = data->isPlayer(1) ? 1 : 0;
    int opp = !me;

    QScriptValue battle = myengine->newQObject(_interface);
    myengine->globalObject().setProperty("battle", battle);
    battle.setProperty("data", myengine->newQObject(dynamic_cast<QObject*>(pdata)));
    battle.setProperty("me", QScriptValue(me));
    battle.setProperty("opp", QScriptValue(opp));
    battle.setProperty("id", _interface->battleId());

    /* Removes clientscripting' print function and add ours. Client scripting's print function
      can still be accessed with sys.print() */
    QScriptValue printfun = myengine->newFunction(nativePrint);
    printfun.setData(myengine->newQObject(this));
    myengine->globalObject().setProperty("print", myengine->nullValue());
    myengine->globalObject().setProperty("print", printfun);

    _interface->addOutput(this);
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

QScriptValue BattleScripting::simulateMove(QScriptContext *context, QScriptEngine *engine)
{
    BattleScripting *obj = qobject_cast<BattleScripting*>(context->callee().data().toQObject());

    if (context->argumentCount() <= 0) {
        obj->printLine("Not enough arguments to simulateMove");
        return engine->undefinedValue();
    }
    int move = context->argument(0).toInt32();

    QByteArray data;
    DataStream stream(&data, QIODevice::WriteOnly);
    stream << uchar(BattleEnum::UseAttack) << uchar(0) << qint16(move);

    obj->myinterface->receiveData(data);

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
