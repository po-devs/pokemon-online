#ifndef BASEBATTLEWINDOWINTERFACE_H
#define BASEBATTLEWINDOWINTERFACE_H

#include <QLabel>
#include "../Utilities/functions.h"

class BaseBattleWindowInterface : public QWidget
{
    Q_OBJECT
    PROPERTY(quint32, battleId)
public:

public slots:
    virtual void receiveInfo(QByteArray){}
signals:
    void closedBW(int);
    void battleMessage(int, QString);
};

#endif // BASEBATTLEWINDOWINTERFACE_H
