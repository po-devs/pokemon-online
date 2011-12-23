#ifndef SPECTATORWINDOW_H
#define SPECTATORWINDOW_H

#include <QObject>

#include "../BattleManager/battledatatypes.h"
#include "../BattleManager/battleenum.h"

class BattleClientLog;
class BattleScene;
class BattleInput;
class PokeTextEdit;
template <class T> class FlowCommandManager;
class PlayerInfo;
class FullBattleConfiguration;

/* A window which takes binary as input, and manages
  a battle scene as well as a battle log.

  Those widgets can be gotten individually or as
  a whole */
class SpectatorWindow : public QObject
{
public:
    SpectatorWindow(const FullBattleConfiguration &conf);
    ~SpectatorWindow();

    /* Receives the binary data */
    void receiveData(const QByteArray &data);

    /* Gets the battle log widget */
    PokeTextEdit * getLogWidget();
    /* gets the scene widget */
    QWidget *getSceneWidget();

    BattleClientLog *getLog();
    FlowCommandManager<BattleEnum> * getBattle();
    BattleInput *getInput();
    void addOutput(FlowCommandManager<BattleEnum>*);

    advbattledata_proxy *getBattleData();

    /* Gets a premade widget. The caller
      is responsible for managing the widget's lifetime
      and free it */
    QWidget *getSampleWidget();

    void reloadTeam(int player);
private:
    BattleClientLog *log;
    BattleInput *input;

    QWidget *battleView;
    BattleScene *scene;

    PokeTextEdit* logWidget;

    battledata_basic *data;
    advbattledata_proxy *data2;

    FlowCommandManager<BattleEnum> *lastOutput;

    static int qmlcount;// qml windows use opengl, so only one can be open at all times
    bool qmlwindow;
};

#endif // SPECTATORWINDOW_H
