#ifndef TESTIMPOSTER_H
#define TESTIMPOSTER_H

#include <PokemonInfo/battlestructs.h>

#include "battleservertest.h"

class TestImposter : public BattleServerTest
{
    Q_OBJECT
public slots:
    void onBattleServerConnected();
    void onBattleMessage(int b, int p, const QByteArray &);
};

#endif // TESTIMPOSTER_H
