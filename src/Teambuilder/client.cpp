#include "../Utilities/functions.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../PokemonInfo/teamholder.h"

#include "client.h"
#include "mainwindow.h"
#include "logmanager.h"
#include "findbattledialog.h"
#include "Teambuilder/teambuilder.h"
#include "battlewindow.h"
#include "basebattlewindow.h"
#include "pmsystem.h"
#include "controlpanel.h"
#include "ranking.h"
#include "poketextedit.h"
#include "channel.h"
#include "theme.h"
#include "soundconfigwindow.h"
#include "challengedialog.h"
#include "tieractionfactory.h"
#include "pluginmanager.h"
#include "plugininterface.h"
#include "loadwindow.h"
#ifdef QT5
#include <QCompleter>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSlider>
#include <QSplitter>
#endif

Client::Client(PluginManager *p, TeamHolder *t, const QString &url , const quint16 port) : myteam(t), findingBattle(false), url(url), port(port), myrelay(new Analyzer()), pluginManager(p)
{
    exitWarning = globals.value("Client/ShowExitWarning").toBool(); // initiate, to show exit warning or not
    flashingToggled = !globals.contains("Client/Flashing") ? true : globals.value("Client/Flashing").toBool();
    failedBefore = false;
    waitingOnSecond = false;
    top = NULL;
    isConnected = true;
    loggedIn = false;
    _mid = -1;
    selectedChannel = -1;
    setAttribute(Qt::WA_DeleteOnClose, true);
    myteambuilder = NULL;

    /* different events */
    eventlist << "PlayerEvents/ShowIdle" << "PlayerEvents/ShowBattle" << "PlayerEvents/ShowChannel" << "PlayerEvents/ShowTeam";

    QHBoxLayout *h = new QHBoxLayout(this);
    QSplitter *s = new QSplitter(Qt::Horizontal);
    h->addWidget(s);
    s->setChildrenCollapsible(false);

    QTabWidget *mytab = new QTabWidget();

    mytab->setMovable(true);
    mytab->addTab(playersW = new QStackedWidget(), tr("Players"));
    mytab->addTab(battlesW = new QStackedWidget(), tr("Battles"));
    mytab->setObjectName("playersWidget");
    QWidget *channelContainer = new QWidget();
    mytab->addTab(channelContainer, tr("Channels"));
    QGridLayout *containerLayout = new QGridLayout(channelContainer);
    channels = new QListWidget();
    channels->setIconSize(QSize(24,24));
    channels->setContextMenuPolicy(Qt::CustomContextMenu);
    chatot = Theme::Icon("activechannel");
    greychatot = Theme::Icon("idlechannel");
    containerLayout->addWidget(channels, 0, 0, 1, 2);
    containerLayout->addWidget(new QLabel(tr("Join: ")), 1, 0);
    containerLayout->addWidget(channelJoin = new QLineEdit(), 1, 1);
    QNickValidator *val = new QNickValidator(channelJoin);
    channelJoin->setValidator(val);
    QCompleter *cpl = new QCompleter(channels->model(), channelJoin);
    channelJoin->setCompleter(cpl);

    connect(channels, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(itemJoin(QListWidgetItem*)));
    connect(channelJoin, SIGNAL(returnPressed()), this, SLOT(lineJoin()));
    connect(channels, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showChannelsContextMenu(QPoint)));

    s->addWidget(mytab);

    QWidget *container = new QWidget();
    s->addWidget(container);

    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setMargin(0);

    layout->addWidget(server_announcement = new SmallPokeTextEdit());
    server_announcement->setObjectName("Announcement");
    server_announcement->setOpenExternalLinks(true);
    server_announcement->hide();
    server_announcement->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(mainChat = new QExposedTabWidget(), 100);
    mainChat->setObjectName("MainChat");
    mainChat->setMovable(true);
    mainChat->setTabsClosable(true);

    layout->addWidget(myline = new QIRCLineEdit());
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(findMatch = new QPushButton(tr("&Find Battle")));
    buttonsLayout->addWidget(myregister = new QPushButton(tr("&Register")));
    buttonsLayout->addWidget(myexit = new QPushButton(tr("&Exit")));
    buttonsLayout->addWidget(mysender = new QPushButton(tr("&Send")));

    findMatch->setObjectName("FindBattle");
    myregister->setObjectName("Register");
    myexit->setObjectName("Exit");
    mysender->setObjectName("Send");

    QPalette pal = palette();
    pal.setColor(QPalette::AlternateBase, Qt::blue);
    pal.setColor(QPalette::Base, Qt::blue);
    setPalette(pal);
    myregister->setDisabled(true);
    mynick = t->name();

    s->setSizes(QList<int>() << 200 << 800);

    connect(mainChat, SIGNAL(tabCloseRequested(int)), SLOT(leaveChannelR(int)));
    connect(mainChat, SIGNAL(currentChanged(int)), SLOT(firstChannelChanged(int)));
    connect(myexit, SIGNAL(clicked()), SLOT(showExitWarning()));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendText()));
    connect(mysender, SIGNAL(clicked()), SLOT(sendText()));
    connect(myregister, SIGNAL(clicked()), SLOT(sendRegister()));
    connect(findMatch, SIGNAL(clicked()), SLOT(openBattleFinder()));

    initRelay();

    relay().connectTo(url, port);

    const char * statuses[] = {"avail", "away", "battle", "ignore"};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < LastStatus; j++) {
            statusIcons << Theme::Icon(QString("%1%2").arg(statuses[j]).arg(i));
        }
    }

    QTimer *tim = new QTimer(this);

    connect(tim, SIGNAL(timeout()), SLOT(fadeAway()));
    /* Every 2 minutes */
    tim->setInterval(2*60*1000);
    tim->start();

    /* Default channel on to display messages */
    channelPlayers(-1);

    /* PM System */
    pmSystem = new PMSystem(globals.value("PMs/Tabbed").toBool()); // We leave it here for future use. :)
    pmSystem->setParent(this);
    pmSystem->setWindowFlags(Qt::Window);

    connect(this, SIGNAL(destroyed()), pmSystem, SLOT(deleteLater()));
    connect(this, SIGNAL(togglePMs(bool)), pmSystem, SLOT(togglePMs(bool)));
    connect(this, SIGNAL(PMDisconnected(bool)), pmSystem, SLOT(PMDisconnected(bool)));

    connect(pmSystem, SIGNAL(notification(QString,QString)), MainEngine::inst, SLOT(showMessage(QString,QString)));

    /* move player tab to right if user has selected it
     * this needs to be done at the end of this function to work properly
     */
    if (globals.value("Client/UserListAtRight").toBool()) {
        s->addWidget(mytab);
    }
    sortCBN = globals.value("Client/SortChannelsByName").toBool();
    if(sortCBN) {
        sortChannels();
    }
    pmsTabbed = globals.value("PMs/Tabbed").toBool();
    pmReject = globals.value("PMs/RejectIncoming").toBool();

    pluginManager->launchClient(this);
    foreach(OnlineClientPlugin *pl, plugins) {
        pl->clientStartUp();
    }
}

Client::~Client()
{
    foreach(OnlineClientPlugin *pl, plugins) {
        pl->clientShutDown();
    }

    pluginManager->quitClient(this);

    relay().logout();
    writeSettings(this);
}

