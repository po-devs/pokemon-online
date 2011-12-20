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

SpectatorWindow::SpectatorWindow(const BattleConfiguration &conf, const PlayerInfo& p1,
                                 const PlayerInfo& p2)
{
    qmlwindow = false;
    data = new battledata_basic(&conf);
    data2 = new advbattledata_proxy(&conf);

    data->team(0).name() = p1.team.name;
    data->team(1).name() = p2.team.name;
    data2->team(0).setName(p1.team.name);
    data2->team(1).setName(p2.team.name);
    data2->team(0).setAvatar(p1.avatar);
    data2->team(1).setAvatar(p2.avatar);

    log = new BattleClientLog(data, Theme::getBattleTheme());
    input = new BattleInput(&conf);

    logWidget = new PokeTextEdit();
    connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));

    /* All the previous message which didn't get a chance to be emitted */
    log->emitAll();

    QSettings s;

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
    } else {
        RegularBattleScene *battle = new RegularBattleScene(data2, Theme::getBattleTheme());

        input->addOutput(data);
        input->addOutput(log);
        input->addOutput(data2);
        input->addOutput(battle);

        connect(battle, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

        battleView = battle;
    }
}

void SpectatorWindow::reloadTeam(int player)
{
    data->reloadTeam(player);
    data2->reloadTeam(player);
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
