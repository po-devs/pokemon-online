#ifndef BASEBATTLEWINDOWINTERFACE_H
#define BASEBATTLEWINDOWINTERFACE_H

#include <QLabel>
#include "../Utilities/functions.h"
#include "spectatorwindow.h"

struct BattleChoice;

class BaseBattleWindowInterface : public QWidget, public SpectatorWindow
{
    Q_OBJECT
    PROPERTY(quint32, battleId)
public:
    virtual void disable() {}
signals:
    void closedBW(int);
    void battleMessage(int, QString);
    void battleCommand(int battleId, const BattleChoice &);
};

#endif // BASEBATTLEWINDOWINTERFACE_H
