#include <QtDeclarative/QDeclarativeView>
#include <QDeclarativeError>

#include "spectatorwindow.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battledata.h"
#include "../BattleManager/battleinput.h"
#include "../BattleManager/battlescene.h"
#include "poketextedit.h"
#include "theme.h"

SpectatorWindow::SpectatorWindow()
{
    data = new BattleData();
    log = new BattleClientLog(data, Theme::getBattleTheme());
    input = new BattleInput();
    scene = new BattleScene(data);

    input->addOutput(data);
    input->addOutput(log);
    input->addOutput(scene);

    logWidget = new PokeTextEdit();

    connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));

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

BattleData *SpectatorWindow::accessData()
{
    return data;
}
