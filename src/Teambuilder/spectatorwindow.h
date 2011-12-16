#ifndef SPECTATORWINDOW_H
#define SPECTATORWINDOW_H

#include <QObject>

#include "../BattleManager/battledatatypes.h"

class BattleClientLog;
class BattleScene;
class BattleInput;
class PokeTextEdit;
class BattleConfiguration;

/* A window which takes binary as input, and manages
  a battle scene as well as a battle log.

  Those widgets can be gotten individually or as
  a whole */
class SpectatorWindow : public QObject
{
public:
    SpectatorWindow(const BattleConfiguration &conf, QString name1, QString name2);
    ~SpectatorWindow();

    /* Receives the binary data */
    void receiveData(const QByteArray &data);

    /* Gets the battle log widget */
    PokeTextEdit * getLogWidget();
    /* gets the scene widget */
    QWidget *getSceneWidget();

    /* Gets a premade widget. The caller
      is responsible for managing the widget's lifetime
      and free it */
    QWidget *getSampleWidget();

    void reloadTeam(int player);
private:
    BattleClientLog *log;
    BattleInput *input;
    BattleScene *scene;

    PokeTextEdit* logWidget;

    battledata_basic *data;
    advbattledata_proxy *data2;
};

#endif // SPECTATORWINDOW_H
