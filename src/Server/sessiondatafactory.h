#ifndef SESSIONDATAFACTORY_H
#define SESSIONDATAFACTORY_H

#include <QObject>
#include <QtScript>
#include <QString>

class SessionDataFactory : public QObject
{
Q_OBJECT
public:
    explicit SessionDataFactory(QScriptEngine *engineLink);
    void disableAll();
    void handleUserLogIn(int user_id);
    void handleUserLogOut(int user_id);
    void handleChannelCreate(int channel_id);
    void handleChannelDestroy(int channel_id);
    void handleInitialState();

    Q_INVOKABLE void registerUserFactory(QScriptValue factoryFunction);
    Q_INVOKABLE void registerChannelFactory(QScriptValue factoryFunction);
    Q_INVOKABLE void registerGlobalFactory(QScriptValue factoryFunction);
    Q_INVOKABLE QScriptValue users(int id);
    Q_INVOKABLE QScriptValue channels(int id);
    Q_INVOKABLE QScriptValue global();
    Q_INVOKABLE void identifyScriptAs(QString scriptId);

private:
    QScriptEngine *engine;
    QScriptValue userFactoryFunction;
    QScriptValue channelFactoryFunction;
    QHash<int, QScriptValue> userFactoryStorage;
    QHash<int, QScriptValue> channelFactoryStorage;
    bool userFactoryEnabled;
    bool channelFactoryEnabled;
    bool initialStateHandled;
    QString currentScriptId;
    void clearAll();
    QScriptValue globalFactoryFunction;
    QScriptValue globalFactoryStorage;
    bool globalFactoryEnabled;
};

#endif // SESSIONDATAFACTORY_H
