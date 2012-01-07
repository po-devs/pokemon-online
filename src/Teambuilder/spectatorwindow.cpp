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

SpectatorWindow::SpectatorWindow(const FullBattleConfiguration &conf)
{
    qmlwindow = false;
    data = new battledata_basic(&conf);
    data2 = new advbattledata_proxy(&conf);

    data->team(0).name() = conf.getName(0);
    data->team(1).name() = conf.getName(1);
    data2->team(0).setName(conf.getName(0));
    data2->team(1).setName(conf.getName(1));

    QSettings s;
    bool usePokemonNames = s.value("use_pokemon_names").toBool();

    log = new BattleClientLog(data, Theme::getBattleTheme(), !usePokemonNames);
    input = new BattleInput(&conf);

    logWidget = new PokeTextEdit();
    connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));

    /* All the previous message which didn't get a chance to be emitted */
    log->emitAll();

    bool qml = !(s.value("old_battle_window", true).toBool() || conf.mode != ChallengeInfo::Singles || qmlcount > 0);

    if (qml) {
        BattleScene *scene = new BattleScene(data2);

        input->addOutput(scene);
        scene->addOutput(data);
        scene->addOutput(log);
        scene->addOutput(data2);

        connect(scene, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

        scene->launch();

        foreach(QDeclarativeError error, scene->getWidget()->errors()) {
            log->printLine("Error", error.toString());
        }

        battleView = scene->getWidget();

        qmlcount ++;
        qmlwindow = true;

        lastOutput = scene;
    } else {
        RegularBattleScene *battle = new RegularBattleScene(data2, Theme::getBattleTheme(), !usePokemonNames);

        input->addOutput(data);
        input->addOutput(log);
        input->addOutput(data2);
        input->addOutput(battle);
        battle->deletable = false;

        connect(battle, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

        battleView = battle;
        lastOutput = battle;
    }
}

FlowCommandManager<BattleEnum> * SpectatorWindow::getBattle()
{
    return lastOutput;
}

void SpectatorWindow::reloadTeam(int player)
{
    data->reloadTeam(player);
    data2->reloadTeam(player);
}

BattleClientLog* SpectatorWindow::getLog()
{
    return log;
}

BattleInput *SpectatorWindow::getInput()
{
    return input;
}

PokeTextEdit *SpectatorWindow::getLogWidget()
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
        qmlcount--;
    }

    input->deleteTree();
    delete input;
}

void SpectatorWindow::receiveData(const QByteArray &data)
{
    input->receiveData(data);
}

advbattledata_proxy *SpectatorWindow::getBattleData()
{
    return data2;
}
