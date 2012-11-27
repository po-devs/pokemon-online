#include "moveinfoaccessor.h"
#include "../PokemonInfo/pokemoninfo.h"

MoveInfoAccessor::MoveInfoAccessor(QObject *parent, Pokemon::gen gen) :
    QObject(parent),
    mGen(gen)
{
}

QString MoveInfoAccessor::name(int move)
{
    return MoveInfo::Name(move);
}

int MoveInfoAccessor::type(int move)
{
    return MoveInfo::Type(move, mGen);
}

int MoveInfoAccessor::power(int move)
{
    return MoveInfo::Power(move, mGen);
}
