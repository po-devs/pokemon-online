#include <QtDeclarative/QDeclarativeView>
#include <QDeclarativeError>

#include "spectatorwindow.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battledata.h"
#include "../BattleManager/battleinput.h"
#include "../BattleManager/battlescene.h"
#include "../BattleManager/battledatatypes.h"
#include "poketextedit.h"
#include "theme.h"

SpectatorWindow::SpectatorWindow(QString name1, QString name2)
{
    battledata_basic * data = new battledata_basic();
    battledata_proxy * data2 = new battledata_proxy();

    data->team(0).name() = data2->team(0).name() = name1;
    data->team(1).name() = data2->team(1).name() = name2;

    log = new BattleClientLog(data, Theme::getBattleTheme());
    input = new BattleInput();
    scene = new BattleScene(data2);

    /* Will get unpaused by scene when it's loaded */
    input->pause();

    input->addOutput(data);
    input->addOutput(log);
    input->addOutput(data2);
    input->addOutput(scene);

    logWidget = new PokeTextEdit();

    connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));
    connect(scene, SIGNAL(printMessage(QString)), logWidget, SLOT(insertPlainText(QString)));

    foreach(QDeclarativeError error, scene->getWidget()->errors()) {
        log->printLine(error.toString());
    }
}

PokeTextEdit *SpectatorWindow::getLogWidget()
{
    return logWidget;
}

QDeclarativeView *SpectatorWindow::getSceneWidget()
{
    return scene->getWidget();
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
    input->deleteTree();
    delete input;
}

void SpectatorWindow::receiveData(const QByteArray &data)
{
    input->receiveData(data);
}
