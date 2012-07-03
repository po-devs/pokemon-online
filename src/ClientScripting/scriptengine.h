#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore>
#include <QtScript>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QHostInfo>

#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/functions.h"
#include "../Teambuilder/plugininterface.h"

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
    int onPlayerReceived(int id);
    int onPlayerRemoved(int id);

    /* Prevents the event from happening.
       For exemple, if called in 'beforeChatMessage', the message won't appear.
       If called in 'beforeChallengeIssued', the challenge won't be issued.
       */
    Q_INVOKABLE void stopEvent();

    /* Print on the client. Useful for debug purposes */
    Q_INVOKABLE void print(QScriptContext *context, QScriptEngine *engine);
    Q_INVOKABLE void clearChat();

    Q_INVOKABLE bool validColor(const QString &color);
    Q_INVOKABLE QString hexColor(const QString &colorname);

    /* Accepts string as 1st parameter. */
    Q_INVOKABLE int callLater(const QString &s, int delay);
    Q_INVOKABLE int callQuickly(const QString &s, int delay);

    /* Accepts function as 1st parameter. */
    Q_INVOKABLE int quickCall(const QScriptValue &func, int delay);
    Q_INVOKABLE int delayedCall(const QScriptValue &func, int delay);

    /* Interval timers. */
    Q_INVOKABLE int intervalTimer(const QString &expr, int delay);
    Q_INVOKABLE int intervalCall(const QScriptValue &func, int delay);

    /* Stops a timer. */
    Q_INVOKABLE bool stopTimer(int timerId);

    /* Evaluates the script given in parameter */
    Q_INVOKABLE QScriptValue eval(const QString &script);

    Q_INVOKABLE int rand(int min, int max);
    Q_INVOKABLE long time();

    Q_INVOKABLE QScriptValue getScript();

    Q_INVOKABLE int pokeType1(int id, int gen = GenInfo::GenMax());
    Q_INVOKABLE int pokeType2(int id, int gen = GenInfo::GenMax());

    Q_INVOKABLE QScriptValue pokemon(int num);
    Q_INVOKABLE QScriptValue pokeNum(const QString &name);
    Q_INVOKABLE QScriptValue move(int num);
    Q_INVOKABLE QScriptValue moveNum(const QString &name);
    Q_INVOKABLE int moveType(int moveNum, int gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue item(int num);
    Q_INVOKABLE QScriptValue itemNum(const QString &item);
    Q_INVOKABLE QScriptValue nature(int num);
    Q_INVOKABLE QScriptValue natureNum(const QString &nature);
    Q_INVOKABLE QScriptValue ability(int num);
    Q_INVOKABLE QScriptValue abilityNum(const QString &nature);
    Q_INVOKABLE QScriptValue genderNum(QString genderName);
    Q_INVOKABLE QString gender(int genderNum);

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);

    Q_INVOKABLE QString sha1(const QString &text);
    Q_INVOKABLE QString md4(const QString &text);
    Q_INVOKABLE QString md5(const QString &text);

    Q_INVOKABLE bool isSafeScripts();
    Q_INVOKABLE bool showingWarnings();

    Q_INVOKABLE void hostName(const QString &ip, const QScriptValue &function);

    /* Save vals using the QSettings (persistent vals, that stay after the shutdown of the server */
    Q_INVOKABLE void saveVal(const QString &key, const QVariant &val);
    Q_INVOKABLE void removeVal(const QString &key);
    Q_INVOKABLE QScriptValue getVal(const QString &key);
    Q_INVOKABLE QScriptValue getValKeys();

    /* Save vals in the system registry. */
    Q_INVOKABLE void saveRegVal(const QString &key, const QVariant &val);
    Q_INVOKABLE void removeRegVal(const QString &key);
    Q_INVOKABLE QScriptValue getRegVal(const QString &key);
    Q_INVOKABLE QScriptValue getRegKeys();

    Q_INVOKABLE QScriptValue filesForDirectory (const QString &dir_);
    Q_INVOKABLE QScriptValue dirsForDirectory (const QString &dir_);

    // Direct file access.
    Q_INVOKABLE void appendToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void writeToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void deleteFile(const QString &fileName);
    Q_INVOKABLE QScriptValue getFileContent(const QString &path);

    /* GET call */
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

    void changeSafeScripts(bool safe);
    void changeWarnings(bool warn);

private:
    ClientInterface *myclient;
    QScriptEngine myengine;
    QScriptValue myscript;
    QTimer * step_timer;
    QVector<bool> stopevents;

    QNetworkAccessManager manager;
    QHash<QNetworkReply*,QScriptValue> webCallEvents;

    QHash<QTimer*,QString> timerEvents;
    QHash<QTimer*,QScriptValue> timerEventsFunc;
    QHash<int,QScriptValue> myHostLookups;

    void startStopEvent() {stopevents.push_back(false);}
    bool endStopEvent() {
        bool res = stopevents.back();
        stopevents.pop_back();
        return res;
    }

    QEventLoop sync_loop;
    QString sync_data;

    bool safeScripts;
    bool warnings;

    QString datalocation;

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
