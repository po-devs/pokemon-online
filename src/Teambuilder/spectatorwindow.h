#ifndef SPECTATORWINDOW_H
#define SPECTATORWINDOW_H

#include "../BattleManager/battledatatypes.h"
#include "../BattleManager/battleenum.h"

template <class T> class FlowCommandManager;
class BattleClientLog;
class BattleScene;
class BattleInput;
class QScrollDownTextBrowser;
class QWidget;
class PlayerInfo;
struct FullBattleConfiguration;

/* A window which takes binary as input, and manages
  a battle scene as well as a battle log.

  Those widgets can be gotten individually or as
  a whole */
/* The virtual functions are so that plugins can access them -
  otherwise you'd have to link against the binary (why not?) */
class SpectatorWindow
{
public:
    SpectatorWindow();
    SpectatorWindow(const FullBattleConfiguration &conf);
    ~SpectatorWindow();

    void init(const FullBattleConfiguration &conf);

    /* Receives the binary data */
    virtual void receiveData(const QByteArray &data);

    /* Gets the battle log widget */
    virtual QScrollDownTextBrowser * getLogWidget();
    /* gets the scene widget */
    QWidget *getSceneWidget();

    BattleClientLog *getLog();
    FlowCommandManager<BattleEnum> * getBattle();
    virtual BattleInput *getInput();
    virtual void addOutput(FlowCommandManager<BattleEnum>*);

    virtual advbattledata_proxy *getBattleData() const;

    /* Gets a premade widget. The caller
      is responsible for managing the widget's lifetime
      and free it */
    QWidget *getSampleWidget();

    void reloadTeam(int player);

    void onDisconnection();
private:
    BattleClientLog *log;
    BattleInput *input;

    QWidget *battleView;
    BattleScene *scene;

    QScrollDownTextBrowser* logWidget;

    battledata_basic *mData;
    advbattledata_proxy *data2;

    FlowCommandManager<BattleEnum> *lastOutput;

    static int qmlcount;// qml windows use opengl, so only one can be open at all times
    bool qmlwindow;
};

#endif // SPECTATORWINDOW_H
