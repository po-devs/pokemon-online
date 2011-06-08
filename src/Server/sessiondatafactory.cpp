#include "sessiondatafactory.h"

SessionDataFactory::SessionDataFactory(QScriptEngine *engineLink) :
        engine(engineLink), userFactoryEnabled(false), channelFactoryEnabled(false),
        initialStateHandled(false), globalFactoryEnabled(false), refillNeeded(false)
{
}

void SessionDataFactory::registerUserFactory(QScriptValue factoryFunction)
{
    if (factoryFunction.isFunction()) {
        userFactoryFunction = factoryFunction;
        userFactoryEnabled = true;
    }
}

void SessionDataFactory::registerChannelFactory(QScriptValue factoryFunction)
{
    if (factoryFunction.isFunction()) {
        channelFactoryFunction = factoryFunction;
        channelFactoryEnabled = true;
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
        return userFactoryStorage.value(id, engine->undefinedValue());
    }else{
        return engine->undefinedValue();
    }
}

QScriptValue SessionDataFactory::channels(int id)
{
    if (channelFactoryEnabled) {
        return channelFactoryStorage.value(id, engine->undefinedValue());
    }else{
        return engine->undefinedValue();
    }
}

void SessionDataFactory::handleInitialState()
{
    if (!initialStateHandled) {
        handleChannelCreate(0);
        initialStateHandled = true;
        if (globalFactoryEnabled) {
            globalFactoryStorage = globalFactoryFunction.construct(QScriptValueList() << currentScriptId);
            checkError();
        }
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
    initialStateHandled = false;
}

void SessionDataFactory::identifyScriptAs(QString scriptId)
{
    if (scriptId.isEmpty() || (currentScriptId != scriptId)) {
        clearAll();
        refillNeeded = true;
    }
    currentScriptId = scriptId;
}

void SessionDataFactory::registerGlobalFactory(QScriptValue factoryFunction)
{
    if (factoryFunction.isFunction()) {
        globalFactoryFunction = factoryFunction;
        globalFactoryEnabled = true;
    }
}

QScriptValue SessionDataFactory::global()
{
    if (globalFactoryEnabled) {
        return globalFactoryStorage;
    }else{
        return engine->undefinedValue();
    }
}

bool SessionDataFactory::isRefillNeeded()
{
    return refillNeeded;
}

void SessionDataFactory::refillDone()
{
    refillNeeded = false;
}

void SessionDataFactory::checkError()
{
    if (engine->hasUncaughtException()) {
        qDebug() << QString("Session handler error: line %1. %2").arg(
            QString::number(engine->uncaughtExceptionLineNumber())
        ).arg(engine->uncaughtException().toString());
    }
}
