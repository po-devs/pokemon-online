#include <QMenuBar>
#include <QInputDialog>

#include "../Utilities/qscrolldowntextbrowser.h"
#include "serverwidget.h"
#include "server.h"
#include "player.h"
#include "challenge.h"
#include "playerswindow.h"
#include "security.h"
#include "antidos.h"
#include "serverconfig.h"
#include "../Utilities/otherwidgets.h"
#include "scriptengine.h"
#include "../Shared/config.h"
#include "tierwindow.h"
#include "battlingoptions.h"
#include "sql.h"
#include "sqlconfig.h"
#include "pluginmanager.h"
#include "plugininterface.h"
#include "../Utilities/pluginmanagerwidget.h"
#include "modswindow.h"

ServerWidget::ServerWidget(Server *myserver)
{
    server = myserver;

    loadGuiSettings();

    QGridLayout *mylayout = new QGridLayout (this);

    mylist = new QListWidget();
    mylayout->addWidget(mylist,1,0,2,1);

    mymainchat = new QScrollDownTextBrowser();
    mylayout->addWidget(mymainchat,1,1);

    myline = new QLineEdit();
    mylayout->addWidget(myline, 2,1);

    mylist->setContextMenuPolicy(Qt::CustomContextMenu);
    mylist->setSortingEnabled(true);
    mylist->setFixedWidth(150);

    connect(mylist, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendServerMessage()));

    //the server emits a signal when there is a (chat)message to be shown
    connect(server, SIGNAL(chatMessage(QString)), SLOT(addChatline(QString)));
    connect(server, SIGNAL(serverMessage(QString)), SLOT(addChatline(QString)));

    connect(server, SIGNAL(player_incomingconnection(int)), SLOT(playerConnects(int)));
    connect(server, SIGNAL(player_logout(int)), SLOT(playerDisconnects(int)));
    connect(server, SIGNAL(player_authchange(int, QString)), SLOT(playerChangedName(int, QString)));

    mainchat()->setMinimumWidth(500);

    server->start();

    connect(server->myengine, SIGNAL(clearTheChat()), this, SLOT(clearChat()));
}

void ServerWidget::loadGuiSettings()
{
    QSettings s("config", QSettings::IniFormat);

    showTrayPopup = s.value("GUI/ShowTrayPopup", true).toBool();
    minimizeToTray = s.value("GUI/MinimizeToTray", true).toBool();
    doubleClick = s.value("GUI/DoubleClickIcon", true).toBool();
}

QMenuBar* ServerWidget::createMenuBar() {
    QMenuBar *bar = new QMenuBar(this);
    QMenu *options = bar->addMenu("&Options");
    options->addAction("&Players", this, SLOT(openPlayers()));
    options->addAction("&Anti DoS", this, SLOT(openAntiDos()));
    options->addAction("&Config", this, SLOT(openConfig()));
    options->addAction("&Scripts", this, SLOT(openScriptWindow()));
    options->addAction("&Mods", this, SLOT(openModsWindow()));
    options->addAction("&Tiers", this, SLOT(openTiersWindow()));
    options->addAction("&Battle Config", this, SLOT(openBattleConfigWindow()));
    options->addAction("S&QL Config", this, SLOT(openSqlConfigWindow()));
    QMenu *plugins = bar->addMenu("&Plugins");
    plugins->addAction("Plugin &Manager", this, SLOT(openPluginManager()));
    plugins->addSeparator();

    foreach(QString s, server->pluginManager->getVisiblePlugins()) {
        plugins->addAction(s, this, SLOT(openPluginConfig()));
    }

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

        QSignalMapper *mymapper4 = new QSignalMapper(menu);
        QAction *viewinfo3 = menu->addAction("&Temp Ban", mymapper4, SLOT(map()));
        mymapper4->setMapping(viewinfo3, item->id());
        connect(mymapper4, SIGNAL(mapped(int)), this, SLOT(openTempBanDialog(int)));


        menu->exec(mapToGlobal(p));
    }
}

void ServerWidget::clearChat()
{
    mymainchat->clear();
}

void ServerWidget::openTempBanDialog(int pId)
{
#ifdef QT5
    int time = QInputDialog::getInt(this, "Temp Ban " + server->name(pId), "Input the amount of minutes that you want to ban " + server->name(pId), 1, 1, 3600);
#else
    int time = QInputDialog::getInteger(this, "Temp Ban " + server->name(pId), "Input the amount of minutes that you want to ban " + server->name(pId), 1, 1, 3600);
#endif
    server->tempBan(pId, 0, time);
}

void ServerWidget::openPluginConfig()
{
    QAction *ac = dynamic_cast<QAction*>(sender());

    ServerPlugin *s = server->pluginManager->plugin(ac->text());

    if (s) {
        QWidget *config = s->getConfigurationWidget();

        if (config) {
            config->setAttribute(Qt::WA_DeleteOnClose);
            config->show();
        }
    }
}

void ServerWidget::openPlayers()
{
    PlayersWindow *w = new PlayersWindow(0, server->playerDeleteDays());

    w->show();

    connect(w, SIGNAL(authChanged(QString,int)), server, SLOT(changeAuth(QString, int)));
    connect(w, SIGNAL(banned(QString)), server, SLOT(banName(QString)));
}

void ServerWidget::openPluginManager()
{
    PluginManagerWidget *w = new PluginManagerWidget(*server->pluginManager);

    w->show();

    connect(w, SIGNAL(pluginListChanged()), this, SIGNAL(menuBarChanged()));
    connect(w, SIGNAL(error(QString)), server, SLOT(printLine(QString));
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
    connect(w, SIGNAL(privacyChanged(bool)), server, SLOT(regPrivacyChanged(bool)));
    connect(w, SIGNAL(announcementChanged(QString)), server, SLOT(announcementChanged(QString)));
    connect(w, SIGNAL(logSavingChanged(bool)), server, SLOT(logSavingChanged(bool)));
    connect(w, SIGNAL(inactivePlayersDeleteDaysChanged(int)), server, SLOT(inactivePlayersDeleteDaysChanged(int)));
    connect(w, SIGNAL(mainChanChanged(QString)), server, SLOT(mainChanChanged(QString)));
    connect(w, SIGNAL(latencyChanged(bool)), server, SLOT(TCPDelayChanged(bool)));
    connect(w, SIGNAL(safeScriptsChanged(bool)), server, SLOT(safeScriptsChanged(bool)));
    connect(w, SIGNAL(overactiveToggleChanged(bool)), server, SLOT(overactiveToggleChanged(bool)));
    connect(w, SIGNAL(proxyServersChanged(QString)), server, SLOT(proxyServersChanged(QString)));
    connect(w, SIGNAL(serverPasswordChanged(QString)), server, SLOT(serverPasswordChanged(QString)));
    connect(w, SIGNAL(usePasswordChanged(bool)), server, SLOT(usePasswordChanged(bool)));

    connect(w, SIGNAL(destroyed()), SLOT(loadGuiSettings()));
}

void ServerWidget::openModsWindow()
{
    ModsWindow *w = new ModsWindow();

    w->show();

    connect(w, SIGNAL(modChanged(QString)), server, SLOT(changeDbMod(QString)));
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


QScrollDownTextBrowser * ServerWidget::mainchat()
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