void Client::initRelay()
{
    Analyzer *relay = &this->relay();
    connect(relay, SIGNAL(connectionError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(relay, SIGNAL(protocolError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(relay, SIGNAL(connected()), SLOT(connected()));
    connect(relay, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(relay, SIGNAL(messageReceived(QString)), SLOT(printLine(QString)));
    connect(relay, SIGNAL(htmlMessageReceived(QString)), SLOT(printHtml(QString)));
    connect(relay, SIGNAL(channelMessageReceived(QString,int,bool)), SLOT(printChannelMessage(QString, int, bool)));
    connect(relay, SIGNAL(playerReceived(PlayerInfo)), SLOT(playerReceived(PlayerInfo)));
    connect(relay, SIGNAL(playerLogin(PlayerInfo, QStringList)), SLOT(playerLogin(PlayerInfo, QStringList)));
    connect(relay, SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(relay, SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(relay, SIGNAL(battleStarted(int, Battle, TeamBattle, BattleConfiguration)),
            SLOT(battleStarted(int, Battle, TeamBattle, BattleConfiguration)));
    connect(relay, SIGNAL(teamApproved(QStringList)), SLOT(tiersReceived(QStringList)));
    connect(relay, SIGNAL(battleStarted(int, Battle)), SLOT(battleStarted(int, Battle)));
    connect(relay, SIGNAL(battleFinished(int, int,int,int)), SLOT(battleFinished(int, int,int,int)));
    connect(relay, SIGNAL(battleMessage(int, QByteArray)), this, SLOT(battleCommand(int, QByteArray)));
    connect(relay, SIGNAL(passRequired(QByteArray)), SLOT(askForPass(QByteArray)));
    connect(relay, SIGNAL(serverPassRequired(QByteArray)), SLOT(serverPass(QByteArray)));
    connect(relay, SIGNAL(notRegistered(bool)), myregister, SLOT(setEnabled(bool)));
    connect(relay, SIGNAL(playerKicked(int,int)),SLOT(playerKicked(int,int)));
    connect(relay, SIGNAL(playerBanned(int,int)),SLOT(playerBanned(int,int)));
    connect(relay, SIGNAL(playerTempBanned(int,int,int)), SLOT(playerTempBanned(int,int,int)));
    connect(relay, SIGNAL(PMReceived(int,QString)), SLOT(PMReceived(int,QString)));
    connect(relay, SIGNAL(awayChanged(int, bool)), SLOT(awayChanged(int, bool)));
    connect(relay, SIGNAL(ladderChanged(int,bool)), SLOT(ladderChanged(int,bool)));
    connect(relay, SIGNAL(spectatedBattle(int,BattleConfiguration)), SLOT(watchBattle(int,BattleConfiguration)));
    connect(relay, SIGNAL(spectatingBattleMessage(int,QByteArray)), SLOT(spectatingBattleMessage(int , QByteArray)));
    connect(relay, SIGNAL(spectatingBattleFinished(int)), SLOT(stopWatching(int)));
    connect(relay, SIGNAL(versionDiff(ProtocolVersion, int)), SLOT(versionDiff(ProtocolVersion, int)));
    connect(relay, SIGNAL(serverNameReceived(QString)), SLOT(serverNameReceived(QString)));
    connect(relay, SIGNAL(tierListReceived(QByteArray)), SLOT(tierListReceived(QByteArray)));
    connect(relay, SIGNAL(announcement(QString)), SLOT(announcementReceived(QString)));
    connect(relay, SIGNAL(channelsListReceived(QHash<qint32,QString>)), SLOT(channelsListReceived(QHash<qint32,QString>)));
    connect(relay, SIGNAL(channelPlayers(int,QVector<qint32>)), SLOT(channelPlayers(int,QVector<qint32>)));
    connect(relay, SIGNAL(channelCommandReceived(int,int,DataStream*)), SLOT(channelCommandReceived(int,int,DataStream*)));
    connect(relay, SIGNAL(addChannel(QString,int)), SLOT(addChannel(QString,int)));
    connect(relay, SIGNAL(removeChannel(int)), SLOT(removeChannel(int)));
    connect(relay, SIGNAL(channelNameChanged(int,QString)), SLOT(channelNameChanged(int,QString)));
    connect(relay, SIGNAL(reconnectPassGiven(QByteArray)), SLOT(setReconnectPass(QByteArray)));
    connect(relay, SIGNAL(reconnectSuccess()), SLOT(cleanData()));
    connect(relay, SIGNAL(reconnectFailure(int)), SLOT(onReconnectFailure(int)));
}

class TierActionFactoryTeams : public TierActionFactory
{
public:
    TierActionFactoryTeams(TeamHolder *t) {
        team = t;
    }

    QList<QAction* > createTierActions(QMenu *m, QObject *o, const char *slot) {
        QList<QAction* > ret;
        for (int i = 0; i < team->officialCount(); i++) {
            //Todo: subclass QAction to show the 6 pokemons in the QAction?
            ret.push_back(m->addAction(team->team(i).name(), o, slot));
            ret.back()->setCheckable(true);
            ret.back()->setProperty("team", i);
        }

        return ret;
    }

    TeamHolder *team;
};

int Client::ownAuth() const
{
    return auth(ownId());
}

int Client::auth(int id) const
{
    return player(id).auth;
}

int Client::ownId() const
{
    return _mid;
}

int Client::currentChannel() const
{
    if (mychannels.size() == 0) {
        return -1;
    }

    return m_channelByNames[mainChat->tabText(mainChat->currentIndex()).toLower()];
}

QIcon Client::statusIcon(int auth, Status status) const
{
    if (auth > 3 || auth < 0)
        auth = 0;
    return statusIcons[auth*LastStatus+status];
}

QStringList const& Client::eventSettings() const {
    return eventlist;
}

void Client::battleListActivated(QTreeWidgetItem *it)
{
    QIdTreeWidgetItem *i;
    if ( (i=dynamic_cast<QIdTreeWidgetItem*>(it)) ) {
        watchBattleRequ(i->id());
    }
}

void Client::firstChannelChanged(int tabindex)
{
    int chanid = m_channelByNames.value(mainChat->tabText(tabindex).toLower());

    if (!hasChannel(chanid)) {
        myline->setPlayers(0);
        return;
    }

    Channel *c = channel(chanid);

    playersW->setCurrentWidget(c->playersWidget());
    battlesW->setCurrentWidget(c->battlesWidget());
    c->state = Channel::Inactive;

    /* Restores the black color of a possibly activated channel */
    mainChat->tabBar()->setTabTextColor(tabindex, mainChat->tabBar()->palette().text().color());
    myline->setPlayers(c->playersWidget()->model());
}

void Client::channelsListReceived(const QHash<qint32, QString> &channelsL)
{
    channels->clear();

    m_channelNames = channelsL;
    QHashIterator<qint32, QString> it (m_channelNames);

    while (it.hasNext()) {
        it.next();

        QString lower = it.value().toLower();

        if (m_channelByNames.contains(lower) && it.key() != m_channelByNames.value(lower)) {
            changeChannelId(m_channelByNames.value(lower), it.key());
        }
        m_channelByNames.insert(lower, it.key());

        /* We would have a default screen open */
        if (mychannels.contains(it.key())) {
            mainChat->setTabText(mainChat->indexOf(mychannels.value(it.key())->mainChat()), it.value());
            channelNameChanged(it.key(),it.value());
        }

        if (hasChannel(it.key()))
            channels->addItem(new QIdListWidgetItem(it.key(), chatot, it.value()));
        else
            channels->addItem(new QIdListWidgetItem(it.key(), greychatot, it.value()));
    }
}

void Client::sortChannelsToggle(bool newvalue)
{
    globals.setValue("Client/SortChannelsByName", newvalue);

    sortCBN = newvalue;
    sortChannels();
}

void Client::sortChannels() {
    if(sortCBN) {
        channels->sortItems();
    }
    channels->setSortingEnabled(sortCBN);
}

void Client::channelPlayers(int chanid, const QVector<qint32> &ids)
{
    if (hasChannel(chanid)) {
        /* Then for some reason we aren't synchronized, but let's get it smooth */
        Channel *c = mychannels.value(chanid);

        c->receivePlayerList(ids);

        return;
    }

    Channel *c;

    /* Test if there is a channel or not. If no channel, the already in-place channel space is used
      for the new channels */
    if (mychannels.contains(-1))  {
        c = mychannels[-1];
        mychannels[chanid] = c;
        mychannels.remove(-1);
        c->setId(chanid);

        channelNameChanged(chanid, m_channelNames.take(chanid));

        // set tab complete for first chan
        myline->setPlayers(c->playersWidget()->model());
    } else {
        c = new Channel(channelName(chanid), chanid, this);
        mychannels[chanid] = c;

        playersW->addWidget(c->playersWidget());
        mainChat->addTab(c->mainChat(), c->name());
        battlesW->addWidget(c->battlesWidget());

        connect(c, SIGNAL(quitChannel(int)), SLOT(leaveChannel(int)));
        connect(c, SIGNAL(battleReceived2(int,Battle)), this, SLOT(battleReceived(int,Battle)));
        connect(c, SIGNAL(activated(Channel*)), this, SLOT(channelActivated(Channel*)));
        connect(c, SIGNAL(pactivated(Channel*)), this, SLOT(pingActivated(Channel*)));
    }

    for(int i =0;i < channels->count(); i++) {
        if (channels->item(i)->text() == c->name())
            channels->item(i)->setIcon(chatot);
    }

    c->receivePlayerList(ids);
}

void Client::channelActivated(Channel *c)
{
    if (c->state != Channel::Inactive) {
        return;
    }
    if (c->mainChat() == mainChat->currentWidget())
        return;
    c->state = Channel::Active;
    for (int i = 0; i < mainChat->count(); i++) {
        if (mainChat->widget(i) == c->mainChat()) {
            mainChat->tabBar()->setTabTextColor(i, Theme::Color("Client/channelTabActive"));
            break;
        }
    }
}

void Client::pingActivated(Channel *c)
{
    if (c->state == Channel::Flashed) {
        return;
    }
    if (c->mainChat() == mainChat->currentWidget())
        return;
    c->state = Channel::Flashed;
    for (int i = 0; i < mainChat->count(); i++) {
        if (mainChat->widget(i) == c->mainChat()) {
            mainChat->tabBar()->setTabTextColor(i, Theme::Color("Client/channelTabFlash"));
            break;
        }
    }
}


void Client::showChannelsContextMenu(const QPoint & point)
{
    QIdListWidgetItem *item = dynamic_cast<QIdListWidgetItem*>(channels->itemAt(point));
    if (item) {
        QSettings s;
        s.beginGroup("channelevents");
        s.beginGroup(m_channelNames.value(item->id()));

        QMenu *show_events = new QMenu(this);
        mychanevents.clear();
        QAction *action;

        bool found = false;
        foreach (QString str, eventSettings()) {
            if (s.contains(str)) {
                found = true;
                break;
            }
        }

        if (found) {
            action = show_events->addAction(tr("Custom settings"));
            action->setEnabled(false);
            action = show_events->addAction(tr("Use global"));
            createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
            connect(action, SIGNAL(triggered()), SLOT(deleteCustomEvents()));
            createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        } else {
            action = show_events->addAction(tr("Global settings"));
            action->setEnabled(false);
        }

        show_events->addSeparator();

        action = show_events->addAction(tr("Enable all events"));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered()), SLOT(enablePlayerEvents()));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);

        action = show_events->addAction(tr("Disable all events"));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered()), SLOT(disablePlayerEvents()));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);

        show_events->addSeparator();

        action = show_events->addAction(tr("Enable idle events"));
        action->setCheckable(true);
        action->setChecked(s.value("PlayerEvents/ShowIdle",
                                   globals.value("PlayerEvents/ShowIdle").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showIdleEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Enable battle events"));
        action->setCheckable(true);
        action->setChecked(s.value("PlayerEvents/ShowBattle",
                                   globals.value("PlayerEvents/ShowBattle").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showBattleEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Enable channel events"));
        action->setCheckable(true);
        action->setChecked(s.value("PlayerEvents/ShowChannel",
                                   globals.value("PlayerEvents/ShowChannel").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showChannelEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Enable name change events"));
        action->setCheckable(true);
        action->setChecked(s.value("PlayerEvents/ShowTeam",
                                   globals.value("PlayerEvents/ShowTeam").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showTeamEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        show_events->addSeparator();

        QString name = channelName(item->id());
        QString ip = relay().getIp();

        if(defaultChannel() != name)  {
            action = show_events->addAction(tr("Auto-join"));
            action->setCheckable(true);
            action->setChecked(globals.value(QString("AutoJoinChannels/%1").arg(ip)).toStringList().contains(name));
            createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
            connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleAutoJoin(bool)));
            createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
            mychanevents.push_back(action);
        }
        action = show_events->addAction(tr("Default Channel"));
        action->setCheckable(true);
        action->setChecked(defaultChannel() == name);
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleDefaultChannel(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Ignore Global Messages"));
        action->setCheckable(true);
        action->setChecked(globals.value(QString("GlobalMessageChannels/%1").arg(ip)).toStringList().contains(name));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleGlobalMessage(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        show_events->exec(channels->mapToGlobal(point));
    }
}

QStringList Client::autojoinChannels()
{
    return globals.value(QString("AutoJoinChannels/%1").arg(relay().getIp())).toStringList();
}

QStringList Client::myChannels()
{
    QStringList channels;
    foreach(Channel *c, mychannels){
        channels.push_back(c->name());
    }
    return channels;
}

QString Client::defaultChannel()
{
    return globals.value(QString("DefaultChannels/%1").arg(relay().getIp())).toString();
}

void Client::addChannel(const QString &name, int id)
{
    m_channelNames.insert(id, name);
    if (m_channelByNames.contains(name.toLower()) && id != m_channelByNames.value(name.toLower())) {
        changeChannelId(m_channelByNames.value(name.toLower()), id);
    }
    m_channelByNames.insert(name.toLower(),id);
    channels->addItem(new QIdListWidgetItem(id, greychatot, name));
}

void Client::channelNameChanged(int id, const QString &name)
{
    QString old = channelName(id);
    m_channelNames[id] = name;
    m_channelByNames.remove(old.toLower());
    m_channelByNames[name.toLower()] = id;

    for(int i = 0; i < channels->count(); i++) {
        if (channels->item(i)->text() == old) {
            channels->item(i)->setText(name);
        }
    }

    for (int i = 0; i < mainChat->count(); i++) {
        if (mainChat->tabText(i) == old) {
            mainChat->setTabText(i, name);
        }
    }

    if (hasChannel(id)) {
        channel(id)->setName(name);
    }
}

void Client::removeChannel(int id)
{
    for (int i = 0; i < channels->count(); i++)  {
        QIdListWidgetItem *item = dynamic_cast<QIdListWidgetItem*>(channels->item(i));

        if (item->id() == id) {
            delete channels->takeItem(i);
        }
    }

    QString chanName = m_channelNames.take(id);
    m_channelByNames.remove(chanName.toLower());
}

void Client::leaveChannelR(int index)
{
    if (mychannels.size() == 1)
        return;

    if (!m_channelByNames.contains(mainChat->tabText(index).toLower())) {
        foreach(Channel *c, mychannels) {
            if (c->name() == mainChat->tabText(index)) {
                c->makeReadyToQuit();
                int id = c->id();

                leaveChannel(id);
                break;
            }
        }
        return;
    }

    int id = channelId(mainChat->tabText(index));

    if (channel(id)->isReadyToQuit()) {
        leaveChannel(id);
    } else {
        channel(id)->makeReadyToQuit();
        relay().notify(NetworkCli::LeaveChannel, qint32(id));
    }
}

void Client::leaveChannel(int id)
{
    if (!hasChannel(id))
        return;

    Channel *c = channel(id);

    if (!c->isReadyToQuit()) {
        c->makeReadyToQuit();
        return;
    }

    QString name = channel(id)->name();

    for(int i = 0; i < channels->count(); i++) {
        if (channels->item(i)->text() == name) {
            channels->item(i)->setIcon(greychatot);
        }
    }

    int index = -1;
    for(int i = 0; i < mainChat->count(); i++) {
        if (mainChat->tabText(i).toLower() == name.toLower())
        {
            index = i;
            break;
        }
    }

    mainChat->removeTab(index);
    playersW->removeWidget(c->playersWidget());
    battlesW->removeWidget(c->battlesWidget());

    mychannels.remove(id);

    c->deleteLater();
}

void Client::activateChannel(const QString& text) {
    for (int i = 0; i < mainChat->count(); ++i) {
        if (0 == text.compare(mainChat->tabText(i))) {
            mainChat->setCurrentIndex(i);
            return;
        }
    }
}

void Client::cleanData()
{
    foreach(Channel *c, mychannels) {
        c->cleanData();
    }
}

void Client::join(const QString& text)
{
    if (m_channelByNames.contains(text.toLower())) {
        int id = channelId(text.toLower());

        if (hasChannel(id)) {
            /* No use joining the same channel twice */
            return;
        }
    }

    relay().notify(NetworkCli::JoinChannel, text);
}


void Client::itemJoin(QListWidgetItem *it)
{
    QString text = it->text().trimmed();
    channelJoin->setText(text);

    lineJoin();
}

void Client::lineJoin()
{
    QString text = channelJoin->text().trimmed();

    if (text.length() == 0) {
        return;
    }

    join(text);
}

void Client::watchBattleOf(int player)
{
    if (battles.empty() || battles.contains(0)) {
        //Old Server, or bad luck, assuming old server anyway
        watchBattleRequ(player);
        return;
    }

    QHashIterator<int, Battle> h(battles);

    while (h.hasNext()) {
        h.next();

        if (h.value().id1 == player || h.value().id2 == player) {
            watchBattleRequ(h.key());
            return;
        }
    }
}

void Client::watchBattleRequ(int id)
{
    relay().notify(NetworkCli::SpectateBattle, qint32(id), Flags(true));
}

void Client::kick(int p) {
    relay().notify(NetworkCli::PlayerKick, qint32(p));
}

void Client::ban(int p) {
    relay().notify(NetworkCli::PlayerBan, qint32(p));
}

void Client::tempban(int p, int time) {
    relay().notify(NetworkCli::PlayerBan, qint32(p), qint32(time));
}

void Client::pmcp(QString p) {
    int pm = id(p);
    if (!mypms.contains(pm)) {
        startPM(pm);
    }
    return;
}

void Client::startPM(int id)
{
    if (id == this->id(ownName()) || !playerExist(id)) {
        return;
    }

    if (mypms.contains(id) || pmSystem->myPMWindows.contains(id)) {
        if (!pmSystem->isVisible()) {
            // When using non-tabbed PMs, a blank PM window will sometimes come up when a message is received
            // This is why we check if PMs are tabbed...
            if (pmsTabbed) {
                pmSystem->show();
            }
        }
        return;
    }

    PMStruct *p = new PMStruct(id, ownName(), name(id), "", ownAuth());

    pmSystem->startPM(p);
    pmSystem->flash(p);

    connect(p, SIGNAL(challengeSent(int)), this, SLOT(seeInfo(int)));
    connect(p, SIGNAL(messageEntered(int,QString)), this, SLOT(sendPM(int,QString)));
    connect(p, SIGNAL(messageEntered(int,QString)), this, SLOT(registerPermPlayer(int)));
    connect(p, SIGNAL(destroyed(int,QString)), this, SLOT(removePM(int,QString)));
    connect(p, SIGNAL(controlPanel(int)), this, SLOT(controlPanel(int)));
    connect(p, SIGNAL(ignore(int,bool)), this, SLOT(ignore(int, bool)));
    connect(this, SIGNAL(destroyed()), p, SLOT(deleteLater()));

    mypms[id] = p;
}

void Client::registerPermPlayer(int id)
{
    pmedPlayers.insert(id);
}

void Client::goAway(int away)
{
    relay().notify(NetworkCli::OptionsChange, Flags(ladder->isChecked()  + (away << 1)));
    goaway->setChecked(away);
    globals.setValue("Client/Idle", away);
}

void Client::showTimeStamps(bool b)
{
    globals.setValue("Client/ShowTimestamps", b);
    showTS = b;
}

void Client::showTimeStamps2(bool b)
{
    globals.setValue("PMs/ShowTimestamps", b);
}

void Client::showSeconds(bool b)
{
    globals.setValue("PMs/ShowSeconds", b);
}

void Client::toggleIncomingPM(bool b)
{
    globals.setValue("PMs/RejectIncoming", b);
    pmReject = b;
}

void Client::togglePMTabs(bool b)
{
    globals.setValue("PMs/Tabbed", b);
    pmsTabbed = b;
    emit togglePMs(b);
}

void Client::togglePMNotifications(bool notify)
{
    globals.setValue("PMs/Notifications", notify);
    emit pmNotificationsChanged(notify);
}

void Client::togglePMLogs(bool b) {
    LogManager::obj()->changeLogSaving(PMLog, b);
}

void Client::ignoreServerVersion(bool b)
{
    QString key  = QString("ignore_version_%1_%2").arg(serverVersion.version).arg(serverVersion.subversion);
    if (b) {
        globals.setValue(key, true);
    } else {
        globals.remove(key);
    }
}

void Client::enableLadder(bool b)
{
    globals.setValue("Client/EnableLadder", b);

    relay().notify(NetworkCli::OptionsChange, Flags(b | (goaway->isChecked() << 1)));
    ladder->setChecked(b);
}

void Client::setChannelSelected(int id)
{
    selectedChannel = id;
}

void Client::enablePlayerEvents()
{
    foreach(QAction *event, selectedChannel == -1 ? myevents : mychanevents) {
        event->setChecked(true);
    }
    showIdleEvents(true);
    showBattleEvents(true);
    showChannelEvents(true);
    showTeamEvents(true);
}

void Client::disablePlayerEvents()
{
    foreach(QAction *event, selectedChannel == -1 ? myevents : mychanevents) {
        event->setChecked(false);
    }
    showIdleEvents(false);
    showBattleEvents(false);
    showChannelEvents(false);
    showTeamEvents(false);

}

void Client::deleteCustomEvents() {
    if (selectedChannel != -1) {
        QSettings s;
        s.beginGroup("channelevents");
        s.beginGroup(channelName(selectedChannel));
        s.remove(""); // removes all settings
    }
    if (mychannels.contains(selectedChannel)) {
        mychannels.value(selectedChannel)->resetEvents();
    }
}

int Client::getEventsForChannel(QString const& channel)
{
    QSettings s;
    s.beginGroup("channelevents");
    if (s.childGroups().indexOf(channel) > -1) {
        s.beginGroup(channel);
        int ret = 0;
        int event = 1; /* trusts that eventSettings() returns
                the list in order */
        foreach (QString str, eventSettings()) {
            if (s.value(str).toBool()) {
                ret |= event;
            } else {
                ret &= ~event;
            }
            event *= 2;
        }
        return ret;
    } else {
        return -1;
    }
}

void Client::showPlayerEvents(bool b, int event, QString option)
{
    QSettings s;
    if (selectedChannel != -1) {
        s.beginGroup("channelevents");
        s.beginGroup(channelName(selectedChannel));
    }
    s.setValue(option, b);
    if (selectedChannel != -1) {
        if (mychannels.contains(selectedChannel)) {
            if (b) {
                mychannels.value(selectedChannel)->addEvent(event);
            } else {
                mychannels.value(selectedChannel)->removeEvent(event);
            }
        }
        /* make sure every setting is saved */
        foreach (QString str, eventSettings()) {
            if (!s.contains(str)) {
                s.setValue(str, globals.value(str).toBool());
            }
        }
    } else {
        if (b) {
            showPEvents |= event;
        } else {
            showPEvents &= ~event;
        }
    }
}

void Client::showIdleEvents(bool b)
{
    showPlayerEvents(b, IdleEvent, "PlayerEvents/ShowIdle");
}

void Client::showBattleEvents(bool b)
{
    showPlayerEvents(b, BattleEvent, "PlayerEvents/ShowBattle");
}

void Client::showChannelEvents(bool b)
{
    showPlayerEvents(b, ChannelEvent, "PlayerEvents/ShowChannel");
}

void Client::showTeamEvents(bool b)
{
    showPlayerEvents(b, TeamEvent, "PlayerEvents/ShowTeam");
}

void Client::toggleAutoJoin(bool autojoin)
{
    QStringList AutoJoinChannels = globals.value(QString("AutoJoinChannels/%1").arg(relay().getIp())).toStringList();
    if(autojoin) {
        AutoJoinChannels.push_back(channelName(selectedChannel));
    } else {
        AutoJoinChannels.removeOne(channelName(selectedChannel));
    }
    globals.setValue(QString("AutoJoinChannels/%1").arg(relay().getIp()), AutoJoinChannels);
}

void Client::toggleDefaultChannel(bool d)
{
    toggleAutoJoin(false);
    if (d) {
        globals.setValue(QString("DefaultChannels/%1").arg(relay().getIp()), channelName(selectedChannel));
    } else {
        globals.remove(QString("DefaultChannels/%1").arg(relay().getIp()));
    }
}

void Client::toggleGlobalMessage(bool gmessage)
{
    QStringList globalMessageIgnore = globals.value(QString("GlobalMessageChannels/%1").arg(relay().getIp())).toStringList();
    if(gmessage) {
        globalMessageIgnore.push_back(channelName(selectedChannel));
    } else {
        globalMessageIgnore.removeOne(channelName(selectedChannel));
    }
    globals.setValue(QString("GlobalMessageChannels/%1").arg(relay().getIp()), globalMessageIgnore);
}

bool Client::ignoringGlobalMessage(const QString &channelName) { /* so client scripts can detect and stop the message */
    if (globals.value(QString("GlobalMessageChannels/%1").arg(relay().getIp())).toStringList().contains(channelName)) {
        return true;
    }
    return false;
}

void Client::seeRanking(int id)
{
    if (!playerExist(id)) {
        return;
    }

    if (myRanking) {
        myRanking->raise();
        myRanking->activateWindow();
        return;
    }

    myRanking = new RankingDialog(tierList);

    myRanking->setParent(this);
    myRanking->setWindowFlags(Qt::Window);
    myRanking->show();

    connect(myRanking, SIGNAL(lookForPlayer(QString,QString)), &relay(), SLOT(getRanking(QString,QString)));
    connect(myRanking, SIGNAL(lookForPage(QString,int)), &relay(), SLOT(getRanking(QString,int)));
    connect(&relay(), SIGNAL(rankingReceived(QString,int)), myRanking, SLOT(showRank(QString,int)));
    connect(&relay(), SIGNAL(rankingStarted(int,int,int)), myRanking, SLOT(startRanking(int,int,int)));

    myRanking->init(name(id), tier(id));
}

void Client::controlPanel(int id)
{
    if (!playerExist(id)) {
        return;
    }

    if (myCP) {
        myCP->raise();
        myCP->activateWindow();
        return;
    }

    myCP = new ControlPanel(ownAuth(), UserInfo(name(id), UserInfo::Online, auth(id)));
    myCP->setParent(this);
    myCP->setWindowFlags(Qt::Window);
    myCP->show();

    connect(myCP, SIGNAL(getUserInfo(QString)), &relay(), SLOT(getUserInfo(QString)));
    connect(&relay(), SIGNAL(userInfoReceived(UserInfo)), this, SLOT(setPlayer(UserInfo)));
    connect(&relay(), SIGNAL(userAliasReceived(QString)), myCP, SLOT(addAlias(QString)));
    connect(this, SIGNAL(userInfoReceived(UserInfo)), myCP, SLOT(setPlayer(UserInfo)));
    connect(&relay(), SIGNAL(banListReceived(QString,QString,QDateTime)), myCP, SLOT(addNameToBanList(QString, QString, QDateTime)));
    connect(myCP, SIGNAL(getBanList()), &relay(), SLOT(getBanList()));
    connect(myCP, SIGNAL(banRequested(QString)), SLOT(requestBan(QString)));
    connect(myCP, SIGNAL(unbanRequested(QString)), &relay(), SLOT(CPUnban(QString)));
    connect(myCP, SIGNAL(tempBanRequested(QString, int)),this, SLOT(requestTempBan(QString,int)));
    connect(myCP, SIGNAL(pmcp(QString)), SLOT(pmcp(QString)));
}

void Client::openBattleFinder()
{
    if (findingBattle) {
        cancelFindBattle(true);
        return;
    }

    if (myBattleFinder) {
        myBattleFinder->raise();
        return;
    }

    myBattleFinder = new FindBattleDialog(this);
    myBattleFinder->setTeam(team());
    myBattleFinder->show();

    connect(myBattleFinder, SIGNAL(findBattle(FindBattleData)), SLOT(findBattle(FindBattleData)));
}

void Client::findBattle(const FindBattleData&data)
{
    relay().notify(NetworkCli::FindBattle,data);
    findMatch->setText(tr("&Cancel Find Battle"));
    findingBattle = true;
}

void Client::setPlayer(const UserInfo &ui)
{
    if (id(ui.name) == -1) {
        emit userInfoReceived(ui);
    } else {
        UserInfo ui2 (ui);
        ui2.flags |= UserInfo::Online;
        emit userInfoReceived(ui2);
    }
}

void Client::sendPM(int id, QString pm) {
    if (call("beforePMSent(int,QString)", id, pm)) {
        relay().sendPM(id, pm);
        call("afterPMSent(int,QString)", id, pm);
    }
}

void Client::PMReceived(int id, QString pm)
{
    if (!call("beforePMReceived(int,QString)",id,pm)) {
        return;
    }

    if(pmReject && !mypms.contains(id) && auth(id) == 0) {
        relay().sendPM(id, "This player is rejecting incoming PMs.");
        return;
    }

    if(!playerExist(id) || myIgnored.contains(id)) {
        return;
    }

    startPM(id);

    if(mypms.contains(id) && !mypms[id]->isVisible()) {
        //mypms[id]->show();
    }

    registerPermPlayer(id);
    /* sometimes the line won't be printed, so we do this check */
    if (!mypms.contains(id)) {
        mypms[id]->printLine(pm);
    } else {
        pmSystem->myPMWindows[id]->printLine(pm);
    }
    call("afterPMReceived(int,QString)",id,pm);
}


void Client::closePM(int id)
{
    mypms[id]->close();
}

void Client::removePM(int id, const QString name)
{
    mypms.remove(id);
    disabledpms.remove(name);
}

void Client::loadTeam()
{
    LoadWindow *w = new LoadWindow(this, getTierList(), mynick);
    w->setAttribute(Qt::WA_DeleteOnClose, true);
    w->show();

    connect(w, SIGNAL(teamLoaded(TeamHolder)), SLOT(changeTeam(TeamHolder)));
}

void Client::sendText()
{
    if (currentChannel() == -1)
        return;

    int cid = currentChannel();

    QString text = myline->text().trimmed();
    if (text.length() > 0) {
        QStringList s = text.split('\n');
        foreach(QString s1, s) {
            if (s1.length() > 0 && call("beforeSendMessage(QString,int)", s1, cid)) {
                relay().sendChanMessage(cid, s1);
            }
        }
    }

    myline->clear();
}

bool Client::hasChannel(int channelid) const
{
    return mychannels.contains(channelid);
}

Channel *Client::channel(int channelid)
{
    return mychannels.value(channelid);
}

void Client::channelCommandReceived(int command, int channel, DataStream *stream)
{
    if (!hasChannel(channel))
        return;

    this->channel(channel)->dealWithCommand(command, stream);

    /* Do not let a battle pass by! */
    if (command == NetworkCli::ChannelBattle || command == NetworkCli::BattleList) {
        battles.unite(this->channel(channel)->getBattles());
    }
}

QMenuBar * Client::createMenuBar(MainEngine *w)
{
    myevents.clear();

    top = w;
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setObjectName("MainChat");

    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    bool oldShort = globals.value("Client/UsingOldShortcuts").toBool();
    if (oldShort){
        fileMenu->addAction(tr("New &tab"), w, SLOT(openNewTab()), tr("Ctrl+N", "New tab"));
    } else {
        fileMenu->addAction(tr("New &tab"), w, SLOT(openNewTab()), tr("Ctrl+T", "New tab"));
    }
    fileMenu->addAction(tr("Close tab"), w, SLOT(closeTab()), tr("Ctrl+W", "Close tab"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Load team"),this,SLOT(loadTeam()),tr("Ctrl+L", "Load team"));
    if (oldShort) {
        fileMenu->addAction(tr("&Open TeamBuilder"),this,SLOT(openTeamBuilder()), tr("Ctrl+T", "Open teambuilder"));
    } else {
        fileMenu->addAction(tr("&Open TeamBuilder"),this,SLOT(openTeamBuilder()), tr("Ctrl+O", "Open teambuilder"));
    }
    fileMenu->addAction(tr("Open &Replay"),w,SLOT(loadReplayDialog()), tr("Ctrl+R", "Open replay"));

    w->addThemeMenu(menuBar);
    w->addStyleMenu(menuBar);

    QMenu * menuActions = menuBar->addMenu(tr("&Options"));
    goaway = menuActions->addAction(tr("&Idle"));
    goaway->setCheckable(true);
    goaway->setChecked(this->away());
    connect(goaway, SIGNAL(triggered(bool)), this, SLOT(goAwayB(bool)));

    ladder = menuActions->addAction(tr("Enable &ladder"));
    ladder->setCheckable(true);
    connect(ladder, SIGNAL(triggered(bool)), SLOT(enableLadder(bool)));
    ladder->setChecked(globals.value("Client/EnableLadder").toBool());

    QMenu* show_events = menuActions->addMenu(tr("Player events"));
    showPEvents = NoEvent;

    QAction *action;
    action = show_events->addAction(tr("Enable all events"));
    connect(action, SIGNAL(triggered()), SLOT(enablePlayerEvents()));

    action = show_events->addAction(tr("Disable all events"));
    connect(action, SIGNAL(triggered()), SLOT(disablePlayerEvents()));

    show_events->addSeparator();

    action = show_events->addAction(tr("Enable idle events"));
    action->setCheckable(true);
    action->setChecked(globals.value("PlayerEvents/ShowIdle").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showIdleEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= IdleEvent;
    } else {
        showPEvents &= ~IdleEvent;
    }
    myevents.push_back(action);

    action = show_events->addAction(tr("Enable battle events"));
    action->setCheckable(true);
    action->setChecked(globals.value("PlayerEvents/ShowBattle").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showBattleEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= BattleEvent;
    } else {
        showPEvents &= ~BattleEvent;
    }
    myevents.push_back(action);

    action = show_events->addAction(tr("Enable channel events"));
    action->setCheckable(true);
    action->setChecked(globals.value("PlayerEvents/ShowChannel").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showChannelEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= ChannelEvent;
    } else {
        showPEvents &= ~ChannelEvent;
    }
    myevents.push_back(action);

    action = show_events->addAction(tr("Enable name change events"));
    action->setCheckable(true);
    action->setChecked(globals.value("PlayerEvents/ShowTeam").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showTeamEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= TeamEvent;
    } else {
        showPEvents &= ~TeamEvent;
    }
    myevents.push_back(action);

    QAction * show_ts = menuActions->addAction(tr("Enable &timestamps"));
    show_ts->setCheckable(true);
    connect(show_ts, SIGNAL(triggered(bool)), SLOT(showTimeStamps(bool)));
    show_ts->setChecked(globals.value("Client/ShowTimestamps").toBool());
    showTS = show_ts->isChecked();

    QMenu * pmMenu = menuActions->addMenu(tr("&PM options"));

    QAction * pmsTabbedToggle = pmMenu->addAction(tr("Show PMs in tabs"));
    pmsTabbedToggle->setCheckable(true);
    connect(pmsTabbedToggle, SIGNAL(triggered(bool)), SLOT(togglePMTabs(bool)));
    pmsTabbedToggle->setChecked(globals.value("PMs/Tabbed").toBool());

    QAction * pmNotifications = pmMenu->addAction(tr("Show PM &notifications"));
    pmNotifications->setCheckable(true);
    connect(pmNotifications, SIGNAL(triggered(bool)), SLOT(togglePMNotifications(bool)));
    pmNotifications->setChecked(globals.value("PMs/Notifications").toBool());

    QAction * save_logs = pmMenu->addAction(tr("Enable logs in &PM"));
    save_logs->setCheckable(true);
    connect(save_logs, SIGNAL(triggered(bool)), SLOT(togglePMLogs(bool)));
    save_logs->setChecked(globals.value("PMs/Logged").toBool());

    QAction * show_ts2 = pmMenu->addAction(tr("Enable timestamps in PMs"));
    show_ts2->setCheckable(true);
    connect(show_ts2, SIGNAL(triggered(bool)), SLOT(showTimeStamps2(bool)));
    show_ts2->setChecked(globals.value("PMs/ShowTimestamps").toBool());

    QAction * show_sec = pmMenu->addAction(tr("Enable seconds in PMs"));
    show_sec->setCheckable(true);
    connect(show_sec, SIGNAL(triggered(bool)), SLOT(showSeconds(bool)));
    show_sec->setChecked(globals.value("PMs/ShowSeconds").toBool());

    QAction * pm_reject = pmMenu->addAction(tr("Reject incoming PMs"));
    pm_reject->setCheckable(true);
    connect(pm_reject, SIGNAL(triggered(bool)), SLOT(toggleIncomingPM(bool)));
    pm_reject->setChecked(globals.value("PMs/RejectIncoming").toBool());

    QMenu * sortMenu = menuActions->addMenu(tr("&Sort players"));

    QAction *sortByTier = sortMenu->addAction(tr("Sort players by &tiers"));
    sortByTier->setCheckable(true);
    connect(sortByTier, SIGNAL(triggered(bool)), SLOT(sortPlayersByTiers(bool)));
    sortByTier->setChecked(globals.value("Client/SortPlayersByTier").toBool());
    sortBT = sortByTier->isChecked();

    QAction *sortByAuth = sortMenu->addAction(tr("Sort players by auth &level"));
    sortByAuth->setCheckable(true);
    connect(sortByAuth, SIGNAL(triggered(bool)), SLOT(sortPlayersByAuth(bool)));
    sortByAuth->setChecked(globals.value("Client/SortPlayersByAuth").toBool());
    sortBA = sortByAuth->isChecked();

    QAction *useExitWarning = menuActions->addAction(tr("Show exit warning"));
    useExitWarning->setCheckable(true);
    connect(useExitWarning, SIGNAL(triggered(bool)), SLOT(changeExitWarning(bool)));
    useExitWarning->setChecked(exitWarning);

    QAction *sortChannelsName = menuActions->addAction(tr("Sort channels by name"));
    sortChannelsName->setCheckable(true);
    sortChannelsName->setChecked(globals.value("Client/SortChannelsByName").toBool());
    connect(sortChannelsName, SIGNAL(triggered(bool)), SLOT(sortChannelsToggle(bool)));
    sortCBN = sortChannelsName->isChecked();

    QAction *list_right = menuActions->addAction(tr("Move player list to &right"));
    list_right->setCheckable(true);
    connect(list_right, SIGNAL(triggered(bool)), SLOT(movePlayerList(bool)));
    list_right->setChecked(globals.value("Client/UserListAtRight").toBool());

    QAction *oldShortcuts = menuActions->addAction(tr("Use old shortcuts"));
    oldShortcuts->setCheckable(true);
    connect(oldShortcuts, SIGNAL(triggered(bool)), SLOT(useOldShortcuts(bool)));
    oldShortcuts->setChecked(globals.value("Client/UsingOldShortcuts").toBool());

    mytiermenu = menuBar->addMenu(tr("&Tiers"));
    rebuildTierMenu();

    QMenu *battleMenu = menuBar->addMenu(tr("&Battle options", "Menu"));
    QAction * saveLogs = battleMenu->addAction(tr("Save &Battle Logs"));
    saveLogs->setCheckable(true);
    connect(saveLogs, SIGNAL(triggered(bool)), SLOT(saveBattleLogs(bool)));
    saveLogs->setChecked(globals.value("Battle/SaveLogs").toBool());

    battleMenu->addAction(tr("Change &log folder ..."), this, SLOT(changeBattleLogFolder()));

    battleMenu->addAction(tr("&Sound configuration"),this, SLOT(openSoundConfig()));

    QAction *animateHpBar = battleMenu->addAction(tr("Animate HP Bar"));
    animateHpBar->setCheckable(true);
    connect(animateHpBar, SIGNAL(triggered(bool)), SLOT(animateHpBar(bool)));
    animateHpBar->setChecked(globals.value("Battle/AnimateHp").toBool());

    QAction *oldStyleButtons = battleMenu->addAction(tr("Old school buttons"));
    oldStyleButtons->setCheckable(true);
    connect(oldStyleButtons, SIGNAL(triggered(bool)), SLOT(changeButtonStyle(bool)));
    oldStyleButtons->setChecked(globals.value("Battle/OldAttackButtons").toBool());

    QAction *oldBattleWindow = battleMenu->addAction(tr("Old battle window"));
    oldBattleWindow->setCheckable(true);
    connect(oldBattleWindow, SIGNAL(triggered(bool)), SLOT(changeBattleWindow(bool)));
    oldBattleWindow->setChecked(globals.value("Battle/OldWindow", true).toBool());

    QMenu *newBattleWindow = battleMenu->addMenu(tr("New battle window"));
    {
        QAction *animatedLogger = newBattleWindow->addAction(tr("Animated logger"));
        animatedLogger->setCheckable(true);
        connect(animatedLogger, SIGNAL(triggered(bool)), SLOT(changeBattleLogger(bool)));
        animatedLogger->setChecked(globals.value("Battle/AnimatedLogger", false).toBool());

        QAction *animatedWeather = newBattleWindow->addAction(tr("Show weather animation everytime"));
        animatedWeather->setCheckable(true);
        connect(animatedWeather, SIGNAL(triggered(bool)), SLOT(changeBattleWeather(bool)));
        animatedWeather->setChecked(globals.value("Battle/AnimatedWeather", "always").toString()=="always");

        QAction *screenSize = newBattleWindow->addAction(tr("16:9 animated screen"));
        screenSize->setCheckable(true);
        connect(screenSize, SIGNAL(triggered(bool)), SLOT(changeBattleScreenSize(bool)));
        screenSize->setChecked(globals.value("Battle/AnimatedScreenSize", "712x400").toString() == "712x400");
    }


    QAction *dontUseNicknames = battleMenu->addAction(tr("Don't show Pokemon Nicknames"));
    dontUseNicknames->setCheckable(true);
    connect(dontUseNicknames, SIGNAL(triggered(bool)), SLOT(changeNicknames(bool)));
    dontUseNicknames->setChecked(globals.value("Battle/NoNicknames").toBool());

    mymenubar = menuBar;

    return menuBar;
}

void Client::addPlugin(OnlineClientPlugin *o)
{
    plugins.insert(o);
    hooks.insert(o, o->getHooks());
}

void Client::removePlugin(OnlineClientPlugin *o)
{
    plugins.remove(o);
    hooks.remove(o);
}

void Client::playerKicked(int dest, int src) {
    QString mess;

    if (src == 0) {
        mess = tr("%1 was kicked by the server!").arg(name(dest));
    } else {
        mess = tr("%1 kicked %2!").arg(name(src), name(dest));
    }
    printHtml(toBoldColor(mess, Qt::red));
}

void Client::playerBanned(int dest, int src) {
    QString mess;

    if (src == 0) {
        mess = tr("%1 was banned by the server!").arg(name(dest));
    } else {
        mess = tr("%1 banned %2!").arg(name(src), name(dest));
    }
    printHtml(toBoldColor(mess, Qt::red));
}

void Client::playerTempBanned(int dest, int src, int time)
{
    QString mess;
    time = int(time);
    if(src == 0) {
        mess = tr("%1 was banned by the server for %n minute(s)!", "", time).arg(name(dest));
    } else {
        mess = tr("%1 banned %2 for %n minute(s)!", "", time).arg(name(src), name(dest));
    }
    printHtml(toBoldColor(mess, Qt::red));
}

void Client::askForPass(const QByteArray &salt) {

    QString pass;
    QStringList warns;
    bool ok = wallet.retrieveUserPassword(relay().getIp(), serverName, (secondTeam.name().isEmpty() ? myteam->name() : secondTeam.name()), salt, pass, warns);
    if (!warns.empty()) warns.prepend(""); // for join()

    /* Create a dialog for password input */
    QDialog dialog(this);
    dialog.setObjectName("passwordDialog");
    dialog.setWindowTitle(tr("Enter your password"));
    QVBoxLayout* layout = new QVBoxLayout;
    // Label
    layout->addWidget(new QLabel(tr("<html>Enter the password for your current name.<br/>"
                                    "If you don't have it, the name you have chosen might be already taken."
                                    " Choose different name.<br/>"
                                    "<br/>It is advised to use a slightly different password for each server."
                                    " (The server only sees the encrypted form of the pass, but still...)")
                                 + "<span style='color:red;'>" + warns.join("<br/>") + "</span></html>"));
    // Password input
    QLineEdit *passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passEdit);

    // Save pass
    QCheckBox *savePass = new QCheckBox;
    savePass->setText(tr("Save the user password"));
    layout->addWidget(savePass);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    layout->addWidget(buttonBox);

    dialog.setLayout(layout);

    if (ok) {
        passEdit->setText(pass);
        savePass->setChecked(true);
    }

    int ret = dialog.exec();
    if (!ret) {
        if (loggedIn)
            myregister->setEnabled(true);
        return;
    }
    pass = passEdit->text();
    if (savePass->isChecked()) {
        // TODO: ipv6 support in the future
        wallet.saveUserPassword(relay().getIp(), serverName, myteam->name(), salt, pass);
    }

    QByteArray hash = QCryptographicHash::hash(md5_hash(pass.toLatin1())+salt, QCryptographicHash::Md5);
    relay().notify(NetworkCli::AskForPass, hash);
}

void Client::setReconnectPass(const QByteArray &pass)
{
    reconnectPass = pass;
}

void Client::onReconnectFailure(int reason)
{
    reconnectPass.clear();
    QString reasons[] = {
        tr("Server doesn't have data stored for the reconnection."),
        tr("There's an error when trying to reconnect."),
        tr("The disconnection has lasted too long."),
        tr("Your IP is too different from what's expected.")
    };

    if (unsigned(reason) < sizeof(reasons)/sizeof(QString) && reason >= 0) {
        printLine(tr("The server refused the reconnect attempt with the reason: %1").arg(reasons[reason]));
    } else {
        printLine(tr("The server refused the reconnect attempt."));
    }
    failedBefore = true;
}
void Client::newConnection() {
    printLine(tr("Attempting a new connection."));
    foreach(Channel *chan, mychannels) {
        channelsIWasOn += chan->name();
    }
    relay().connectTo(url, port);
}

void Client::serverPass(const QByteArray &salt) {
    QString pass;
    QStringList warns;
    bool ok = wallet.retrieveServerPassword(relay().getIp(), serverName, pass, warns);
    if (!warns.empty()) warns.prepend(""); // for join()

    /* Create a dialog for password input */
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Enter the server password"));
    QVBoxLayout* layout = new QVBoxLayout;
    // Label
    layout->addWidget(new QLabel(tr("Enter the password for this server.\n"
                                    "This server requires a password to log in.")
                                 + warns.join("\n")));
    // Password input
    QLineEdit *passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passEdit);

    // Save pass
    QCheckBox *savePass = new QCheckBox;
    savePass->setText(tr("Save the server password"));
    layout->addWidget(savePass);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    layout->addWidget(buttonBox);

    dialog.setLayout(layout);

    if (ok) {
        passEdit->setText(pass);
        savePass->setChecked(true);
    }

    int ret = dialog.exec();
    if (!ret) {
        return;
    }
    pass = passEdit->text();
    if (savePass->isChecked()) {
        // TODO: ipv6 support in the future
        wallet.saveServerPassword(relay().getIp(), serverName, pass);
    }
    QByteArray hash = QCryptographicHash::hash(QCryptographicHash::hash(pass.toUtf8(), QCryptographicHash::Md5)+salt, QCryptographicHash::Md5);
    relay().notify(NetworkCli::ServerPass, hash);
}

void Client::sendRegister() {
    if (isConnected) {
        relay().notify(NetworkCli::Register);
        myregister->setDisabled(true);
    } else {
        relay().connectTo(url, port);
    }
}

void Client::reconnect()
{
    relay().connectTo(url, port);
    failedBefore = false;
}

void Client::changeBattleLogFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Logs Directory"),
                                                    QDir::home().absoluteFilePath(LogManager::obj()->getDirectory()));

    if (dir.isNull()) {
        return;
    }

    LogManager::obj()->changeBaseDirectory(dir);
}

void Client::openSoundConfig()
{
    SoundConfigWindow *w = new SoundConfigWindow();

    foreach(BaseBattleWindowInterface *i, mySpectatingBattles) {
        connect(w, SIGNAL(cryVolumeChanged(int)), i, SLOT(changeCryVolume(int)));
        connect(w, SIGNAL(musicVolumeChanged(int)), i, SLOT(changeMusicVolume(int)));
    }

    foreach(BattleWindow *i, mybattles) {
        connect(w, SIGNAL(cryVolumeChanged(int)), i, SLOT(changeCryVolume(int)));
        connect(w, SIGNAL(musicVolumeChanged(int)), i, SLOT(changeMusicVolume(int)));
    }
}

void Client::changeButtonStyle(bool old)
{
    globals.setValue("Battle/OldAttackButtons", old);
}

void Client::changeBattleWindow(bool old)
{
    globals.setValue("Battle/OldWindow", old);
}

void Client::changeBattleLogger(bool logger)
{
    globals.setValue("Battle/AnimatedLogger", logger);
}

void Client::changeBattleScreenSize(bool big)
{
    globals.setValue("Battle/AnimatedScreenSize", big ? "712x400" : "500x400");
}

void Client::changeBattleWeather(bool everyTurn)
{
    globals.setValue("Battle/AnimatedWeather", everyTurn ? "always" : "firstTurn");
}

void Client::changeNicknames(bool old)
{
    globals.setValue("Battle/NoNicknames", old);
}

void Client::saveBattleLogs(bool save)
{
    LogManager::obj()->changeLogSaving(BattleLog, save);
}

void Client::animateHpBar(bool save)
{
    globals.setValue("Battle/AnimateHp", save);
}

void Client::spectatingBattleMessage(int battleId, const QByteArray &command)
{
    if (mySpectatingBattles.contains(battleId)) {
        mySpectatingBattles[battleId]->receiveData(command);
    }
}

bool Client::battling() const
{
    return mybattles.size() > 0;
}

void Client::versionDiff(const ProtocolVersion &v, int level)
{
    serverVersion = v;

    printHtml(toColor(tr("Your client version doesn't match with the server's (%1).").arg(level <= 0 ? tr("older") : tr("more recent")),
                      QColor("#e37800")));

    if (level <= 0) {
        if (globals.contains(QString("ignore_version_%1_%2").arg(v.version).arg(v.subversion)))
            return;

        QString message;
        switch (level) {
        case -3:
            message = tr ("Your version is severely outdated compared to the server. There is going to be important communication problems");
            break;
        case -2:
            message = tr ("Your version is outdated compared to the server. There are going to be some compatibility problems.");
            break;
        case -1:
            message = tr ("Some features have been added to interact with the server since you downloaded your version. Update!");
            break;
        case 0:
            message = tr ("Your version is slightly behind on the server's, though no problems should arise.");
        }

        QMessageBox *update = new QMessageBox(QMessageBox::Information, tr("Old Version"), message,
                                              QMessageBox::Ok | QMessageBox::Ignore, NULL, Qt::Window);
        int result = update->exec();

        if (result & QMessageBox::Ignore)
            ignoreServerVersion(true);
    }
}

void Client::serverNameReceived(const QString &sName)
{
    serverName = sName;
    setWindowTitle(sName);
    pmSystem->setServerName(sName);

    emit titleChanged();
}

void Client::announcementReceived(const QString &ann)
{
    if (ann.length() == 0)
        return;

    server_announcement->setText(ann);
    server_announcement->setAlignment(Qt::AlignCenter);
    server_announcement->show();
}

void Client::tierListReceived(const QByteArray &tl)
{
    tierList.clear();
    tierRoot.buildFromRaw(tl);
    tierList = tierRoot.getTierList();

    if (tierList.empty()) {
        tierRoot.subNodes.push_back(new TierNode("All"));
        tierList = tierRoot.getTierList();
    }

    rebuildTierMenu();
    changeTiersChecked();

    if (globals.value("Client/SortPlayersByTier").toBool()) {
        foreach(Channel *c, mychannels)
            c->sortAllPlayersByTier();
    }
    emit tierListFormed(tierList);
}

void Client::rebuildTierMenu()
{
    if (!mytiermenu) {
        return;
    }

    mytiermenu->clear();
    foreach(QAction *a, mytiers) {
        a->deleteLater();
    }
    mytiers.clear();

    TierActionFactoryTeams f(team());
    mytiers = tierRoot.buildMenu(mytiermenu, this, team()->officialCount() <= 1 ? NULL : &f);
    foreach(QAction *a, mytiers) {
        a->setParent(this);
    }
    singleTeam = team()->officialCount() <= 1;
}

void Client::sortPlayersByTiers(bool byTier)
{
    sortBT = byTier;
    globals.setValue("Client/SortPlayersByTier", sortBT);

    if (sortBT) {
        foreach(Channel *c, mychannels)
            c->sortAllPlayersByTier();
    } else {
        foreach(Channel *c, mychannels)
            c->sortAllPlayersNormally();
    }
}

void Client::sortPlayersByAuth(bool byAuth)
{
    sortBA = byAuth;
    globals.setValue("Client/SortPlayersByAuth", sortBA);

    if (sortBT) {
        foreach(Channel *c, mychannels)
            c->sortAllPlayersByTier();
    } else {
        foreach(Channel *c, mychannels)
            c->sortAllPlayersNormally();
    }
}

void Client::movePlayerList(bool right)
{
    globals.setValue("Client/UserListAtRight", right);

    QWidget *mytab = playersW->parentWidget()->parentWidget();
    QWidget *chatcontainer = mainChat->parentWidget();
    QSplitter *splitter = reinterpret_cast<QSplitter*>( mytab->parentWidget() );
    if (right) {
        splitter->addWidget(mytab);
    } else {
        splitter->addWidget(chatcontainer);
    }
}

void Client::useOldShortcuts(bool old) {
    globals.setValue("Client/UsingOldShortcuts", old);
    top->updateMenuBar();
}

void Client::changeTiersChecked()
{
    if (singleTeam != (team()->officialCount() <= 1)) {
        rebuildTierMenu();
    }
    foreach(QAction *a, mytiers) {
        QString tier = a->property("tier").toString();
        int team = a->property("team").toInt();

        a->setChecked(team < this->team()->officialCount() && this->team()->tier(team) == tier);
    }
}

void Client::changeTier()
{
    QAction *a = (QAction*)sender();

    int team = a->property("team").toInt();
    QString tier = a->property("tier").toString();

    a->setChecked(team < this->team()->officialCount() && this->team()->tier(team) == tier);
    relay().notify(NetworkCli::TierSelection, quint8(team), tier);
}

bool Client::playerExist(int id) const
{
    return myplayersinfo.contains(id);
}

PlayerInfo Client::player(int id) const
{
    return myplayersinfo.value(id);
}

void Client::seeInfo(QTreeWidgetItem *it)
{
    QTreeWidgetItem *parent = it->parent();

    if (dynamic_cast<QIdTreeWidgetItem*>(it))
        seeInfo(((QIdTreeWidgetItem*)(it))->id(), parent ? parent->text(0) : "");
}

void Client::seeInfo(int id, QString tier)
{
    if (playerExist(id))
    {
        int challengeId = freeChallengeId();
        ChallengeDialog *mychallenge = new ChallengeDialog(player(id), team(), ownId(), challengeId);
        mychallenge->setChallenging(tier);

        connect(mychallenge, SIGNAL(challenge(ChallengeInfo)), &relay(), SLOT(sendChallengeStuff(ChallengeInfo)));
        connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
        connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));

        mychallengeids.insert(challengeId, mychallenge);
        mychallenges.insert(mychallenge);
    }
}

void Client::seeChallenge(const ChallengeInfo &c)
{
    int challengeId = freeChallengeId();
    if (playerExist(c))
    {
        if (!call("beforeChallengeReceived(int,int,QString,int)", challengeId, c.opponent(), c.desttier, static_cast<int>(c.clauses))) {
            ChallengeInfo d = c;
            d.dsc = ChallengeInfo::Busy;
            relay().sendChallengeStuff(d);
        } else if (busy()) {
            /* Warns the server that we are too busy to accept the challenge */
            ChallengeInfo d = c;
            d.dsc = ChallengeInfo::Busy;
            relay().sendChallengeStuff(d);
        } else {
            ChallengeDialog *mychallenge = new ChallengeDialog(player(c), team(), ownId(), challengeId);
            mychallenge->setChallengeInfo(c);

            connect(mychallenge, SIGNAL(challenge(ChallengeInfo)), &relay(), SLOT(sendChallengeStuff(ChallengeInfo)));
            connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
            connect(mychallenge, SIGNAL(cancel(ChallengeInfo)), &relay(), SLOT(sendChallengeStuff(ChallengeInfo)));
            connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
            mychallenge->activateWindow();

            mychallengeids.insert(challengeId, mychallenge);
            mychallenges.insert(mychallenge);

            call("afterChallengeReceived(int,int,QString,int)", challengeId, c.opponent(), c.desttier, static_cast<int>(c.clauses));
        }
    }
}

void Client::battleStarted(int battleId, const Battle &battle, const TeamBattle &team, const BattleConfiguration &conf)
{
    if (!mybattles.contains(battleId)) {
        int id = battle.id1 == ownId() ? battle.id2: battle.id1;

        cancelFindBattle(false);
        BattleWindow * mybattle = new BattleWindow(battleId, player(ownId()), player(id), team, conf);
        connect(this, SIGNAL(destroyed()), mybattle, SLOT(deleteLater()));

        mybattle->activateWindow();

        connect(mybattle, SIGNAL(forfeit(int)), SLOT(forfeitBattle(int)));
        connect(mybattle, SIGNAL(battleCommand(int, BattleChoice)), &relay(), SLOT(battleCommand(int, BattleChoice)));
        connect(mybattle, SIGNAL(battleMessage(int, QString)), &relay(), SLOT(battleMessage(int, QString)));
        connect(this, SIGNAL(destroyed()), mybattle, SLOT(close()));
        connect(&relay(), SIGNAL(disconnected()), mybattle, SLOT(onDisconnection()));
        //connect(this, SIGNAL(musicPlayingChanged(bool)), mybattle, SLOT(playMusic(bool)));

        mybattles[battleId] = mybattle;

        battleStarted(battleId, battle);

        call("onBattleStarted(BaseBattleWindowInterface*)", static_cast<BaseBattleWindowInterface*>(mybattle));
    } else {
        //We reconnected probably, and our team changed
        mybattles[battleId]->updateTeam(team);
    }
}

void Client::battleStarted(int bid, const Battle &battle)
{
    myplayersinfo[battle.id1].flags.setFlag(PlayerInfo::Battling, true);
    myplayersinfo[battle.id2].flags.setFlag(PlayerInfo::Battling, true);

    battles.insert(bid, battle);
    foreach(Channel *c, mychannels) {
        c->battleStarted(bid, battle);
    }
}


void Client::battleReceived(int battleid, const Battle &battle)
{
    if (battles.contains(battleid)) {
        return;
    }

    myplayersinfo[battle.id1].flags.setFlag(PlayerInfo::Battling, true);
    myplayersinfo[battle.id2].flags.setFlag(PlayerInfo::Battling, true);

    battles.insert(battleid, battle);

    updateState(battle.id1);
    updateState(battle.id2);
}

void Client::watchBattle(int battleId, const BattleConfiguration &conf)
{
    BaseBattleWindowInterface *battle = new BaseBattleWindowIns(player(conf.ids[0]), player(conf.ids[1]), conf, ownId());

    connect(this, SIGNAL(destroyed()), battle, SLOT(close()));
    connect(this, SIGNAL(destroyed()), battle, SLOT(deleteLater()));
    connect(battle, SIGNAL(closedBW(int)), SLOT(stopWatching(int)));
    connect(battle, SIGNAL(battleMessage(int, QString)), &relay(), SLOT(battleMessage(int, QString)));

    battle->battleId() = battleId;
    mySpectatingBattles[battleId] = battle;
}

void Client::stopWatching(int battleId)
{
    if (mySpectatingBattles.contains(battleId)) {
        if (dynamic_cast<Analyzer*>(sender())) {
            mySpectatingBattles[battleId]->disable();
        } else {
            relay().notify(NetworkCli::SpectateBattle, qint32(battleId), Flags(0));
            mySpectatingBattles[battleId]->close();
        }
        mySpectatingBattles.remove(battleId);
    }
}

void Client::forfeitBattle(int id)
{
    relay().sendBattleResult(id, Forfeit);
    removeBattleWindow(id);
}

void Client::battleFinished(int battleid, int res, int winner, int loser)
{
    if (res == Close || res == Forfeit) {
        if (mybattles.contains(battleid) || mySpectatingBattles.contains(battleid)) {
            disableBattleWindow(battleid);
        }
    }

    foreach(Channel *c, mychannels) {
        c->battleEnded(battleid, res, winner, loser);
    }

    battles.remove(battleid);

    if (myplayersinfo.contains(winner))
        myplayersinfo[winner].flags.setFlag(PlayerInfo::Battling, false);
    if (myplayersinfo.contains(loser))
        myplayersinfo[loser].flags.setFlag(PlayerInfo::Battling, false);

    foreach(Battle b, battles) {
        if (myplayersinfo.contains(winner) && (b.id1 == winner || b.id2 == winner)) {
            myplayersinfo[winner].flags.setFlag(PlayerInfo::Battling, true);
        }
        if (myplayersinfo.contains(loser) && (b.id1 == loser || b.id2 == loser)) {
            myplayersinfo[loser].flags.setFlag(PlayerInfo::Battling, true);
        }
    }

    updateState(winner);
    updateState(loser);
}

void Client::battleCommand(int battleid, const QByteArray &command)
{
    if (!mybattles.contains(battleid))
        return;

    mybattles[battleid]->receiveData(command);
}

void Client::disableBattleWindow(int battleid)
{
    if (!mybattles.contains(battleid) && !mySpectatingBattles.contains(battleid))
        return;

    BaseBattleWindowInterface *w = mybattles.contains(battleid) ? mybattles.take(battleid) : mySpectatingBattles.take(battleid);
    w->disable();
}

void Client::removeBattleWindow(int battleid)
{
    if (!mybattles.contains(battleid))
        return;

    BattleWindow *w = mybattles.take(battleid);
    w->close();
}

QString Client::name(int id) const
{
    if (myplayersinfo.contains(id))
        return player(id).name;
    else
        return "~Unknown~";
}

void Client::cancelFindBattle(bool verbose)
{
    if (!findingBattle) {
        return;
    }
    if (verbose) {
        relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Cancelled, 0));
    }
    findMatch->setText(tr("&Find battle"));
    findingBattle = false;
}

void Client::challengeStuff(const ChallengeInfo &c)
{
    if (c.desc() == ChallengeInfo::Sent) {
        if (busy()) {
            relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Busy, c));
        } else if (myIgnored.contains(player(c).id)) {
            relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Refused, c));
        } else {
            seeChallenge(c);
        }
    } else {
        if (playerExist(c.opponent())) {
            if (c.opponent() != ownId()) {
                ChallengeDialog *b;
                if (c.desc() == ChallengeInfo::Refused) {
                    printLine(tr("%1 refused your challenge.").arg(name(c)));
                } else if (c.desc() == ChallengeInfo::Busy) {
                    printLine(tr("%1 is busy.").arg(name(c)));
                } else if (c.desc() == ChallengeInfo::Cancelled) {
                    printLine(tr("%1 cancelled their challenge.").arg(name(c)));
                    while ( (b = getChallengeWindow(c)) ) {
                        closeChallengeWindow(b);
                    }
                } else if (c.desc() == ChallengeInfo::InvalidTeam) {
                    printLine(tr("%1 has an invalid team.").arg(name(c)));
                } else if (c.desc() == ChallengeInfo::InvalidGen) {
                    printLine(tr("%1 has a different gen than yours.").arg(name(c)));
                } else if (c.desc() == ChallengeInfo::InvalidTier) {
                    printLine(tr("%1 doesn't have a team with the tier: %2.").arg(name(c), c.desttier));
                }
            }
        }
    }
}

