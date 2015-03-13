#include <QGuiApplication>
#include <QQuickView>
#include <QScreen>
#include <QQmlEngine>
#include <QQmlContext>
#include <QtQml/QtQml>
#include "serverchoicemodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qreal dpi = QGuiApplication::screens().at(0)->logicalDotsPerInch() * app.devicePixelRatio();
    Q_INIT_RESOURCE(qml);

    QQuickView *view = new QQuickView();

    qmlRegisterType<PokemonOnlineQML::ServerChoiceModel>("PokemonOnlineQml", 1, 0, "ServerChoiceModel");

    view->engine()->rootContext()->setContextProperty("screenDpi", dpi);

    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setTitle("Pokemon online");
    view->setSource(QUrl("qrc:/qml/main.qml"));
    view->show();
    return app.exec();
}
