#ifndef TESTMEGANATURE_H
#define TESTMEGANATURE_H

#include "battleservertest.h"

/**
 * @brief Check if a pokemon's nature vanishes during mega evolution
 */
class TestMegaNature : public BattleServerTest
{
public:
    void onBattleServerConnected();
    void onBattleMessage(int b, int p, const QByteArray &);
};

#endif // TESTMEGANATURE_H
