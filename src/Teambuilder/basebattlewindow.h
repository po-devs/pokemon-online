#ifndef BASEBATTLEWINDOW_H
#define BASEBATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"
#include "client.h"

#include "basebattlewindowinterface.h"
#include "../BattleManager/battledatatypes.h"
#include "../BattleManager/battlecommandmanager.h"

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>

class BaseBattleDisplay;
class QScrollDownTextBrowser;
class QClickPBar;
class Log;
class SpectatorWindow;

struct BaseBattleInfo
{
    BaseBattleInfo(const PlayerInfo & me, const PlayerInfo &opp, int mode, int myself=0, int opponent=1);
    /* name [0] = mine, name[1] = other */
    PlayerInfo pInfo[2];
    advbattledata_proxy *data;

    int mode;
    int gen;

    int myself;
    int opponent;
    int numberOfSlots;

    /* Opponent pokemon */

    QString name(int x) const {
        return pInfo[x].team.name;
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
        return *info().data;
    }
    advbattledata_proxy &data() {
        return *info().data;
    }

    virtual void switchToNaught(int){}

    BaseBattleWindow(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf, int ownid, Client *client);
    void init(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf,
              int _ownid, Client *client);

    int gen() const {
        return info().gen;
    }

    virtual void addSpectator(bool add, int id);

    //void playCry(int pokenum);

    QString name(int spot) const;
    int player(int spot) const;
    int opponent(int player) const;

    Client *& client() {
        return _mclient;
    }

    const Client * client() const {
        return _mclient;
    }

    void onKo(int spot);
    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool silent);
    void onSendBack(int spot, bool silent);
    void onUseAttack(int spot, int attack, bool);
    void onSpectatorJoin(int id, const QString& name);
    void onSpectatorLeave(int id);
    void onBattleEnd(int res, int winner);

    bool musicPlayed() const;
    bool flashWhenMoved() const;
    void close();
    virtual void disable();

public slots:
    void receiveInfo(QByteArray);
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
    void criesProblem(Phonon::State newState);
signals:
    void battleCommand(int battleId, const BattleChoice &);
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
    Client *_mclient;

    QCheckBox *saveLogs;
    QCheckBox *musicOn;
    QCheckBox *flashWhenMoveDone;

    /* The device which outputs the sound */
    Phonon::AudioOutput *audioOutput;
    /* The media the device listens from */
    Phonon::MediaObject *mediaObject;
    /* The media sources for the music */
    QList<QString> sources;
    /* The device for cries */
    Phonon::AudioOutput *cryOutput;
    /* The media the device listens from for pokemon cries */
    Phonon::MediaObject * cryObject;
    /* The pokemon cries stored in memory */
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

    void closeEvent(QCloseEvent *);
};

class BaseBattleWindowIns : public BaseBattleWindow, public BattleCommandManager<BaseBattleWindowIns>
{
public:
    BaseBattleWindowIns(const PlayerInfo &me, const PlayerInfo &opponent, const BattleConfiguration &conf, int ownid, Client *client)
        : BaseBattleWindow(){
        init(me, opponent, conf, ownid, client);
    }
};
#endif // BASEBATTLEWINDOW_H