void Client::awayChanged(int id, bool away)
{
    if (player(id).away() == away) {
        return;
    }

    if (away) {
        printLine(IdleEvent, id, tr("%1 is idling.").arg(name(id)));
    } else {
        printLine(IdleEvent, id, tr("%1 is active and ready for battles.").arg(name(id)));
    }

    playerInfo(id).changeState(PlayerInfo::Away, away);
    updateState(id);
}

void Client::ladderChanged(int id, bool ladder)
{
    if (player(id).flags[PlayerInfo::LadderEnabled] == ladder) {
        return;
    }

    playerInfo(id).changeState(PlayerInfo::LadderEnabled, ladder);
}

bool Client::busy() const
{
    return away();
}

bool Client::away() const
{
    return player(ownId()).away();
}

QString Client::tier(int player) const
{
    return this->player(player).ratings.begin().key();
}

QStringList Client::tiers(int player) const
{
    return this->player(player).ratings.keys();
}

void Client::clearChallenge()
{
    mychallenges.remove(dynamic_cast<ChallengeDialog*>(sender()));
}

void Client::errorFromNetwork(int errnum, const QString &errorDesc)
{
    /* error 1 is simply 'remote host disconnected you */
    if (errnum != 1) {
        printHtml("<i>"+tr("Error while connected to server -- Received error n°%1: %2").arg(errnum).arg(errorDesc) + "</i>");
    }
}

