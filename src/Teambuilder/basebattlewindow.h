#ifndef BASEBATTLEWINDOW_H
#define BASEBATTLEWINDOW_H

#include "../PokemonInfo/battlestructs.h"
#include "client.h"

#include "basebattlewindowinterface.h"
#include "../BattleManager/battledatatypes.h"
#include "../BattleManager/battlecommandmanager.h"

#ifdef QT5
#include <QMediaPlayer>
#include <QAudioOutput>
#else
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#endif

class BaseBattleDisplay;
class QScrollDownTextBrowser;
class QClickPBar;
struct Log;
class SpectatorWindow;
class QCheckBox;

struct BaseBattleInfo
{
    BaseBattleInfo(const PlayerInfo & me, const PlayerInfo &opp, int mode, int myself=0, int opponent=1);
    /* name [0] = mine, name[1] = other */
    PlayerInfo pInfo[2];
    advbattledata_proxy *data;

    int mode;
    Pokemon::gen gen;

    int myself;
    int opponent;
    int numberOfSlots;

    /* Opponent pokemon */

    QString name(int x) const {
        return pInfo[x].name;
    }
};

/* The battle window called by the client, online */
class Client;

class BaseBattleWindow : public BaseBattleWindowInterface
{
    Q_OBJECT

    PROPERTY(int, ownid)
    PROPERTY(bool, started)
    PROPERTY(bool, playBattleCries)
    PROPERTY(bool, playBattleMusic)
    PROPERTY(FullBattleConfiguration, conf)
public:
    BaseBattleInfo *myInfo;
    const BaseBattleInfo &info() const {
        return *myInfo;
    }
    BaseBattleInfo &info() {
        return *myInfo;
    }

    const advbattledata_proxy &data() const {
        return *getBattleData();
    }
    advbattledata_proxy &data() {
        return *getBattleData();
    }

    virtual void switchToNaught(int){}

    BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf, int ownid);
    void init(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf,
              int _ownid);

    Pokemon::gen gen() const {
        return info().gen;
    }

    virtual void addSpectator(bool add, int id, const QString &name="");

    QString name(int spot) const;
    Q_INVOKABLE int player(int spot) const;
    Q_INVOKABLE int opponent(int player) const;

    void onDisconnection();
    void onKo(int spot);
    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool silent);
    void onSendBack(int spot, bool silent);
    void onUseAttack(int spot, int attack, bool);
    void onSpectatorJoin(int id, const QString& name);
    void onSpectatorLeave(int id);
    void onBattleEnd(int res, int winner);

    bool musicPlayed() const;
    bool flashWhenMoved() const;
    virtual void disable();

    void receiveData(const QByteArray&);
public slots:
    void close();

    void sendMessage();
    void clickClose();
    void delay(qint64 msec=0);
    void undelay();
    void playCry(int pokemon);
    void changeCryVolume(int);
    void changeMusicVolume(int);

    void ignoreSpectators();

    void musicPlayStop();
    void enqueueMusic();
#ifdef QT5
    void criesProblem(QAudio::State newState);
#else
    void criesProblem(Phonon::State newState);
#endif
protected:
    int ignoreSpecs;

    enum IgnoreMode {
        NoIgnore,
        IgnoreSpecs,
        IgnoreAll
    };

    QGridLayout *mylayout;
    QScrollDownTextBrowser *mychat;
    QIRCLineEdit *myline;
    QPushButton *myclose, *mysend, *myignore;

    QCheckBox *saveLogs;
    QCheckBox *musicOn;
    QCheckBox *flashWhenMoveDone;

#ifdef QT5
    QMediaPlayer *audio;
    QAudioOutput *cry;
#else
    /* The device which outputs the sound */
    Phonon::AudioOutput *audioOutput;
    /* The media the device listens from */
    Phonon::MediaObject *mediaObject;
    /* The device for cries */
    Phonon::AudioOutput *cryOutput;
    /* The media the device listens from for pokemon cries */
    Phonon::MediaObject * cryObject;
    /* The pokemon cries stored in memory */
#endif
    /* The media sources for the music */
    QList<QString> sources;

    QHash<int, QByteArray> cries;
    QBuffer cryBuffer;
    bool undelayOnSounds;

    QSet<int> spectators;

    bool blankMessage;
    bool battleEnded;

    Log *log, *replay;
    SpectatorWindow *test;

    struct ReplaySavingData {
        QElapsedTimer t;
        QByteArray data;
    };

    ReplaySavingData replayData;

    BaseBattleWindow();
    void init();
    void checkAndSaveLog();
    void flashIfNeeded();
    void addReplayData(const QByteArray &inf);

    void closeEvent(QCloseEvent *);
};

class BaseBattleWindowIns : public BaseBattleWindow, public BattleCommandManager<BaseBattleWindowIns>
{
public:
    BaseBattleWindowIns(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf, int ownid)
        : BaseBattleWindow(){
        init(me, opponent, conf, ownid);
    }
};
#endif // BASEBATTLEWINDOW_H
