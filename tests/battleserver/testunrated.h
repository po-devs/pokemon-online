#ifndef TESTUNRATED_H
#define TESTUNRATED_H

#include <PokemonInfo/battlestructs.h>

#include "battleservertest.h"

class TestUnrated : public BattleServerTest
{
    Q_OBJECT
public slots:
    void onBattleServerConnected();
    void onBattleMessage(int battleid, int playerid, const QByteArray &array);
};

#endif // TESTUNRATED_H