void Client::connected()
{
    printLine(tr("Connected to Server!"));
    isConnected = true;
    myregister->setText(tr("&Register"));

    QSettings s;

    s.beginGroup("password");
    if (s.childKeys().contains(QCryptographicHash::hash(relay().getIp().toUtf8(), QCryptographicHash::Md5)))
        relay().disconnectFromHost();
    s.endGroup();

    if (reconnectPass.isEmpty()) {
        if (channelsIWasOn.isEmpty()) {
            //qDebug() << "isempty";
            QStringList AutoJoinChannels = s.value(QString("AutoJoinChannels/%1").arg(relay().getIp())).toStringList();
            QString DefaultChannel = s.value(QString("DefaultChannels/%1").arg(relay().getIp())).toString();
            relay().login(*team(), s.value("Client/EnableLadder").toBool(), globals.value("Client/Idle").toBool(), team()->color(), DefaultChannel, AutoJoinChannels);
        } else {
            //qDebug() << "notempty";
            QStringList autoJoinChannels = channelsIWasOn;
            QString defaultChannel = autoJoinChannels.takeFirst();
            channelsIWasOn.clear();
            //QStringList AutoJoinChannels = s.value(QString("AutoJoinChannels/%1").arg(relay().getIp())).toStringList();
            relay().login(*team(), ladder->isChecked(), goaway->isChecked(), team()->color(), defaultChannel, autoJoinChannels);
        }
    } else {
        relay().notify(NetworkCli::Reconnect, quint32(ownId()), reconnectPass, quint32(relay().getCommandCount()));
    }

    emit PMDisconnected(false);
}

