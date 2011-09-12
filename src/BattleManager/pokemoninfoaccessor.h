#ifndef POKEMONINFOACCESSOR_H
#define POKEMONINFOACCESSOR_H

#include <QDeclarativeImageProvider>
#include "../PokemonInfo/pokemoninfo.h"

class PokemonInfoAccessor : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT
public:
    PokemonInfoAccessor() : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap) {

    }

/*    Q_INVOKABLE QString sprite(const QVariantMap &params) {
        return PokemonInfo::Picture(params.value("num", Pokemon::NoPoke).value<Pokemon::uniqueId>(),
                                    params.value("gen", GEN_MAX).toInt(),
                                    params.value("gender", Pokemon::Male).toInt(),
                                    params.value("shiny", false).toBool(),
                                    params.value("back", false).toBool());
    }

    Q_INVOKABLE icon(Pokemon::uniqueId num) {
        return PokemonInfo::Icon(num);
    }*/

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) {
        (void) requestedSize;

        QPixmap ret;

        if (id.startsWith("icon/")) {
            ret = PokemonInfo::Icon(id.section("/", 1).toInt());
        } else if (id.startsWith("pokemon/")){
            /* Todo */
        }

        *size = ret.size();

        return ret;
    }

    static PokemonInfoAccessor *getInstance() {
        if (mInstance == NULL) {
            mInstance = new PokemonInfoAccessor();
        }
        return mInstance;
    }

private:
    static PokemonInfoAccessor *mInstance;
};

#endif // POKEMONINFOACCESSOR_H
