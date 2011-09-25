#ifndef SPECTATORWINDOW_H
#define SPECTATORWINDOW_H

#include <QObject>

class BattleClientLog;
class BattleScene;
class BattleInput;
class PokeTextEdit;
class QDeclarativeView;
class BattleConfiguration;

/* A window which takes binary as input, and manages
  a battle scene as well as a battle log.

  Those widgets can be gotten individually or as
  a whole */
class SpectatorWindow : public QObject
{
public:
    SpectatorWindow(BattleConfiguration &conf, QString name1, QString name2);
    ~SpectatorWindow();

    /* Receives the binary data */
    void receiveData(const QByteArray &data);

    /* Gets the battle log widget */
    PokeTextEdit * getLogWidget();
    /* gets the scene widget */
    QDeclarativeView *getSceneWidget();

    /* Gets a premade widget. The caller
      is responsible for managing the widget's lifetime
      and free it */
    QWidget *getSampleWidget();
private:
    BattleClientLog *log;
    BattleInput *input;
    BattleScene *scene;

    PokeTextEdit* logWidget;
};

#endif // SPECTATORWINDOW_H