void Client::disconnected()
{
    if (reconnectPass.length() > 0) {
        printHtml(tr("<hr><br>Disconnected from Server! If the disconnect is due to an internet problem, try to <a href=\"po:reconnect\">reconnect</a> once the issue is solved.<br><hr>"));
    } else {
        printHtml(tr("<hr><br>Disconnected from Server!<br><hr>"));
    }

    onDisconnection();

    isConnected = false;
    loggedIn = false;
    myregister->setText(tr("&Reconnect"));
    myregister->setEnabled(true);

    emit PMDisconnected(true);
    if (failedBefore)
        newConnection();
    failedBefore = false;
}

void Client::onDisconnection()
{
    /* Handle things on disconnection */

    /* Let our existing spectating battles know we have disconnected */
    foreach(BaseBattleWindowInterface *interface, mySpectatingBattles) {
        interface->onDisconnection();
    }
}

TeamHolder* Client::team()
{
    return myteam;
}

Analyzer &Client::relay()
{
    return *myrelay;
}

QString Client::announcement()
{
    return server_announcement->document()->toPlainText();
}

void Client::playerLogin(const PlayerInfo& p, const QStringList &tiers, bool ignore)
{
    if (!ignore) {
        cleanData();
        _mid = p.id;
        mynick = p.name;
        myplayersinfo[p.id] = p;
        mynames[p.name] = p.id;

        mylowernames[p.name.toLower()] = p.id;

        playerReceived(p);
        tiersReceived(tiers);

        /* If it's us, we know we've logged in */
        if (p.id == ownId()) {
            loggedIn = true;
        }
    }
}

