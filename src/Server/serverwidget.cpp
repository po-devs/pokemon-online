#include "../Utilities/otherwidgets.h"
#include "serverwidget.h"
#include "server.h"
#include "player.h"
#include "challenge.h"
#include "battle.h"
#include "moves.h"
#include "items.h"
#include "abilities.h"
#include "playerswindow.h"
#include "security.h"
#include "antidos.h"
#include "serverconfig.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/movesetchecker.h"
#include "../Utilities/otherwidgets.h"
#include "scriptengine.h"
#include "../Shared/config.h"
#include "tiermachine.h"
#include "battlingoptions.h"
#include "sql.h"
#include "sqlconfig.h"

ServerWidget::ServerWidget(Server *myserver)
{
    server = myserver;
    
    try{

        QGridLayout *mylayout = new QGridLayout (this);

        mylist = new QListWidget();
        mylayout->addWidget(mylist,1,0,2,1);

        mymainchat = new QScrollDownTextEdit();
        mylayout->addWidget(mymainchat,1,1);

        myline = new QLineEdit();
        mylayout->addWidget(myline, 2,1);

        mylist->setContextMenuPolicy(Qt::CustomContextMenu);
        mylist->setSortingEnabled(true);
        mylist->setFixedWidth(150);

        connect(mylist, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
        connect(myline, SIGNAL(returnPressed()), SLOT(sendServerMessage()));

        //the server emits a signal when there is a (chat)message to be shown
        connect(server, SIGNAL(chatmessage(QString)), SLOT(addChatline(QString)));
        connect(server, SIGNAL(servermessage(QString)), SLOT(addChatline(QString)));

        connect(server, SIGNAL(player_incomingconnection(int)), SLOT(playerConnects(int)));
        connect(server, SIGNAL(player_logout(int)), SLOT(playerDisconnects(int)));
        connect(server, SIGNAL(player_authchange(int, QString)), SLOT(playerChangedName(int, QString)));

        mainchat()->setMinimumWidth(500);

    } catch (const QString &e) {
        qDebug() << "Exception" << e;
    }


}


QMenuBar* ServerWidget::createMenuBar() {
    QMenuBar *bar = new QMenuBar(this);
    QMenu *options = bar->addMenu("&Options");
    options->addAction("&Players", this, SLOT(openPlayers()));
    options->addAction("&Anti DoS", this, SLOT(openAntiDos()));
    options->addAction("&Config", this, SLOT(openConfig()));
    options->addAction("&Scripts", this, SLOT(openScriptWindow()));
    options->addAction("&Tiers", this, SLOT(openTiersWindow()));
    options->addAction("&Battle Config", this, SLOT(openBattleConfigWindow()));
    options->addAction("&SQL Config", this, SLOT(openSqlConfigWindow()));
    return bar;
}

void ServerWidget::showContextMenu(const QPoint &p) {
    QIdListWidgetItem *item = dynamic_cast<QIdListWidgetItem*>(list()->itemAt(p));

    if (item)
    {
        QMenu *menu = new QMenu(this);

        QSignalMapper *mymapper3 = new QSignalMapper(menu);
        QAction *viewinfo = menu->addAction("&Silent Kick", mymapper3, SLOT(map()));
        mymapper3->setMapping(viewinfo, item->id());
        connect(mymapper3, SIGNAL(mapped(int)), server, SLOT(silentKick(int)));

        QSignalMapper *mymapper = new QSignalMapper(menu);
        viewinfo = menu->addAction("&Kick", mymapper, SLOT(map()));
        mymapper->setMapping(viewinfo, item->id());
        connect(mymapper, SIGNAL(mapped(int)), server, SLOT(kick(int)));

        QSignalMapper *mymapper2 = new QSignalMapper(menu);
        QAction *viewinfo2 = menu->addAction("&Ban", mymapper2, SLOT(map()));
        mymapper2->setMapping(viewinfo2, item->id());
        connect(mymapper2, SIGNAL(mapped(int)), server, SLOT(ban(int)));

        menu->exec(mapToGlobal(p));
    }
}

void ServerWidget::openPlayers()
{
    PlayersWindow *w = new PlayersWindow();

    w->show();

    connect(w, SIGNAL(authChanged(QString,int)), server, SLOT(changeAuth(QString, int)));
    connect(w, SIGNAL(banned(QString)), server, SLOT(banName(QString)));
}

void ServerWidget::openAntiDos()
{
    AntiDosWindow *w = new AntiDosWindow();

    w->show();
}

void ServerWidget::openConfig()
{
    ServerWindow *w = new ServerWindow();

    w->show();

    connect(w, SIGNAL(nameChanged(QString)), server, SLOT(regNameChanged(const QString)));
    connect(w, SIGNAL(descChanged(QString)), server, SLOT(regDescChanged(const QString)));
    connect(w, SIGNAL(maxChanged(int)), server, SLOT(regMaxChanged(int)));
    connect(w, SIGNAL(privacyChanged(int)), server, SLOT(regPrivacyChanged(int)));
    connect(w, SIGNAL(announcementChanged(QString)), server, SLOT(announcementChanged(QString)));
    connect(w, SIGNAL(logSavingChanged(bool)), server, SLOT(logSavingChanged(bool)));
}

void ServerWidget::openScriptWindow()
{
    myscriptswindow = new ScriptWindow();

    myscriptswindow->show();

    connect(myscriptswindow, SIGNAL(scriptChanged(QString)), server, SLOT(changeScript(QString)));
}

void ServerWidget::openTiersWindow()
{
    TierWindow *w = new TierWindow();

    w->show();

    connect(w, SIGNAL(tiersChanged()), server, SLOT(tiersChanged()));
}

void ServerWidget::openBattleConfigWindow()
{
    BattlingOptionsWindow *w = new BattlingOptionsWindow();

    w->show();

    connect(w, SIGNAL(settingsChanged()), server, SLOT(loadRatedBattlesSettings()));
}

void ServerWidget::openSqlConfigWindow()
{
    SQLConfigWindow *w = new SQLConfigWindow();

    w->show();
}


QScrollDownTextEdit * ServerWidget::mainchat()
{
    return mymainchat;
}

QListWidget * ServerWidget::list() {
    return mylist;
}


void ServerWidget::sendServerMessage()
{
    if (myline->text().trimmed().length() == 0) {
        return;
    }

    //Not sure if it's better to this directly or with a connect
    server->sendServerMessage(myline->text());

    myline->clear();
}


void ServerWidget::addChatline(const QString &line){
    mainchat()->insertPlainText(line + "\n");

}

void ServerWidget::atShutDown()
{
    server->atServerShutDown();
}

/**
 * The following functions are slots which will take care
 * of the online userlist
 */
void ServerWidget::playerConnects(int id){
    QIdListWidgetItem *it = new QIdListWidgetItem(id, QString::number(id));
    list()->addItem(it);
    myplayersitems[id] = it;
}

void ServerWidget::playerChangedName(int id, const QString &name){
    myplayersitems[id]->setText(name);
}

void ServerWidget::playerDisconnects(int id){
    int row = list()->row(myplayersitems[id]);
    delete list()->takeItem(row);
    myplayersitems.remove(id);
}
