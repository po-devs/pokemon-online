#ifndef SESSIONDATAFACTORY_H
#define SESSIONDATAFACTORY_H

#include <QObject>
#include <QtScript>

class SessionDataFactory : public QObject
{
Q_OBJECT
public:
    explicit SessionDataFactory(QScriptEngine *engineLink);
    bool isUserFactoryEnabled() { return userFactoryEnabled; }
    bool isChannelFactoryEnabled() { return channelFactoryEnabled; }
    void disableAll() { userFactoryEnabled = false; channelFactoryEnabled = false; }
    void handleUserLogIn(int user_id);
    void handleUserLogOut(int user_id);
    void handleChannelCreate(int channel_id);
    void handleChannelDestroy(int channel_id);
    void handleInitialState();

    Q_INVOKABLE void registerUserFactory(QScriptValue factoryFunction);
    Q_INVOKABLE void registerChannelFactory(QScriptValue factoryFunction);
    Q_INVOKABLE QScriptValue users(int id);
    Q_INVOKABLE QScriptValue channels(int id);

private:
    QScriptEngine *engine;
    QScriptValue userFactoryFunction;
    QScriptValue channelFactoryFunction;
    QHash<int, QScriptValue> userFactoryStorage;
    QHash<int, QScriptValue> channelFactoryStorage;
    bool userFactoryEnabled;
    bool channelFactoryEnabled;
    bool initialStateHandled;
};

#endif // SESSIONDATAFACTORY_H
