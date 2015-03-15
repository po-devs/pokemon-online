#include <QGuiApplication>
#include <QQuickView>
#include <QScreen>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtQml/QtQml>
#include "serverchoicemodel.h"
#include "libraries/TeambuilderLibrary/poketablemodel.h"
#include "libraries/PokemonInfo/teamholder.h"
#include "libraries/PokemonInfo/pokemoninfo.h"
#include "libraries/PokemonInfo/movesetchecker.h"

void reloadPokemonDatabase() {
    QSettings s;

    GenInfo::init("db/gens/");
    PokemonInfo::init("db/pokes/");
    MoveSetChecker::init("db/pokes/", s.value("TeamBuilder/EnforceMinLevels").toBool());
    ItemInfo::init("db/items/");
    MoveInfo::init("db/moves/");
    TypeInfo::init("db/types/");
    NatureInfo::init("db/natures/");
    CategoryInfo::init("db/categories/");
    AbilityInfo::init("db/abilities/");
    GenderInfo::init("db/genders/");
    HiddenPowerInfo::init("db/types/");
    StatInfo::init("db/status/");
}

int main(int argc, char *argv[])
{
    reloadPokemonDatabase();

    QGuiApplication app(argc, argv);
    qreal dpi = QGuiApplication::screens().at(0)->logicalDotsPerInch() * app.devicePixelRatio();
    Q_INIT_RESOURCE(qml);

    QQuickView *view = new QQuickView();

    qmlRegisterType<PokemonOnlineQML::ServerChoiceModel>("PokemonOnlineQml", 1, 0, "ServerChoiceModel");
    qmlRegisterType<PokeTableModel>("PokemonOnlineQml", 1, 0, "PokeTableModel");
    qmlRegisterType<TeamHolder>("PokemonOnlineQml", 1, 0, "TeamHolder");
    view->engine()->rootContext()->setContextProperty("screenDpi", dpi);

    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setTitle("Pokemon online");
    view->setSource(QUrl("qrc:/qml/main.qml"));
    view->show();
    return app.exec();
}
