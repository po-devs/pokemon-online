#include "sessiondatafactory.h"

SessionDataFactory::SessionDataFactory(ScriptEngine *engine) :
        engine(engine), scriptEngine(engine->getEngine()),
        userFactoryEnabled(false), channelFactoryEnabled(false),
        globalFactoryEnabled(false)
{
}

void SessionDataFactory::registerUserFactory(QScriptValue factoryFunction)
{
    if (factoryFunction.isFunction()) {
        userFactoryFunction = factoryFunction;
        userFactoryEnabled = true;
        refill(RefillUsers);
    }
}

void SessionDataFactory::registerChannelFactory(QScriptValue factoryFunction)
{
    if (factoryFunction.isFunction()) {
        channelFactoryFunction = factoryFunction;
        channelFactoryEnabled = true;
        refill(RefillChannels);
    }
}

void SessionDataFactory::handleUserLogIn(int user_id)
{
    if (userFactoryEnabled) {
        QScriptValue dataObject = userFactoryFunction.construct(QScriptValueList() << user_id);
        userFactoryStorage[user_id] = dataObject;
        checkError();
    }
}

void SessionDataFactory::handleUserLogOut(int user_id)
{
    if (userFactoryEnabled) {
        userFactoryStorage.remove(user_id);
    }
}

void SessionDataFactory::handleChannelCreate(int channel_id)
{
    if (channelFactoryEnabled) {
        QScriptValue dataObject = channelFactoryFunction.construct(QScriptValueList() << channel_id);
        channelFactoryStorage[channel_id] = dataObject;
        checkError();
    }
}

void SessionDataFactory::handleChannelDestroy(int channel_id)
{
    if (channelFactoryEnabled) {
        channelFactoryStorage.remove(channel_id);
    }
}

QScriptValue SessionDataFactory::users(int id)
{
    if (userFactoryEnabled) {
        return userFactoryStorage.value(id, scriptEngine->undefinedValue());
    }else{
        return scriptEngine->undefinedValue();
    }
}

QScriptValue SessionDataFactory::channels(int id)
{
    if (channelFactoryEnabled) {
        return channelFactoryStorage.value(id, scriptEngine->undefinedValue());
    }else{
        return scriptEngine->undefinedValue();
    }
}

void SessionDataFactory::disableAll()
{
    userFactoryEnabled = false;
    channelFactoryEnabled = false;
    globalFactoryEnabled = false;
}

void SessionDataFactory::clearAll()
{
    disableAll();
    userFactoryFunction = QScriptValue();
    channelFactoryFunction = QScriptValue();
    globalFactoryFunction = QScriptValue();
    globalFactoryStorage = QScriptValue();
    userFactoryStorage.clear();
    channelFactoryStorage.clear();
}

void SessionDataFactory::identifyScriptAs(QString scriptId)
{
    if (scriptId.isEmpty() || (currentScriptId != scriptId)) {
        clearAll();
    }

    currentScriptId = scriptId;
}

void SessionDataFactory::registerGlobalFactory(QScriptValue factoryFunction)
{
    if (factoryFunction.isFunction()) {
        globalFactoryFunction = factoryFunction;
        globalFactoryEnabled = true;
        globalFactoryStorage = globalFactoryFunction.construct(QScriptValueList() << currentScriptId);
        checkError();
    }
}

QScriptValue SessionDataFactory::global()
{
    if (globalFactoryEnabled) {
        return globalFactoryStorage;
    }else{
        return scriptEngine->undefinedValue();
    }
}

void SessionDataFactory::refill(RefillType type)
{
    if (type == RefillUsers || type == RefillAll) {
        // Refill player session info if session data is no longer valid.
        QList<int> keys = engine->getServer()->myplayers.keys();
        for (int i = 0; i < keys.size(); i++) {
            if (!hasUser(keys[i])) {
                handleUserLogIn(keys[i]);
            }
        }
    }

    if (type == RefillChannels || type == RefillAll) {
        // Refill channels as well.
        QList<int> keys = engine->getServer()->channels.keys();
        for (int i = 0; i < keys.size(); i++) {
            int current_channel = keys[i];
            if (!hasChannel(current_channel)) {
                handleChannelCreate(current_channel);
            }
        }
    }
}

bool SessionDataFactory::hasUser(int id)
{
    return userFactoryStorage.contains(id);
}

bool SessionDataFactory::hasChannel(int id)
{
    return channelFactoryStorage.contains(id);
}

QString SessionDataFactory::dump()
{
    QStringList dump;
    dump << QString("SESSION dump @ %1").arg(QString::number(engine->time()));
    dump << QString("Script: %1").arg(currentScriptId);
    dump << QString("%1 users, %2 channels.").arg(QString::number(userFactoryStorage.count()), QString::number(channelFactoryStorage.count()));
    dump << QString("User: %1; Channel: %2, Global: %3").arg(userFactoryEnabled ? "enabled" : "disabled",
                                                             channelFactoryEnabled ? "enabled" : "disabled",
                                                             globalFactoryEnabled ? "enabled" : "disabled");
    return dump.join("\n");
}

void SessionDataFactory::checkError()
{
    if (scriptEngine->hasUncaughtException()) {
        engine->printLine(QString("Session handler error: line %1. %2").arg(
            QString::number(scriptEngine->uncaughtExceptionLineNumber())
        ).arg(scriptEngine->uncaughtException().toString()));
    }
}