void Client::tiersReceived(const QStringList &tiers)
{
    if (waitingOnSecond) {
        waitingOnSecond = false;
        *team() = secondTeam;
    }
    team()->setTiers(tiers);
    rebuildTierMenu();
    changeTiersChecked();
}

void Client::playerLogout(int id)
{
    /* removes the item in the playerlist */
    removePlayer(id);
}

bool Client::hasPlayer (int id)
{
    if (pmedPlayers.contains(id)) {
        return true;
    }

    if (mypms.contains(id)) {
        return true;
    }

    foreach (Channel *c, mychannels) {
        if (c->hasPlayer(id)) {
            return false;
        }

    }

    return true;
}

bool Client::hasKnowledgeOf(int id) {

    /* This function helps detect log outs... it's a fix somewhat,
       but it'll say a user logged out if they leave a channel and
       they are then no longer in any of your channels

       TODO: More in-depth fix for later releases
    */

    if (mypms.contains(id)) {
        return true;
    }

    int nc = 0;

    foreach (Channel *c, mychannels) {
        if (c->hasPlayer(id)) {
            nc++;
        }
    }

    return nc != 0;
}

void Client::removePlayer(int id)
{
    QString name = player(id).name;

    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(id))
            c->playerLogOut(id);
    }

    if (myIgnored.contains(id))
        removeIgnore(id);

    myplayersinfo.remove(id);
    pmedPlayers.remove(id);
    fade.remove(id);

    QHash<int, PMStruct*>::iterator pm = mypms.find(id);
    if (pm != mypms.end()) {
        pm.value()->disable();
        disabledpms[name] = pm.value();
        mypms.erase(pm);
    }

    /* Name removed... Only if no one took it since the 10 minutes we never saw the guy */
    if (mynames.value(name) == id) {
        mynames.remove(name);
    }

    if (mylowernames.value(name.toLower()) == id) {
        mylowernames.remove(name.toLower());
    }
}

