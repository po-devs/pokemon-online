#include "pokemoninfoaccessor.h"

PokemonInfoAccessor *PokemonInfoAccessor::mInstance = NULL;

QPixmap PokemonInfoAccessor::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    (void) requestedSize;

    QPixmap ret;

    if (id.startsWith("icon/")) {
        ret = PokemonInfo::Icon(id.section("/", 1).toInt());
    } else if (id.startsWith("pokemon/")){
        ret = PokemonInfo::Picture(id.section('/', 1));
    } else if (id.startsWith("item/")) {
        ret = ItemInfo::Icon(id.section("/", 1).toInt());
    }

    *size = ret.size();

    return ret;
}

PokemonInfoAccessor* PokemonInfoAccessor::getInstance()
{
    if (mInstance == NULL) {
        mInstance = new PokemonInfoAccessor();
    }
    return mInstance;
}
