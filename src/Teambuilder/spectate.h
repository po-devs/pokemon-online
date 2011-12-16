#ifndef SPECTATE_H
#define SPECTATE_H

#include "basebattlewindowinterface.h"
#include "../PokemonInfo/battlestructs.h"

class SpectatorWindow;

class SpectatingWindow : public BaseBattleWindowInterface
{
public:
    SpectatingWindow(const BattleConfiguration &conf, QString name1, QString name2);
public slots:
    void receiveInfo(QByteArray);
private:
    BattleConfiguration conf;
    SpectatorWindow *window;
protected:
    void closeEvent(QCloseEvent *);
};

#endif // SPECTATE_H
