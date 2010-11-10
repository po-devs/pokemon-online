#include "sessiondatafactory.h"

SessionDataFactory::SessionDataFactory(QScriptEngine *engineLink) :
        engine(engineLink), userFactoryEnabled(false), channelFactoryEnabled(false),
        initialStateHandled(false)
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
    if (isUserFactoryEnabled()) {
        QScriptValue dataObject = engine->newObject();
        userFactoryFunction.call(dataObject, QScriptValueList() << user_id);
        userFactoryStorage[user_id] = dataObject;
    }
}

void SessionDataFactory::handleUserLogOut(int user_id)
{
    if (isUserFactoryEnabled()) {
        userFactoryStorage.remove(user_id);
    }
}

void SessionDataFactory::handleChannelCreate(int channel_id)
{
    if (isChannelFactoryEnabled()) {
        QScriptValue dataObject = engine->newObject();
        channelFactoryFunction.call(dataObject, QScriptValueList() << channel_id);
        channelFactoryStorage[channel_id] = dataObject;
    }
}

void SessionDataFactory::handleChannelDestroy(int channel_id)
{
    if (isChannelFactoryEnabled()) {
        channelFactoryStorage.remove(channel_id);
    }
}

QScriptValue SessionDataFactory::users(int id)
{
    if (isUserFactoryEnabled()) {
        return userFactoryStorage.value(id, engine->undefinedValue());
    }else{
        return engine->undefinedValue();
    }
}

QScriptValue SessionDataFactory::channels(int id)
{
    if (isChannelFactoryEnabled()) {
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
    }
}
