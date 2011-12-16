#include "spectate.h"
#include "spectatorwindow.h"

#include <QLayout>
#include <QDeclarativeView>
#include <QCheckBox>
#include "poketextedit.h"

SpectatingWindow::SpectatingWindow(const BattleConfiguration &conf, QString name1, QString name2)
{
    this->conf = conf;
    this->conf.receivingMode[0] = this->conf.receivingMode[1] = BattleConfiguration::Spectator;

    setAttribute(Qt::WA_DeleteOnClose, true);

    SpectatorWindow *window = new SpectatorWindow(this->conf, name1, name2);
    connect(this, SIGNAL(destroyed()), window, SLOT(deleteLater()));

    QHBoxLayout *layout = new QHBoxLayout(this);
    QGridLayout *sceneLayout = new QGridLayout();
    layout->addLayout(sceneLayout);
    sceneLayout->addWidget(window->getSceneWidget(), 0, 0, 1, 3);
    sceneLayout->addWidget(new QCheckBox(tr("Music")), 1, 0);
    sceneLayout->addWidget(new QCheckBox(tr("Save log")), 1, 1);
    sceneLayout->addWidget(new QCheckBox(tr("Save replay")), 1, 2);
    layout->addWidget(window->getLogWidget());

    this->window = window;
}

void SpectatingWindow::closeEvent(QCloseEvent *)
{
    emit closedBW(battleId());
    close();
}

void SpectatingWindow::receiveInfo(QByteArray data)
{
    window->receiveData(data);
}
