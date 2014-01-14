#ifndef SESSIONDATAFACTORY_H
#define SESSIONDATAFACTORY_H

#include "scriptengine.h"
#include "server.h"

class SessionDataFactory : public QObject
{
    Q_OBJECT
    Q_ENUMS(RefillType)
public:
    SessionDataFactory(ScriptEngine *engine);
    Q_INVOKABLE void disableAll();
    void handleUserLogIn(int user_id);
    void handleUserLogOut(int user_id);
    void handleChannelCreate(int channel_id);
    void handleChannelDestroy(int channel_id);

    enum RefillType {
        RefillAll,
        RefillUsers,
        RefillChannels,
        ConstructGlobal
    };

    Q_INVOKABLE void registerUserFactory(QScriptValue factoryFunction);
    Q_INVOKABLE void registerChannelFactory(QScriptValue factoryFunction);
    Q_INVOKABLE void registerGlobalFactory(QScriptValue factoryFunction);
    Q_INVOKABLE QScriptValue users(int id);
    Q_INVOKABLE QScriptValue channels(int id);
    Q_INVOKABLE QScriptValue global();
    Q_INVOKABLE bool hasUser(int id);
    Q_INVOKABLE bool hasChannel(int id);
    Q_INVOKABLE void identifyScriptAs(QString scriptId);
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void refill(RefillType type = RefillAll);
    Q_INVOKABLE QString dump();
private:
    ScriptEngine *engine;
    QScriptEngine *scriptEngine;
    QScriptValue userFactoryFunction;
    QScriptValue channelFactoryFunction;
    QHash<int, QScriptValue> userFactoryStorage;
    QHash<int, QScriptValue> channelFactoryStorage;
    bool userFactoryEnabled;
    bool channelFactoryEnabled;
    bool globalFactoryEnabled;
    bool globalFactoryConstructed;
    QString currentScriptId;
    QScriptValue globalFactoryFunction;
    QScriptValue globalFactoryStorage;
    void checkError();
};

#endif // SESSIONDATAFACTORY_H
