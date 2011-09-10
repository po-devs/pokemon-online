#include <QtDeclarative/QDeclarativeView>

#include "spectatorwindow.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battledata.h"
#include "../BattleManager/battleinput.h"
#include "../BattleManager/battlescene.h"
#include "poketextedit.h"

SpectatorWindow::SpectatorWindow()
{
    data = new BattleData();
    log = new BattleClientLog(data);
    input = new BattleInput();
    scene = new BattleScene(data);

    input->addOutput(data);
    input->addOutput(log);
    input->addOutput(scene);

    logWidget = new PokeTextEdit();

    /* To not worry about memory management */
    logWidget->QObject::setParent(this);
    scene->getWidget()->QObject::setParent(this);

    connect(log, SIGNAL(lineToBePrinted(QString)), logWidget, SLOT(insertHtml(QString)));
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
    ret->setAttribute(Qt::WA_DeleteOnClose, true);

    QHBoxLayout *layout = new QHBoxLayout(ret);
    layout->addWidget(getSceneWidget());
    layout->addWidget(getLogWidget());

    return ret;
}

SpectatorWindow::~SpectatorWindow()
{
    input->deleteTree();
}

void SpectatorWindow::receiveData(const QByteArray &data)
{
    input->receiveData(data);
}
