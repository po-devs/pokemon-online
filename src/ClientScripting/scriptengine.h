﻿#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore>
#include <QTextEdit>

#include <QtScript>

#include <QScriptValueIterator>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHostInfo>

#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/functions.h"
#include "../Teambuilder/plugininterface.h"

class Server;
class ChallengeInfo;

class ScriptEngine : public OnlineClientPlugin
{
    Q_OBJECT
public:
    ScriptEngine(ClientInterface *c);
    ~ScriptEngine();

    QHash<QString, Hook> getHooks();

    /* Events */
    void stepEvent();

    void clientStartUp();
    void clientShutDown();

    int beforeSendMessage(const QString &message, int channel);
    int beforeChannelMessage(const QString &message, int channel, bool html);
    int afterChannelMessage(const QString &message, int channel, bool html);
    int beforePMReceived(int id, const QString &message);
    int afterPMReceived(int id, const QString &message);
    int playerLogIn(int id);
    int playerLogOut(int id);

    /* Prevents the event from happening.
       For exemple, if called in 'beforeChatMessage', the message won't appear.
       If called in 'beforeChallengeIssued', the challenge won't be issued.
       */
    Q_INVOKABLE void stopEvent();

    /* Print on the client. Useful for debug purposes */
    Q_INVOKABLE void print(QScriptContext *context, QScriptEngine *engine);
    Q_INVOKABLE void clearChat();
    /* Accepts string as 1st parameter. */
    Q_INVOKABLE void callLater(const QString &s, int delay);
    Q_INVOKABLE void callQuickly(const QString &s, int delay);
    /* Accepts function as 1st parameter. */
    Q_INVOKABLE void delayedCall(const QScriptValue &func, int delay);
    /* Evaluates the script given in parameter */
    Q_INVOKABLE QScriptValue eval(const QString &script);

    Q_INVOKABLE int rand(int min, int max);
    Q_INVOKABLE long time();

    Q_INVOKABLE QScriptValue getScript();

    Q_INVOKABLE int pokeType1(int id, int gen = GEN_MAX);
    Q_INVOKABLE int pokeType2(int id, int gen = GEN_MAX);

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);

    Q_INVOKABLE QString sha1(const QString &text);
    Q_INVOKABLE QString md4(const QString &text);
    Q_INVOKABLE QString md5(const QString &text);

    Q_INVOKABLE void hostName(const QString &ip, const QScriptValue &function);

    /* Save vals using the QSettings (persistent vals, that stay after the shutdown of the server */
    Q_INVOKABLE void saveVal(const QString &key, const QVariant &val);
    Q_INVOKABLE void saveVal(const QString &file, const QString &key, const QVariant &val);
    Q_INVOKABLE void saveSetting (const QString &key, const QVariant &val);

    Q_INVOKABLE void removeVal(const QString &key);
    Q_INVOKABLE void removeVal(const QString &file, const QString &key);

    Q_INVOKABLE QScriptValue getVal(const QString &key);
    Q_INVOKABLE QScriptValue getVal(const QString &file, const QString &key);
    Q_INVOKABLE QScriptValue getSetting(const QString &key);

    // Returns an array of Script_* key names in config.
    Q_INVOKABLE QScriptValue getValKeys();
    Q_INVOKABLE QScriptValue getValKeys(const QString &file);

    // Direct file access.
    Q_INVOKABLE void appendToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void writeToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void deleteFile(const QString &fileName);
    Q_INVOKABLE QScriptValue getFileContent(const QString &path);/* GET call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QScriptValue &callback);

    /* POST call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QScriptValue &callback, const QScriptValue &params_array);
/* synchronous GET call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring);
    /* synchronous POST call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring, const QScriptValue &params_array);
public slots:
    void changeScript(const QString &script, const bool triggerStartUp = false);

private slots:
    void timer();
    void timer_step();
    void timerFunc();

    void webCall_replyFinished(QNetworkReply* reply);
    void synchronousWebCall_replyFinished(QNetworkReply* reply);

    void hostInfo_Ready(const QHostInfo &myInfo);

private:
    ClientInterface *myclient;
    QScriptEngine myengine;
    QScriptValue myscript;
    QTimer * step_timer;
    QVector<bool> stopevents;

    QNetworkAccessManager manager;
    QHash<QTimer*,QString> timerEvents;
    QHash<QTimer*,QScriptValue> timerEventsFunc;
    QHash<QNetworkReply*,QScriptValue> webCallEvents;
    QHash<int,QScriptValue> myHostLookups;

    void startStopEvent() {stopevents.push_back(false);}
    bool endStopEvent() {
        bool res = stopevents.back();
        stopevents.pop_back();
        return res;
    }

    QEventLoop sync_loop;
    QString sync_data;

    void evaluate(const QScriptValue &expr);
    void printLine(const QString &s);

    void warn(const QString &function, const QString &message);

    template <typename ...Params>
    void makeEvent(const QString &event, Params&&... params);
    template <typename ...Params>
    bool makeSEvent(const QString &event, Params&&... params);
};

template<typename ...Params>
void ScriptEngine::makeEvent(const QString &event, Params &&... params)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return;

    QScriptValueList l;
    evaluate(myscript.property(event).call(myscript, pack(l, params...)));
}

template<typename ...Params>
bool ScriptEngine::makeSEvent(const QString &event, Params &&... params)
{
    if (!myscript.property(event, QScriptValue::ResolveLocal).isValid())
        return true;

    startStopEvent();

    QScriptValueList l;
    evaluate(myscript.property(event).call(myscript, pack(l, params...)));

    return !endStopEvent();
}

#endif // SCRIPTENGINE_H
