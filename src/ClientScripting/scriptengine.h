#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore>
#include <QtScript>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSound>
#include <QHostInfo>

#include "scriptengineagent.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/functions.h"
#include "../Teambuilder/plugininterface.h"

class BaseBattleWindowInterface;

class ScriptEngine : public OnlineClientPlugin
{
    Q_OBJECT
    friend class ScriptEngineAgent;
public:
    ScriptEngine(ClientInterface *c);
    ~ScriptEngine();

    QHash<QString, Hook> getHooks();

    static QScriptValue backtrace(QScriptContext *c, QScriptEngine *);
    static QScriptValue lighter(QScriptContext *c, QScriptEngine *);
    static QScriptValue darker(QScriptContext *c, QScriptEngine *);
    static QScriptValue lightness(QScriptContext *c, QScriptEngine *);
    static QScriptValue tint(QScriptContext *c, QScriptEngine *);

    /* Events */
    void stepEvent();

    void clientStartUp();
    void clientShutDown();

    int beforeSendMessage(const QString &message, int channel);
    int beforeChannelMessage(const QString &message, int channel, bool html);
    int afterChannelMessage(const QString &message, int channel, bool html);
    int beforeNewMessage(const QString &message, bool html);
    int afterNewMessage(const QString &message, bool html);
    int beforePMSent(int id, const QString &message);
    int afterPMSent(int id, const QString &message);
    int beforePMReceived(int id, const QString &message);
    int afterPMReceived(int id, const QString &message);
    int onPlayerReceived(int id);
    int onPlayerRemoved(int id);
    int onPlayerJoinChan(int id, int chan);
    int onPlayerLeaveChan(int id, int chan);
    int beforeChallengeReceived(int challengeId, int oppId, QString tier, int clauses);
    int afterChallengeReceived(int challengeId, int oppId, QString tier, int clauses);
    void onBattleStarted(BaseBattleWindowInterface *w);

    /* Prevents the event from happening.
       For exemple, if called in 'beforeChatMessage', the message won't appear.
       If called in 'beforeChallengeIssued', the challenge won't be issued.
       */
    Q_INVOKABLE void stopEvent();

    /* Print on the client. Useful for debug purposes */
    Q_INVOKABLE void print(QScriptContext *context, QScriptEngine *engine);
    Q_INVOKABLE void clearChat();

    /* escapes html also turns http(s):// into links */
    Q_INVOKABLE QScriptValue htmlEscape(const QString &string);

    /* Sound functions */
    Q_INVOKABLE void beep();
    Q_INVOKABLE void playSound(const QString &file);

    Q_INVOKABLE bool validColor(const QString &color);
    Q_INVOKABLE QString hexColor(const QString &colorname);

    /* 2 timer functions to replace the other 7 */
    Q_INVOKABLE int setTimer(const QScriptValue &v, int delay, bool repeats);
    Q_INVOKABLE bool unsetTimer(int timerId);
    Q_INVOKABLE int unsetAllTimers();

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

    /* Gets the client version. */
    Q_INVOKABLE QScriptValue version();

    /* Evaluates the script given in parameter */
    QScriptValue eval(const QString &script);
    QScriptValue eval(const QString &script, const QString &file);

    static QScriptValue eval(QScriptContext *context, QScriptEngine *engine);

    Q_INVOKABLE int rand(int min, int max);
    Q_INVOKABLE long time();

    Q_INVOKABLE QScriptValue getScript();

    Q_INVOKABLE int pokeType1(int id, int gen = GenInfo::GenMax());
    Q_INVOKABLE int pokeType2(int id, int gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue typeNum(const QString &typeName);
    Q_INVOKABLE QScriptValue type(int id);

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
    Q_INVOKABLE QScriptValue pokeAbility(int poke, int slot, int _gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue baseStats(int poke, int stat, int gen = GenInfo::GenMax());
    Q_INVOKABLE QScriptValue pokeBaseStats(int id, int gen = GenInfo::GenMax());

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);
    /* Qt doesn't convert registered types automatically, have to do it manually */
    static QScriptValue channelNames(QScriptContext *context, QScriptEngine *engine);

    Q_INVOKABLE QString sha1(const QString &text);
    Q_INVOKABLE QString md4(const QString &text);
    Q_INVOKABLE QString md5(const QString &text);

    Q_INVOKABLE bool isSafeScripts();
    Q_INVOKABLE bool showingWarnings();

    // Returns the player's operating system (windows, mac, linux)
    Q_INVOKABLE QScriptValue os();

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
    Q_INVOKABLE bool fileExists(const QString &fileName);
    Q_INVOKABLE void appendToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void writeToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void deleteFile(const QString &fileName);
    Q_INVOKABLE void makeDir(const QString &dir);
    Q_INVOKABLE void removeDir(const QString &dir);
    Q_INVOKABLE QScriptValue getCurrentDir();
    Q_INVOKABLE QScriptValue getFileContent(const QString &path);
    Q_INVOKABLE QScriptValue zip(const QString &path, const QString &directory);
    Q_INVOKABLE QScriptValue extractZip(const QString &zipName, const QString &targetDir);
    Q_INVOKABLE QScriptValue extractZip(const QString &zipName);
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
    void changeBattleScript(const QString &bscript);

private slots:
    void timer();
    void timer_step();

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
    QSound * sound;
    QNetworkAccessManager manager;
    QHash<QNetworkReply*,QScriptValue> webCallEvents;

    QHash<QTimer*,QScriptValue> timerEvents;
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
    QString battleScript;

    void evaluate(const QScriptValue &expr);
    void printLine(const QString &s);
    void armScriptEngine(QScriptEngine *engine);
    void armScripts(QScriptEngine *engine, const QString &scripts, bool trigger=false);
    ClientInterface* client();

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