void Client::fadeAway()
{
    foreach(int player, myplayersinfo.keys()) {
        if (pmedPlayers.contains(player))
            continue;
        if (mypms.contains(player))
            continue;
        foreach(Channel *c, mychannels) {
            if (c->hasRemoteKnowledgeOf(player))
                goto refresh;
        }

        fade[player] += 1;
        if (fade[player] >= 5) {
            removePlayer(player);
        }
        continue;
refresh:
        refreshPlayer(player);
    }
}

void Client::refreshPlayer(int id)
{
    fade.remove(id);
}

QString Client::ownName() const
{
    return name(ownId());
}

QString Client::channelName(int id) const
{
    return m_channelNames.value(id);
}

int Client::channelId(const QString &name) const
{
    return m_channelByNames.value(name.toLower());
}

const QHash<qint32, QString>& Client::getChannelNames() const
{
    return m_channelNames;
}

QString Client::authedNick(int id) const
{
    PlayerInfo p = player(id);

    QString nick = p.name;

    return nick;
}

void Client::ownPlayerReceived()
{
    if (ladder) {
        ladder->setChecked(player(ownId()).flags[PlayerInfo::LadderEnabled]);
    }
    if (goaway) {
        goaway->setChecked(player(ownId()).flags[PlayerInfo::Away]);
    }
}

