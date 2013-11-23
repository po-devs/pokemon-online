#include "abilities.h"
#include "items.h"
#include "moves.h"
#include "rbymoves.h"
#include "battleserver.h"

BattleServer::BattleServer(QObject *parent) :
    QObject(parent)
{
}

void BattleServer::start()
{
    MoveEffect::init();
    RBYMoveEffect::init();
    ItemEffect::init();
    AbilityEffect::init();
}
