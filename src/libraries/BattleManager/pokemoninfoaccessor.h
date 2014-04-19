#ifndef POKEMONINFOACCESSOR_H
#define POKEMONINFOACCESSOR_H

#include <QQuickImageProvider>
#include <PokemonInfo/pokemoninfo.h>

class PokemonInfoAccessor : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    PokemonInfoAccessor() : QQuickImageProvider(QQuickImageProvider::Pixmap) {

    }

/*    Q_INVOKABLE QString sprite(const QVariantMap &params) {
        return PokemonInfo::Picture(params.value("num", Pokemon::NoPoke).value<Pokemon::uniqueId>(),
                                    params.value("gen", GenInfo::GenMax()).toInt(),
                                    params.value("gender", Pokemon::Male).toInt(),
                                    params.value("shiny", false).toBool(),
                                    params.value("back", false).toBool());
    }

    Q_INVOKABLE icon(Pokemon::uniqueId num) {
        return PokemonInfo::Icon(num);
    }*/

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

    static PokemonInfoAccessor *getInstance();

private:
    static PokemonInfoAccessor *mInstance;
};

#endif // POKEMONINFOACCESSOR_H