void Client::playerReceived(const PlayerInfo &p)
{
    bool newPlayer = false;

    if (name(p.id) != p.name) {
        printLine(TeamEvent, p.id, tr("%1 changed names and is now known as %2.").arg(name(p.id), p.name));
        if (p.id == ownId()) {
            mynick = p.name;
        }
    }

    if (myplayersinfo.contains(p.id)) {
        /* It's not sync perfectly, so someone who relogs can happen, that's why we do that test */
        if (mynames.value(p.name) == p.id) {
            mynames.remove(player(p.id).name);
        }

        if (mylowernames.value(p.name.toLower()) == p.id) {
            mylowernames.remove(player(p.id).name.toLower());
        }

        myplayersinfo.remove(p.id);
    } else {
        newPlayer = true;
    }

    myplayersinfo.insert(p.id, p);
    refreshPlayer(p.id);

    changeName(p.id, p.name);

    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(p.id))
            c->playerReceived(p.id);
        else
            c->changeName(p.id, p.name); /* Even if the player isn't in the channel, someone in the channel could be battling him, ... */
    }
    QHash<QString, PMStruct*>::iterator pm = disabledpms.find(name(p.id));
    if (pm != disabledpms.end()) {
        PMStruct *window = pm.value();
        disabledpms.erase(pm);
        mypms[p.id] = window;
        window->reuse(p.id);
    }

    if (p.id == ownId()) {
        ownPlayerReceived();
    }

    if (newPlayer) {
        call("onPlayerReceived(int)", p.id);
    }
}

void Client::changeChannelId(int orId, int destId)
{
    if (hasChannel(orId)) {
        Channel *c = mychannels.take(orId);
        mychannels.insert(destId, c);
        c->setId(destId);
    }
}

void Client::changeName(int player, const QString &name)
{
    mynames[name] = player;
    mylowernames[name.toLower()] = player;

    if (mypms.contains(player)) {
        mypms[player]->changeName(name);
    }

    if (player == ownId()) {
        foreach(PMStruct *p, mypms) {
            p->changeSelf(name);
        }
    }
}

QColor Client::color(int id) const
{
    if (player(id).color.name() == "#000000") {
        return Theme::ChatColor(id);
    } else {
        return player(id).color;
    }
}

int Client::id(const QString &name) const
{
    QString nameToLower = name.toLower();

    if (mylowernames.contains(nameToLower)) {
        return mylowernames[nameToLower];
    } else {
        return -1;
    }
}

void Client::closeChallengeWindow(ChallengeDialog *b)
{
    mychallenges.remove(b);
    b->forcedClose();
}

ChallengeDialog * Client::getChallengeWindow(int player)
{
    foreach(ChallengeDialog *c, mychallenges) {
        if (c->id() == player)
            return c;
    }

    return NULL;
}


void Client::sendChallenge(int id, int clauses, int mode)
{
    ChallengeInfo c;
    c.clauses = clauses;
    c.opp = id;
    c.rated = false;
    c.team = myteam->currentTeam();
    c.desttier = myteam->tier();
    c.mode = mode;
    c.dsc = ChallengeInfo::Sent;
    relay().sendChallengeStuff(c);
}

void Client::acceptChallenge(int cId)
{
    foreach(ChallengeDialog *d, mychallengeids) {
        if (d->cid() == cId) {
            ChallengeInfo c = d->challengeInfo();
            c.dsc = ChallengeInfo::Accepted;
            relay().sendChallengeStuff(c);
            closeChallengeWindow(d);
            break;
        }
    }
}

int Client::freeChallengeId()
{
    int ret = 0;
    do {
        ++ret;
    } while (mychallengeids.contains(ret));

    return ret;
}

void Client::openTeamBuilder()
{
    if (myteambuilder) {
        myteambuilder->activateWindow();
        myteambuilder->raise();
        return;
    }

    secondTeam = *team();

    myteambuilder = new TeamBuilder(pluginManager, &secondTeam);
    myteambuilder->show();
    myteambuilder->setAttribute(Qt::WA_DeleteOnClose, true);

    if (top) {
        QMenuBar *bar = myteambuilder->createMenuBar(top);
        myteambuilder->setMenuBar(bar);
    }
    myteambuilder->setTierList(tierList);

    connect(this, SIGNAL(destroyed()), myteambuilder, SLOT(close()));
    connect(myteambuilder, SIGNAL(done()), this, SLOT(changeTeam()));
    connect(myteambuilder, SIGNAL(done()), myteambuilder, SLOT(close()));
    connect(myteambuilder, SIGNAL(reloadMenuBar()), SLOT(reloadTeamBuilderBar()));
    if (top) {
        connect(myteambuilder, SIGNAL(reloadDb()), top, SLOT(reloadPokemonDatabase()));
    }
    connect(this, SIGNAL(tierListFormed(const QStringList &)), myteambuilder, SLOT(setTierList(const QStringList&)));
}

void Client::reloadTeamBuilderBar()
{
    if (top) {
        myteambuilder->setMenuBar(myteambuilder->createMenuBar(top));
    }
}

void Client::changeTeam(const TeamHolder &t)
{
    secondTeam = t;

    changeTeam();
}

void Client::changeTeam()
{
    if (battling() && secondTeam.name() != mynick) {
        printLine(tr("You can't change teams while battling, so your nick was kept."));
        secondTeam.name() = mynick;
    }
    if (secondTeam.name().isEmpty()) {
        secondTeam.name() = mynick;
    }

    cancelFindBattle(false);
    waitingOnSecond = true;
    relay().sendTeam(secondTeam);
}

void Client::changeName(const QString &name)
{
    relay().notify(NetworkCli::SendTeam, Flags(1), name);
}

bool Client::hasPlayerInfo (int id)
{
    return myplayersinfo.contains(id);
}

PlayerInfo &Client::playerInfo(int id)
{
    return myplayersinfo[id];
}

void Client::updateState(int id)
{
    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(id)) {
            c->updateState(id);
        }
    }
}

bool Client::isIgnored(int id) const
{
    return myIgnored.contains(id);
}

void Client::requestBan(const QString &name)
{
    if (id(name) == -1) {
        relay().notify(NetworkCli::CPBan, name);
    } else {
        ban(id(name));
    }
}

void Client::requestTempBan(const QString &name, int time)
{
    if (id(name) == -1) {
        relay().notify(NetworkCli::CPBan, name, qint32(time));
    } else {
        tempban(id(name), time);
    }
}

void Client::ignore(int id)
{
    if (myIgnored.contains(id) || !hasPlayerInfo(id))
        return;
    printLine(id, tr("You ignored %1.").arg(name(id)));
    myIgnored.append(id);
    updateState(id);
}

void Client::ignore(int id, bool ign)
{
    if (ign) {
        ignore(id);
    }
    else {
        removeIgnore(id);
    }
}

void Client::removeIgnore(int id)
{
    if (!myIgnored.contains(id) || !hasPlayerInfo(id)) {
        return;
    }

    printLine(id, tr("You stopped ignoring %1.").arg(name(id)));
    myIgnored.removeOne(id);
    updateState(id);
}

void Client::printHtml(const QString &html)
{
    if(call("beforeNewMessage(QString,bool)", html, true)){
        foreach(Channel *c, mychannels)
            c->printHtml(html, false, true);
    }
    call("afterNewMessage(QString,bool)", html, true);
}

void Client::printLine(const QString &line)
{
    if (mychannels.size() == 0)
        return;

    if(call("beforeNewMessage(QString,bool)", line, false)){
        foreach(Channel *c, mychannels)
            c->printLine(line, false, false, true);
    }
    call("afterNewMessage(QString,bool)", line, false);
}

/* Prints a line regarding a particular player */
void Client::printLine(int playerid, const QString &line)
{
    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(playerid))
            c->printLine(line, false);
    }
}

void Client::printLine(int event, int playerid, const QString &line)
{
    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(playerid) && c->eventEnabled(event))
            c->printLine(line, false, false);
    }
}

void Client::printChannelMessage(const QString &mess, int channel, bool html)
{
    if (hasChannel(channel) && call("beforeChannelMessage(QString,int,bool)", mess, channel, html)) {
        if (html) {
            this->channel(channel)->printHtml(mess);
        } else {
            this->channel(channel)->printLine(mess);
        }
        call("afterChannelMessage(QString,int,bool)", mess, channel, html);
    }
}

void Client::trayMessage(const QString &title, const QString &message)
{
    MainEngine::inst->showMessage(title, message);
}

bool Client::windowActive() {
    return qApp->activeWindow();
}

void Client::changeExitWarning(bool show)
{
    exitWarning = show;
    globals.setValue("Client/ShowExitWarning", show);
}

void Client::showExitWarning()
{
    if (exitWarning && loggedIn) {
        QDialog dialog(this);
        dialog.setObjectName("exitWarning");
        dialog.setWindowTitle(tr("Are you sure?"));
        QVBoxLayout* layout = new QVBoxLayout;

        layout->addWidget(new QLabel(tr("You are about to exit the server.")));

        QCheckBox *showNextTime = new QCheckBox;
        showNextTime->setText(tr("Show this warning next time"));
        showNextTime->setChecked(true);
        layout->addWidget(showNextTime);

        layout->addWidget(new QLabel(tr("(This can be changed by going to Options -> Show exit warning.)")));

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
        layout->addWidget(buttonBox);

        dialog.setLayout(layout);

        int ret = dialog.exec();
        if (!ret) {
            return;
        }
        if (ret != QMessageBox::Cancel) {
            changeExitWarning(showNextTime->isChecked());
            emit done();
        }
    } else {
        emit done();
    }
}
