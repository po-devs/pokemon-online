#include <QtDeclarative/QDeclarativeView>
#include <QDeclarativeError>

#include "spectatorwindow.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battledata.h"
#include "../BattleManager/battleinput.h"
#include "../BattleManager/battlescene.h"
#include "../BattleManager/regularbattlescene.h"
#include "../BattleManager/battledatatypes.h"
#include "poketextedit.h"
#include "theme.h"
#include "../PokemonInfo/networkstructs.h"

int SpectatorWindow::qmlcount = 0;

SpectatorWindow::SpectatorWindow() {

}

SpectatorWindow::SpectatorWindow(const FullBattleConfiguration &conf)
{
    init(conf);
}

void SpectatorWindow::init(const FullBattleConfiguration &conf)
{
    qmlwindow = false;
    mData = new battledata_basic(&conf);
    data2 = new advbattledata_proxy(&conf);

    mData->team(0).name() = conf.getName(0);
    mData->team(1).name() = conf.getName(1);
    data2->team(0).setName(conf.getName(0));
    data2->team(1).setName(conf.getName(1));

    QSettings s;
    bool usePokemonNames = s.value("Battle/NoNicknames").toBool();

    log = new BattleClientLog(mData, Theme::getBattleTheme(), !usePokemonNames);
    input = new BattleInput(&conf);

    logWidget = new PokeTextEdit();
    QObject::connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));

    /* All the previous message which didn't get a chance to be emitted */
    log->emitAll();

    bool qml = !(s.value("Battle/OldWindow", true).toBool() || conf.mode != ChallengeInfo::Singles || qmlcount > 0);

    if (qml) {
        QVariantMap options;
        options.insert("logger", s.value("Battle/AnimatedLogger", false).toBool());
        options.insert("weather", s.value("Battle/AnimatedWeather", "always"));
        options.insert("screensize", s.value("Battle/AnimatedScreenSize", "712x400"));

        BattleScene *scene = new BattleScene(data2, Theme::getBattleTheme(), options);

        input->addOutput(scene);
        scene->addOutput(mData);
        scene->addOutput(log);
        scene->addOutput(data2);

        QObject::connect(scene, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));
        QObject::connect(log, SIGNAL(lineToBePrinted(QString)), scene, SLOT(log(QString)));

        scene->launch();

        foreach(QDeclarativeError error, scene->getWidget()->errors()) {
            log->printLine("Error", error.toString());
        }

        battleView = scene->getWidget();

#ifndef QT5
        qmlcount ++;
#endif
        qmlwindow = true;

        lastOutput = scene;
    } else {
        RegularBattleScene *battle = new RegularBattleScene(data2, Theme::getBattleTheme(), !usePokemonNames);

        input->addOutput(mData);
        input->addOutput(log);
        input->addOutput(data2);
        input->addOutput(battle);
        battle->deletable = false;

        QObject::connect(battle, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

        battleView = battle;
        lastOutput = battle;
    }
}

FlowCommandManager<BattleEnum> * SpectatorWindow::getBattle()
{
    return lastOutput;
}

void SpectatorWindow::addOutput(FlowCommandManager<BattleEnum> *o)
{
    getBattle()->addOutput(o);
}

void SpectatorWindow::reloadTeam(int player)
{
    mData->reloadTeam(player);
    data2->reloadTeam(player);
}

void SpectatorWindow::onDisconnection()
{
    log->onDisconnection();
}

BattleClientLog* SpectatorWindow::getLog()
{
    return log;
}

BattleInput *SpectatorWindow::getInput()
{
    return input;
}

QScrollDownTextBrowser *SpectatorWindow::getLogWidget()
{
    return logWidget;
}

QWidget *SpectatorWindow::getSceneWidget()
{
    return battleView;
}

QWidget *SpectatorWindow::getSampleWidget()
{
    QWidget *ret = new QWidget();

    QHBoxLayout *layout = new QHBoxLayout(ret);
    layout->addWidget(getSceneWidget());
    layout->addWidget(getLogWidget());

    return ret;
}

SpectatorWindow::~SpectatorWindow()
{
    if (qmlwindow) {
#ifndef QT5
        qmlcount--;
#endif
    }

    input->deleteTree();
    delete input;
}

void SpectatorWindow::receiveData(const QByteArray &data)
{
    input->receiveData(data);
}

advbattledata_proxy *SpectatorWindow::getBattleData() const
{
    return data2;
}
