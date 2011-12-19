#include "client.h"
#include "mainwindow.h"
#include "challenge.h"
#include "logmanager.h"
#include "Teambuilder/teambuilder.h"
#include "battlewindow.h"
#include "basebattlewindow.h"
#include "pmwindow.h"
#include "controlpanel.h"
#include "ranking.h"
#include "poketextedit.h"
#include "../Utilities/functions.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "channel.h"
#include "theme.h"

Client::Client(TrainerTeam *t, const QString &url , const quint16 port) : myteam(t), findingBattle(false), url(url), port(port), myrelay()
{
    isConnected = true;
    _mid = -1;
    selectedChannel = -1;
    setAttribute(Qt::WA_DeleteOnClose, true);
    myteambuilder = NULL;
    lastAutoPM = time(NULL);

    /* different events */
    eventlist << "show_player_events_idle" << "show_player_events_battle" << "show_player_events_channel" << "show_player_events_team";

    QHBoxLayout *h = new QHBoxLayout(this);
    QSplitter *s = new QSplitter(Qt::Horizontal);
    s->setStyle(QStyleFactory::create("Plastique"));
    h->addWidget(s);
    s->setChildrenCollapsible(false);

    QSettings settings;

    QTabWidget *mytab = new QTabWidget();
    /* Cancels out the effect of the splitter being Plastique */
    mytab->setStyle(QStyleFactory::create(settings.value("application_style").toString()));
    mytab->setMovable(true);
    mytab->addTab(playersW = new QStackedWidget(), tr("Players"));
    mytab->addTab(battlesW = new QStackedWidget(), tr("Battles"));
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

    layout->addWidget(announcement = new SmallPokeTextEdit());
    announcement->setObjectName("Announcement");
    announcement->setOpenExternalLinks(true);
    announcement->hide();
    announcement->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(mainChat = new QExposedTabWidget(), 100);
    mainChat->setObjectName("MainChat");
    mainChat->setMovable(true);
    mainChat->setTabsClosable(true);
    /* Cancels out the effect of the splitter being Plastique */
    mainChat->setStyle(QStyleFactory::create(settings.value("application_style").toString()));
    //    layout->addWidget(myline = new QLineEdit());
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
    mynick = t->trainerNick();

    s->setSizes(QList<int>() << 200 << 800);

    connect(mainChat, SIGNAL(tabCloseRequested(int)), SLOT(leaveChannelR(int)));
    connect(mainChat, SIGNAL(currentChanged(int)), SLOT(firstChannelChanged(int)));
    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));
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
    channelPlayers(0);

    /* move player tab to right if user has selected it
     * this needs to be done at the end of this function to work properly
     */
    if (settings.value("user_list_at_right").toBool()) {
        s->addWidget(mytab);
    }
    if(settings.value("sort_channels_by_name").toBool()) {
        sortCBN = 1;
    } else {
        sortCBN = 0;
    }
    if(settings.value("pm_disabled").toBool()) {
        pmDisabled = 1;
    } else {
        pmDisabled = 0;
    }
}

Client::~Client()
{
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
    connect(relay, SIGNAL(playerReceived(PlayerInfo)), SLOT(playerReceived(PlayerInfo)));
    connect(relay, SIGNAL(teamChanged(PlayerInfo)), SLOT(teamChanged(PlayerInfo)));
    connect(relay, SIGNAL(playerLogin(PlayerInfo)), SLOT(playerLogin(PlayerInfo)));
    connect(relay, SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(relay, SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(relay, SIGNAL(battleStarted(int, int, TeamBattle, BattleConfiguration)),
            SLOT(battleStarted(int, int, TeamBattle, BattleConfiguration)));
    connect(relay, SIGNAL(battleStarted(int,int, int)), SLOT(battleStarted(int, int, int)));
    connect(relay, SIGNAL(battleFinished(int, int,int,int)), SLOT(battleFinished(int, int,int,int)));
    connect(relay, SIGNAL(battleMessage(int, QByteArray)), this, SLOT(battleCommand(int, QByteArray)));
    connect(relay, SIGNAL(passRequired(QString)), SLOT(askForPass(QString)));
    connect(relay, SIGNAL(serverPassRequired(QString)), SLOT(serverPass(QString)));
    connect(relay, SIGNAL(notRegistered(bool)), myregister, SLOT(setEnabled(bool)));
    connect(relay, SIGNAL(playerKicked(int,int)),SLOT(playerKicked(int,int)));
    connect(relay, SIGNAL(playerBanned(int,int)),SLOT(playerBanned(int,int)));
    connect(relay, SIGNAL(PMReceived(int,QString)), SLOT(PMReceived(int,QString)));
    connect(relay, SIGNAL(awayChanged(int, bool)), SLOT(awayChanged(int, bool)));
    connect(relay, SIGNAL(spectatedBattle(int,BattleConfiguration)), SLOT(watchBattle(int,BattleConfiguration)));
    connect(relay, SIGNAL(spectatingBattleMessage(int,QByteArray)), SLOT(spectatingBattleMessage(int , QByteArray)));
    connect(relay, SIGNAL(spectatingBattleFinished(int)), SLOT(stopWatching(int)));
    connect(relay, SIGNAL(versionDiff(QString, QString)), SLOT(versionDiff(QString, QString)));
    connect(relay, SIGNAL(serverNameReceived(QString)), SLOT(serverNameReceived(QString)));
    connect(relay, SIGNAL(tierListReceived(QByteArray)), SLOT(tierListReceived(QByteArray)));
    connect(relay, SIGNAL(announcement(QString)), SLOT(announcementReceived(QString)));
    connect(relay, SIGNAL(channelsListReceived(QHash<qint32,QString>)), SLOT(channelsListReceived(QHash<qint32,QString>)));
    connect(relay, SIGNAL(channelPlayers(int,QVector<qint32>)), SLOT(channelPlayers(int,QVector<qint32>)));
    connect(relay, SIGNAL(channelCommandReceived(int,int,QDataStream*)), SLOT(channelCommandReceived(int,int,QDataStream*)));
    connect(relay, SIGNAL(addChannel(QString,int)), SLOT(addChannel(QString,int)));
    connect(relay, SIGNAL(removeChannel(int)), SLOT(removeChannel(int)));
    connect(relay, SIGNAL(channelNameChanged(int,QString)), SLOT(channelNameChanged(int,QString)));
}

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

    return channelByNames[mainChat->tabText(mainChat->currentIndex()).toLower()];
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
    int chanid = channelByNames.value(mainChat->tabText(tabindex).toLower());

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

    channelNames = channelsL;
    QHashIterator<qint32, QString> it (channelNames);

    while (it.hasNext()) {
        it.next();

        channelByNames.insert(it.value().toLower(), it.key());

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
    if(sortCBN) {
        sortChannels();
    }
}

void Client::sortChannelsToggle(bool newvalue)
{
    QSettings s;
    s.setValue("sort_channels_by_name", newvalue);
    if(sortCBN) {
        sortChannels();
    }
}

void Client::sortChannels() {
    channels->sortItems();
}

void Client::channelPlayers(int chanid, const QVector<qint32> &ids)
{
    if (hasChannel(chanid)) {
        /* Then for some reason we aren't synchronized, but let's get it smooth */
        Channel *c = mychannels.value(chanid);

        c->receivePlayerList(ids);

        return;
    }

    Channel *c = new Channel(channelNames.value(chanid), chanid, this);

    for(int i =0;i < channels->count(); i++) {
        if (channels->item(i)->text() == c->name())
            channels->item(i)->setIcon(chatot);
    }

    playersW->addWidget(c->playersWidget());
    mainChat->addTab(c->mainChat(), c->name());
    battlesW->addWidget(c->battlesWidget());
    if (mainChat->count() == 1)
        // set tab complete for first chan
        myline->setPlayers(c->playersWidget()->model());

    mychannels[chanid] = c;

    c->receivePlayerList(ids);

    connect(c, SIGNAL(quitChannel(int)), SLOT(leaveChannel(int)));
    connect(c, SIGNAL(battleReceived2(int,int,int)), this, SLOT(battleReceived(int,int,int)));
    connect(c, SIGNAL(activated(Channel*)), this, SLOT(channelActivated(Channel*)));
    connect(c, SIGNAL(pactivated(Channel*)), this, SLOT(pingActivated(Channel*)));
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
        s.beginGroup(channelNames.value(item->id()));
        QSettings globals;

        QMenu *show_events = new QMenu(this);
        mychanevents.clear();
        QAction *action;

        bool found = false;
        foreach (QString str, eventSettings()) {
            if (s.contains(str)) {
                found = true;
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
        action->setChecked(s.value("show_player_events_idle",
                                   globals.value("show_player_events_idle").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showIdleEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Enable battle events"));
        action->setCheckable(true);
        action->setChecked(s.value("show_player_events_battle",
                                   globals.value("show_player_events_battle").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showBattleEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Enable channel events"));
        action->setCheckable(true);
        action->setChecked(s.value("show_player_events_channel",
                                   globals.value("show_player_events_channel").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showChannelEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        action = show_events->addAction(tr("Enable team change events"));
        action->setCheckable(true);
        action->setChecked(s.value("show_player_events_team",
                                   globals.value("show_player_events_team").toBool()).toBool());
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
        connect(action, SIGNAL(triggered(bool)), SLOT(showTeamEvents(bool)));
        createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
        mychanevents.push_back(action);

        if (serverName.size() > 0) {
            show_events->addSeparator();
            action = show_events->addAction(tr("Auto-join"));
            action->setCheckable(item->id() != 0); // can't disable for main channel
            action->setChecked(item->id() == 0 ||
                               autojoinChannels.contains(channelNames.value(item->id())));
            if (item->id() != 0) {
                createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), item->id());
                connect(action, SIGNAL(triggered(bool)), SLOT(toggleAutoJoin(bool)));
                createIntMapper(action, SIGNAL(triggered()), this, SLOT(setChannelSelected(int)), -1);
                mychanevents.push_back(action);
            }
        }

        show_events->exec(channels->mapToGlobal(point));
    }
}

void Client::addChannel(const QString &name, int id)
{
    channelNames.insert(id, name);
    channelByNames.insert(name.toLower(),id);
    channels->addItem(new QIdListWidgetItem(id, greychatot, name));
}

void Client::channelNameChanged(int id, const QString &name)
{
    QString old = channelNames.value(id);
    channelNames[id] = name;
    channelByNames.remove(old.toLower());
    channelByNames[name.toLower()] = id;

    for(int i = 0; i < channels->count(); i++) {
        if (channels->item(id)->text() == old) {
            channels->item(id)->setText(name);
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

    QString chanName = channelNames.take(id);
    channelByNames.remove(chanName.toLower());
}

void Client::leaveChannelR(int index)
{
    if (mychannels.size() == 1)
        return;

    if (!channelByNames.contains(mainChat->tabText(index).toLower())) {
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

    int id = channelByNames.value(mainChat->tabText(index).toLower());

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

void Client::join(const QString& text)
{
    if (channelByNames.contains(text.toLower())) {
        int id = channelByNames.value(text.toLower());

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
    relay().notify(NetworkCli::SpectateBattle, qint32(id));
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

    if(pmFlashing && !pmDisabled)
        activateWindow(); // activate po window when pm recieved

    if (mypms.contains(id)) {
        return;
    }

    PMWindow *p = new PMWindow(id, ownName(), name(id), "", auth(id) >= 4, pmDisabled);
    p->setParent(this);
    p->setWindowFlags(Qt::Window);
    p->show();

    connect(p, SIGNAL(challengeSent(int)), this, SLOT(seeInfo(int)));
    connect(p, SIGNAL(messageEntered(int,QString)), &relay(), SLOT(sendPM(int,QString)));
    connect(this, SIGNAL(PMDisabled(bool)), p, SLOT(disablePM(bool)));
    connect(p, SIGNAL(messageEntered(int,QString)), this, SLOT(registerPermPlayer(int)));
    connect(p, SIGNAL(destroyed(int)), this, SLOT(removePM(int)));
    connect(p, SIGNAL(ignore(int,bool)), this, SLOT(ignore(int, bool)));

    mypms[id] = p;
}

void Client::registerPermPlayer(int id)
{
    pmedPlayers.insert(id);
}

void Client::goAway(int away)
{
    relay().goAway(away);
    goaway->setChecked(away);
}

void Client::showTeam(bool b)
{
    QSettings s;
    s.setValue("show_team", b);
    relay().notify(NetworkCli::ShowTeamChange, b);
}

void Client::showTimeStamps(bool b)
{
    QSettings s;
    s.setValue("show_timestamps", b);
    showTS = b;
}

void Client::showTimeStamps2(bool b)
{
    QSettings s;
    s.setValue("show_timestamps2", b);
}

void Client::pmFlash(bool b)
{
    QSettings s;
    s.setValue("pm_flashing", b);
    pmFlashing = b;
}

void Client::togglePM(bool b)
{
    QSettings s;
    s.setValue("pm_disabled", b);
    pmDisabled = b;
    emit PMDisabled(b);
}


void Client::ignoreServerVersion(bool b)
{
    QSettings s;
    if (b) {
        s.setValue("ignore_version_" + serverVersion, true);
    } else {
        s.remove("ignore_version_" + serverVersion);
    }
}

void Client::enableLadder(bool b)
{
    QSettings s;
    s.setValue("enable_ladder", b);
    relay().notify(NetworkCli::LadderChange, b);
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
    QSettings s;
    if (selectedChannel != -1) {
        s.beginGroup("channelevents");
        s.beginGroup(channelNames.value(selectedChannel));
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
        s.beginGroup(channelNames.value(selectedChannel));
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
        QSettings globals;
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
    showPlayerEvents(b, IdleEvent, "show_player_events_idle");
}

void Client::showBattleEvents(bool b)
{
    showPlayerEvents(b, BattleEvent, "show_player_events_battle");
}

void Client::showChannelEvents(bool b)
{
    showPlayerEvents(b, ChannelEvent, "show_player_events_channel");
}

void Client::showTeamEvents(bool b)
{
    showPlayerEvents(b, TeamEvent, "show_player_events_team");
}

void Client::toggleAutoJoin(bool b)
{
    QString name = channelNames.value(selectedChannel);
    if (b) {
        autojoinChannels.append(name);
    } else {
        autojoinChannels.removeAll(name);
    }
    QSettings s;
    s.setValue(QString("autojoinChannels/%1").arg(serverName), autojoinChannels.join("*"));
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

    myRanking->init(name(id), player(id).tier);
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
    connect(&relay(), SIGNAL(banListReceived(QString,QString)), myCP, SLOT(addNameToBanList(QString, QString)));
    connect(&relay(), SIGNAL(tbanListReceived(QString,QString,int)), myCP, SLOT(addNameToTBanList(QString, QString,int)));
    connect(myCP, SIGNAL(getBanList()), &relay(), SLOT(getBanList()));
    connect(myCP, SIGNAL(getTBanList()), &relay(), SLOT(getTBanList()));
    connect(myCP, SIGNAL(banRequested(QString)), SLOT(requestBan(QString)));
    connect(myCP, SIGNAL(unbanRequested(QString)), &relay(), SLOT(CPUnban(QString)));
    connect(myCP, SIGNAL(tunbanRequested(QString)), &relay(), SLOT(CPTUnban(QString)));
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

    myBattleFinder = new BattleFinder(this);
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

void Client::PMReceived(int id, QString pm)
{
    time_t current = time(NULL);
    double difference = difftime(lastAutoPM, current);
    if (mypms.contains(id)) {
        if(pmDisabled) { // We're avoiding that people that was actually chatting with the user continue talking avoiding the Disable PM =-)
            if((difference > 6) || (difference < -6)) {
                myrelay.sendPM(id, "This player is currently ignoring all private messages.");
                lastAutoPM = current;
            }
            return;
        }
        registerPermPlayer(id);
        mypms[id]->printLine(pm);
    } else {
        if(pmDisabled) {
            if ((difference > 6) || (difference < -6)) {
                myrelay.sendPM(id, "This player is currently ignoring all private messages.");
                lastAutoPM = current;
            }
            return;
        } else {
            if (!playerExist(id) || myIgnored.contains(id)) {
                return;
            }
            if (!mypms.contains(id)) {
                startPM(id);
            }
            registerPermPlayer(id);
            mypms[id]->printLine(pm);
        }
    }
}


void Client::removePM(int id)
{
    mypms.remove(id);
}

void Client::loadTeam()
{
    loadTTeamDialog(*team(), this, SLOT(changeTeam()));
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
            if (s1.length() > 0) {
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

void Client::channelCommandReceived(int command, int channel, QDataStream *stream)
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
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setObjectName("MainChat");

    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &TeamBuilder"),this,SLOT(openTeamBuilder()),Qt::CTRL+Qt::Key_T);

    w->addStyleMenu(menuBar);
    w->addThemeMenu(menuBar);

    QMenu * menuActions = menuBar->addMenu(tr("&Options"));
    goaway = menuActions->addAction(tr("&Idle"));
    goaway->setCheckable(true);
    goaway->setChecked(this->away());
    connect(goaway, SIGNAL(triggered(bool)), this, SLOT(goAwayB(bool)));

    QSettings s;

    QAction * show = menuActions->addAction(tr("&Show team"));
    show->setCheckable(true);
    connect(show, SIGNAL(triggered(bool)), SLOT(showTeam(bool)));
    show->setChecked(s.value("show_team").toBool());

    QAction * ladd = menuActions->addAction(tr("Enable &ladder"));
    ladd->setCheckable(true);
    connect(ladd, SIGNAL(triggered(bool)), SLOT(enableLadder(bool)));
    ladd->setChecked(s.value("enable_ladder").toBool());

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
    action->setChecked(s.value("show_player_events_idle").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showIdleEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= IdleEvent;
    } else {
        showPEvents &= ~IdleEvent;
    }
    myevents.push_back(action);

    action = show_events->addAction(tr("Enable battle events"));
    action->setCheckable(true);
    action->setChecked(s.value("show_player_events_battle").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showBattleEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= BattleEvent;
    } else {
        showPEvents &= ~BattleEvent;
    }
    myevents.push_back(action);

    action = show_events->addAction(tr("Enable channel events"));
    action->setCheckable(true);
    action->setChecked(s.value("show_player_events_channel").toBool());
    connect(action, SIGNAL(triggered(bool)), SLOT(showChannelEvents(bool)));
    if(action->isChecked()) {
        showPEvents |= ChannelEvent;
    } else {
        showPEvents &= ~ChannelEvent;
    }
    myevents.push_back(action);

    action = show_events->addAction(tr("Enable team change events"));
    action->setCheckable(true);
    action->setChecked(s.value("show_player_events_team").toBool());
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
    show_ts->setChecked(s.value("show_timestamps").toBool());
    showTS = show_ts->isChecked();

    QAction * show_ts2 = menuActions->addAction(tr("Enable timestamps in &PMs"));
    show_ts2->setCheckable(true);
    connect(show_ts2, SIGNAL(triggered(bool)), SLOT(showTimeStamps2(bool)));
    show_ts2->setChecked(s.value("show_timestamps2").toBool());

    QAction * pm_flash = menuActions->addAction(tr("Make new PMs &flash"));
    pm_flash->setCheckable(true);
    connect(pm_flash, SIGNAL(triggered(bool)), SLOT(pmFlash(bool)));
    pm_flash->setChecked(s.value("pm_flashing").toBool());

    QAction * pm_disable = menuActions->addAction(tr("Disable PMs"));
    pm_disable->setCheckable(true);
    connect(pm_disable, SIGNAL(triggered(bool)), SLOT(togglePM(bool)));
    pm_disable->setChecked(s.value("pm_disabled").toBool());

    QAction *sortByTier = menuActions->addAction(tr("Sort players by &tiers"));
    sortByTier->setCheckable(true);
    connect(sortByTier, SIGNAL(triggered(bool)), SLOT(sortPlayersCountingTiers(bool)));
    sortByTier->setChecked(s.value("sort_players_by_tier").toBool());
    sortBT = sortByTier->isChecked();

    QAction *sortByAuth = menuActions->addAction(tr("Sort players by auth &level"));
    sortByAuth->setCheckable(true);
    connect(sortByAuth, SIGNAL(triggered(bool)), SLOT(sortPlayersByAuth(bool)));
    sortByAuth->setChecked(s.value("sort_players_by_auth").toBool());
    sortBA = sortByAuth->isChecked();

    QAction *sortChannelsName = menuActions->addAction(tr("Sort channels by name"));
    sortChannelsName->setCheckable(true);
    sortChannelsName->setChecked(s.value("sort_channels_by_name").toBool());
    connect(sortChannelsName, SIGNAL(triggered(bool)), SLOT(sortChannelsToggle(bool)));
    sortCBN = sortChannelsName->isChecked();

    QAction *list_right = menuActions->addAction(tr("Move player list to &right"));
    list_right->setCheckable(true);
    connect(list_right, SIGNAL(triggered(bool)), SLOT(movePlayerList(bool)));
    list_right->setChecked(s.value("user_list_at_right").toBool());

    mytiermenu = menuBar->addMenu(tr("&Tiers"));

    QMenu *battleMenu = menuBar->addMenu(tr("&Battle options", "Menu"));
    QAction * saveLogs = battleMenu->addAction(tr("Save &Battle Logs"));
    saveLogs->setCheckable(true);
    connect(saveLogs, SIGNAL(triggered(bool)), SLOT(saveBattleLogs(bool)));
    saveLogs->setChecked(LogManager::obj()->logsType(BattleLog));

    battleMenu->addAction(tr("Change &log folder ..."), this, SLOT(changeBattleLogFolder()));

    QAction *playMusic = battleMenu->addAction(tr("&Enable sounds (Testing! Remove if problems with the sim)"));
    playMusic->setCheckable(true);
    connect(playMusic, SIGNAL(triggered(bool)), SLOT(playMusic(bool)));
    playMusic->setChecked(s.value("play_battle_music").toBool());

    battleMenu->addAction(tr("Change &music folder ..."), this, SLOT(changeMusicFolder()));

    QAction *animateHpBar = battleMenu->addAction(tr("Animate HP Bar"));
    animateHpBar->setCheckable(true);
    connect(animateHpBar, SIGNAL(triggered(bool)), SLOT(animateHpBar(bool)));
    animateHpBar->setChecked(s.value("animate_hp_bar").toBool());

    QAction *oldStyleButtons = battleMenu->addAction(tr("Old school buttons"));
    oldStyleButtons->setCheckable(true);
    connect(oldStyleButtons, SIGNAL(triggered(bool)), SLOT(changeButtonStyle(bool)));
    oldStyleButtons->setChecked(s.value("old_attack_buttons").toBool());

    QAction *oldBattleWindow = battleMenu->addAction(tr("Old battle window"));
    oldBattleWindow->setCheckable(true);
    connect(oldBattleWindow, SIGNAL(triggered(bool)), SLOT(changeBattleWindow(bool)));
    oldBattleWindow->setChecked(s.value("old_battle_window", true).toBool());

    QAction *dontUseNicknames = battleMenu->addAction(tr("Don't show Pokemon Nicknames"));
    dontUseNicknames->setCheckable(true);
    connect(dontUseNicknames, SIGNAL(triggered(bool)), SLOT(changeNicknames(bool)));
    dontUseNicknames->setChecked(s.value("use_pokemon_names").toBool());

    mymenubar = menuBar;

    return menuBar;
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


void Client::askForPass(const QString &salt) {

    QString pass;
    QStringList warns;
    bool ok = wallet.retrieveUserPassword(relay().getIp(), serverName, myteam->trainerNick(), salt, pass, warns);
    if (!warns.empty()) warns.prepend(""); // for join()

    /* Create a dialog for password input */
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Enter your password"));
    QVBoxLayout* layout = new QVBoxLayout;
    // Label
    layout->addWidget(new QLabel(tr("Enter the password for your current name.\n"
                                    "If you don't have it, the name you have chosen might be already taken."
                                    " Choose different name.\n"
                                    "\nIt is advised to use a slightly different password for each server."
                                    " (The server only sees the encrypted form of the pass, but still...)")
                                 + warns.join("\n")));
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
        return;
    }
    pass = passEdit->text();
    if (savePass->isChecked()) {
        // TODO: ipv6 support in the future
        wallet.saveUserPassword(relay().getIp(), serverName, myteam->trainerNick(), salt, pass);
    }


    QString hash = QString(md5_hash(md5_hash(pass.toAscii())+salt.toAscii()));
    relay().notify(NetworkCli::AskForPass, hash);
}

void Client::serverPass(const QString &salt) {

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
    QString hash = QString(md5_hash(md5_hash(pass.toAscii())+salt.toAscii()));
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

void Client::changeMusicFolder()
{
    QSettings s;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Battle Music Directory"), s.value("battle_music_directory").toString());

    if (dir != "") {
        s.setValue("battle_music_directory", dir + "/");
    }
}

void Client::changeBattleLogFolder()
{
    QSettings s;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Battle Logs Directory"),
                                                    QDir::home().absoluteFilePath(LogManager::obj()->getDirectoryForType(BattleLog)));

    if (dir.isNull()) {
        return;
    }

    LogManager::obj()->changeDirectoryForType(BattleLog, dir);
}

void Client::changeButtonStyle(bool old)
{
    QSettings s;
    s.setValue("old_attack_buttons",old);
}

void Client::changeBattleWindow(bool old)
{
    QSettings s;
    s.setValue("old_battle_window",old);
}

void Client::changeNicknames(bool old)
{
    QSettings s;
    s.setValue("use_pokemon_names",old);
}

void Client::saveBattleLogs(bool save)
{
    LogManager::obj()->changeLogSaving(BattleLog, save);
}

void Client::animateHpBar(bool save)
{
    QSettings s;
    s.setValue("animate_hp_bar", save);
}

void Client::playMusic(bool save)
{
    QSettings s;
    s.setValue("play_battle_music", save);
}

void Client::spectatingBattleMessage(int battleId, const QByteArray &command)
{
    if (mySpectatingBattles.contains(battleId)) {
        mySpectatingBattles[battleId]->receiveInfo(command);
    }
}

bool Client::battling() const
{
    return mybattles.size() > 0;
}

void Client::versionDiff(const QString &a, const QString &b)
{
    serverVersion = a;

    if (a != b) {
        printHtml(toColor(tr("Your client version (%2) doesn't match with the server's (%1).").arg(a,b), QColor("#e37800")));

        if (b.compare(a) < 0) {
            QSettings s;

            if (s.contains("ignore_version_" + serverVersion))
                return;

            QMessageBox *update = new QMessageBox(QMessageBox::Information, tr("Old Version"), tr("Your version is older than the server's, there might be some things you can't do.\n\nhttp://www.pokemon-online.eu/downloads.php for updates."),QMessageBox::Ok  | QMessageBox::Ignore,NULL, Qt::Window);
            int result = update->exec();

            if (result & QMessageBox::Ignore)
                ignoreServerVersion(true);
        }
    }
}

void Client::serverNameReceived(const QString &sName)
{
    QMainWindow* mainwindow = qobject_cast<QMainWindow*>(parent());
    QString titlebase = mainwindow->windowTitle();
    if (serverName.size() > 0) {
        // chop the current title to make room for the new name
        titlebase.chop(3 + serverName.size());
        // automatically copy settings to new name
        QSettings settings;
        QVariant ajc = settings.value(QString("autojoinChannels/%1").arg(serverName));
        settings.setValue(QString("autojoinChannels/%1").arg(sName), ajc);
    }
    serverName = sName;
    if (serverName.size() > 0) {
        QString newTitle = titlebase + " - " + serverName;
        mainwindow->setWindowTitle(newTitle);
    }
}

void Client::announcementReceived(const QString &ann)
{
    if (ann.length() == 0)
        return;

    announcement->setText(ann);
    announcement->setAlignment(Qt::AlignCenter);
    announcement->show();
}

void Client::tierListReceived(const QByteArray &tl)
{
    mytiermenu->clear();
    mytiers.clear();
    tierList.clear();
    tierRoot.buildFromRaw(tl);
    tierList = tierRoot.getTierList();

    if (tierList.empty()) {
        tierRoot.subNodes.push_back(new TierNode("All"));
        tierList = tierRoot.getTierList();
    }

    mytiers = tierRoot.buildMenu(mytiermenu, this);
    changeTierChecked(player(ownId()).tier);

    QSettings s;

    if (s.value("sort_players_by_tier").toBool()) {
        foreach(Channel *c, mychannels)
            c->sortAllPlayersByTier();
    }
    emit tierListFormed(tierList);
}

void Client::sortPlayersCountingTiers(bool byTier)
{
    sortBT = byTier;
    QSettings s;
    s.setValue("sort_players_by_tier", sortBT);

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
    QSettings s;
    s.setValue("sort_players_by_auth", sortBA);

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
    QSettings s;
    s.setValue("user_list_at_right", right);

    QWidget *mytab = playersW->parentWidget()->parentWidget();
    QWidget *chatcontainer = mainChat->parentWidget();
    QSplitter *splitter = reinterpret_cast<QSplitter*>( mytab->parentWidget() );
    if (right) {
        splitter->addWidget(mytab);
    } else {
        splitter->addWidget(chatcontainer);
    }
}

void Client::changeTierChecked(const QString &newtier)
{
    foreach(QAction *a, mytiers) {
        if (a->text() == newtier) {
            a->setChecked(true);
        } else {
            a->setChecked(false);
        }
    }
}

void Client::changeTier()
{
    QAction *a = (QAction*)sender();

    relay().notify(NetworkCli::TierSelection, a->text());
    changeTierChecked(player(ownId()).tier);
}

bool Client::playerExist(int id) const
{
    return myplayersinfo.contains(id);
}

PlayerInfo Client::player(int id) const
{
    return myplayersinfo.value(id);
}

BasicInfo Client::info(int id) const
{
    if (myplayersinfo.contains(id))
        return myplayersinfo[id].team;
    else
        return BasicInfo();
}

void Client::seeInfo(QTreeWidgetItem *it)
{
    if (dynamic_cast<QIdTreeWidgetItem*>(it))
        seeInfo(((QIdTreeWidgetItem*)(it))->id());
}

void Client::seeInfo(int id)
{
    if (playerExist(id))
    {
        BaseChallengeWindow *mychallenge = new ChallengeWindow(player(id));
        connect(mychallenge, SIGNAL(challenge(int)), SLOT(sendChallenge(int)));
        connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
        connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));

        mychallenges.insert(mychallenge);
    }
}

void Client::seeChallenge(const ChallengeInfo &c)
{
    if (playerExist(c))
    {
        if (busy()) {
            /* Warns the server that we are too busy to accept the challenge */
            ChallengeInfo d = c;
            d.dsc = ChallengeInfo::Busy;
            relay().sendChallengeStuff(d);
        } else {
            BaseChallengeWindow *mychallenge = new ChallengedWindow(player(c),c);
            connect(mychallenge, SIGNAL(challenge(int)), SLOT(acceptChallenge(int)));
            connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
            connect(mychallenge, SIGNAL(cancel(int)), SLOT(refuseChallenge(int)));
            connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
            mychallenge->activateWindow();
            mychallenges.insert(mychallenge);
        }
    }
}

void Client::battleStarted(int battleId, int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    cancelFindBattle(false);
    BattleWindow * mybattle = new BattleWindow(battleId, player(ownId()), player(id), team, conf, this);
    connect(this, SIGNAL(destroyed()), mybattle, SLOT(deleteLater()));
    mybattle->setWindowFlags(Qt::Window);
    mybattle->client() = this;
    mybattle->show();
    mybattle->activateWindow();

    connect(mybattle, SIGNAL(forfeit(int)), SLOT(forfeitBattle(int)));
    connect(mybattle, SIGNAL(battleCommand(int, BattleChoice)), &relay(), SLOT(battleCommand(int, BattleChoice)));
    connect(mybattle, SIGNAL(battleMessage(int, QString)), &relay(), SLOT(battleMessage(int, QString)));
    connect(this, SIGNAL(destroyed()), mybattle, SLOT(close()));
    //connect(this, SIGNAL(musicPlayingChanged(bool)), mybattle, SLOT(playMusic(bool)));

    mybattles[battleId] = mybattle;

    battleStarted(battleId, ownId(), id);
}

void Client::battleStarted(int bid, int id1, int id2)
{
    myplayersinfo[id1].flags |= PlayerInfo::Battling;
    myplayersinfo[id2].flags |= PlayerInfo::Battling;

    battles.insert(bid, Battle(id1, id2));
    foreach(Channel *c, mychannels) {
        c->battleStarted(bid, id1, id2);
    }
}


void Client::battleReceived(int battleid, int id1, int id2)
{
    if (battles.contains(battleid)) {
        return;
    }

    myplayersinfo[id1].flags |= PlayerInfo::Battling;
    myplayersinfo[id2].flags |= PlayerInfo::Battling;

    battles.insert(battleid, Battle(id1, id2));

    updateState(id1);
    updateState(id2);
}

void Client::watchBattle(int battleId, const BattleConfiguration &conf)
{
    QSettings s;

    BaseBattleWindowInterface *battle;

    {
        BaseBattleWindow *battlew = new BaseBattleWindow(player(conf.ids[0]), player(conf.ids[1]), conf, ownId(), this);
        battlew->client() = this;
        battle = battlew;
    }

    battle->setWindowFlags(Qt::Window);
    battle->show();

    connect(this, SIGNAL(destroyed()), battle, SLOT(close()));
    connect(battle, SIGNAL(closedBW(int)), SLOT(stopWatching(int)));
    connect(battle, SIGNAL(battleMessage(int, QString)), &relay(), SLOT(battleMessage(int, QString)));

    battle->battleId() = battleId;
    mySpectatingBattles[battleId] = battle;
}

void Client::stopWatching(int battleId)
{
    if (mySpectatingBattles.contains(battleId)) {
        mySpectatingBattles[battleId]->close();
        mySpectatingBattles.remove(battleId);
        relay().notify(NetworkCli::SpectatingBattleFinished, qint32(battleId));
    }
}

void Client::forfeitBattle(int id)
{
    relay().sendBattleResult(id, Forfeit);
    removeBattleWindow(id);
}

void Client::battleFinished(int battleid, int res, int winner, int loser)
{
    /* On old servers battleid is always 0 so you don't want to forfeit that battle ... */
    if ((res == Close || res == Forfeit) && (battleid != 0 || (winner == ownId() || loser == ownId())))
        removeBattleWindow(battleid);

    foreach(Channel *c, mychannels) {
        c->battleEnded(battleid, res, winner, loser);
    }

    battles.remove(battleid);

    if (myplayersinfo.contains(winner))
        myplayersinfo[winner].flags &= 0xFF ^ PlayerInfo::Battling;
    if (myplayersinfo.contains(loser))
        myplayersinfo[loser].flags &= 0xFF ^ PlayerInfo::Battling;

    foreach(Battle b, battles) {
        if (myplayersinfo.contains(winner) && (b.id1 == winner || b.id2 == winner)) {
            myplayersinfo[winner].flags |= PlayerInfo::Battling;
        }
        if (myplayersinfo.contains(loser) && (b.id1 == loser || b.id2 == loser)) {
            myplayersinfo[loser].flags |= PlayerInfo::Battling;
        }
    }

    updateState(winner);
    updateState(loser);
}

void Client::battleCommand(int battleid, const QByteArray &command)
{
    if (!mybattles.contains(battleid))
        return;

    mybattles[battleid]->receiveInfo(command);
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
        return info(id).name;
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
            BaseChallengeWindow *b;
            if (c.desc() == ChallengeInfo::Refused) {
                printLine(tr("%1 refused your challenge.").arg(name(c)));
                while ( (b = getChallengeWindow(c)) ) {
                    closeChallengeWindow(b);
                }
            } else if (c.desc() == ChallengeInfo::Busy) {
                printLine(tr("%1 is busy.").arg(name(c)));
                while ( (b = getChallengeWindow(c)) ) {
                    closeChallengeWindow(b);
                }
            } else if (c.desc() == ChallengeInfo::Cancelled) {
                printLine(tr("%1 cancelled their challenge.").arg(name(c)));
                while ( (b = getChallengeWindow(c)) ) {
                    closeChallengeWindow(b);
                }
            } else if (c.desc() == ChallengeInfo::InvalidTeam) {
                printLine(tr("%1 has an invalid team.").arg(name(c)));
                while ( (b = getChallengeWindow(c)) ) {
                    closeChallengeWindow(b);
                }
            } else if (c.desc() == ChallengeInfo::InvalidGen) {
                printLine(tr("%1 has a different gen than yours.").arg(name(c)));
                while ( (b = getChallengeWindow(c)) ) {
                    closeChallengeWindow(b);
                }
            }
        }
    }
}

void Client::awayChanged(int id, bool away)
{
    if (away) {
        printLine(IdleEvent, id, tr("%1 is idling.").arg(name(id)));
    } else {
        printLine(IdleEvent, id, tr("%1 is active and ready for battles.").arg(name(id)));
    }

    playerInfo(id).changeState(PlayerInfo::Away, away);
    updateState(id);
}

bool Client::busy() const
{
    return away();
}

bool Client::away() const
{
    return player(ownId()).away();
}

void Client::acceptChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Accepted, id));
}

void Client::refuseChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Refused, id));
}

QString Client::tier(int player) const
{
    return this->player(player).tier;
}

void Client::sendChallenge(int id)
{
    QSettings s;

    quint32 clauses = 0;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses |= s.value("clause_"+ChallengeInfo::clause(i)).toBool() ? (1 << i) : 0;
    }

    int mode = s.value("challenge_with_doubles").toInt();

    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Sent, id, clauses, mode));
}

void Client::clearChallenge()
{
    mychallenges.remove(dynamic_cast<BaseChallengeWindow*>(sender()));
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

    FullInfo f = {*team(), s.value("enable_ladder").toBool(), s.value("show_team").toBool(), s.value("trainer_color").value<QColor>()};
    relay().login(f);
}

void Client::disconnected()
{
    printLine(tr("Disconnected from Server!"));

    isConnected = false;
    myregister->setText(tr("&Reconnect"));
    myregister->setEnabled(true);
}

TrainerTeam* Client::team()
{
    return myteam;
}

Analyzer &Client::relay()
{
    return myrelay;
}

void Client::playerLogin(const PlayerInfo& p)
{
    _mid = p.id;
    mynick = p.team.name;
    myplayersinfo[p.id] = p;
    mynames[p.team.name] = p.id;

    if (serverName.size() > 0) {
        QSettings settings;
        autojoinChannels = settings.value(QString("autojoinChannels/%1").arg(serverName)).toString().split("*");
        autojoinChannels.removeAll("");
        foreach (QString channel, autojoinChannels) {
            join(channel);
        }
    }
}

void Client::playerLogout(int id)
{
    /* removes the item in the playerlist */
    removePlayer(id);
}


void Client::removePlayer(int id)
{
    QString name = info(id).name;

    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(id))
            c->playerLogOut(id);
    }

    if (myIgnored.contains(id))
        removeIgnore(id);

    myplayersinfo.remove(id);
    pmedPlayers.remove(id);
    fade.remove(id);

    QHash<int, PMWindow*>::iterator pm = mypms.find(id);
    if (pm != mypms.end()) {
        pm.value()->disable();
        disabledpms[name] = pm.value();
        mypms.erase(pm);
    }

    /* Name removed... Only if no one took it since the 10 minutes we never saw the guy */
    if (mynames.value(name) == id)
        mynames.remove(name);
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

QString Client::authedNick(int id) const
{
    PlayerInfo p = player(id);

    QString nick = p.team.name;

    return nick;
}

void Client::playerReceived(const PlayerInfo &p)
{
    if (myplayersinfo.contains(p.id)) {
        /* It's not sync perfectly, so someone who relogs can happen, that's why we do that test */
        if (mynames.value(p.team.name) == p.id)
            mynames.remove(info(p.id).name);
        myplayersinfo.remove(p.id);
    }

    myplayersinfo.insert(p.id, p);
    refreshPlayer(p.id);

    changeName(p.id, p.team.name);

    if (p.id == ownId()) {
        changeTierChecked(p.tier);
    }

    foreach(Channel *c, mychannels) {
        if (c->hasPlayer(p.id))
            c->playerReceived(p.id);
        else
            c->changeName(p.id, p.team.name); /* Even if the player isn't in the channel, someone in the channel could be battling him, ... */
    }
    QHash<QString, PMWindow*>::iterator pm = disabledpms.find(name(p.id));
    if (pm != disabledpms.end()) {
        PMWindow *window = pm.value();
        disabledpms.erase(pm);
        mypms[p.id] = window;
        window->reuse(p.id);
    }
}

void Client::changeName(int player, const QString &name)
{
    mynames[name] = player;

    if (mypms.contains(player)) {
        mypms[player]->changeName(name);
    }

    if (player == ownId()) {
        foreach(PMWindow *p, mypms) {
            p->changeSelf(name);
        }
    }
}

void Client::teamChanged(const PlayerInfo &p) {
    if (name(p.id) != p.team.name) {
        printLine(TeamEvent, p.id, tr("%1 changed teams and is now known as %2.").arg(name(p.id), p.team.name));
        if (p.id == ownId()) {
            mynick = p.team.name;
        }
    } else {
        printLine(TeamEvent, p.id, tr("%1 changed teams.").arg(name(p.id)));
    }
    playerReceived(p);
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
    if (mynames.contains(name)) {
        return mynames[name];
    } else {
        return -1;
    }
}

void Client::closeChallengeWindow(BaseChallengeWindow *b)
{
    mychallenges.remove(b);
    b->forcedClose();
}

BaseChallengeWindow * Client::getChallengeWindow(int player)
{
    foreach(BaseChallengeWindow *c, mychallenges) {
        if (c->id() == player)
            return c;
    }

    return NULL;
}

void Client::openTeamBuilder()
{
    if (myteambuilder) {
        myteambuilder->activateWindow();
        myteambuilder->raise();
        return;
    }

    myteambuilder = new QMainWindow();

    TeamBuilder *t = new TeamBuilder(myteam);
    myteambuilder->resize(t->size());
    myteambuilder->setCentralWidget(t);
    myteambuilder->show();
    myteambuilder->setAttribute(Qt::WA_DeleteOnClose, true);
    myteambuilder->setMenuBar(t->createMenuBar((MainEngine*)parent()));
    t->setTierList(tierList);

    connect(this, SIGNAL(destroyed()), myteambuilder, SLOT(close()));
    connect(t, SIGNAL(done()), this, SLOT(changeTeam()));
    connect(t, SIGNAL(done()), myteambuilder, SLOT(close()));
    connect(this, SIGNAL(tierListFormed(const QStringList &)), t, SLOT(setTierList(const QStringList&)));
}

void Client::changeTeam()
{
    if (battling() && myteam->trainerNick() != mynick) {
        printLine(tr("You can't change teams while battling, so your nick was kept."));
        myteam->setTrainerNick(mynick);
    }
    cancelFindBattle(false);
    relay().sendTeam(*myteam);
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
        relay().notify(NetworkCli::CPTBan, name, qint32(time));
    } else {
        tempban(id(name), time);
    }
}

void Client::ignore(int id)
{
    if (myIgnored.contains(id))
        return;
    printLine(id, tr("You ignored %1.").arg(name(id)));
    myIgnored.append(id);
    updateState(id);
}

void Client::ignore(int id, bool ign)
{
    if (ign)
        ignore(id);
    else
        removeIgnore(id);
}

void Client::removeIgnore(int id)
{
    if (!myIgnored.contains(id))
        return;
    printLine(id, tr("You stopped ignoring %1.").arg(name(id)));
    myIgnored.removeOne(id);
    updateState(id);
}

void Client::printHtml(const QString &html)
{
    foreach(Channel *c, mychannels)
        c->printHtml(html, false);
}

void Client::printLine(const QString &line)
{
    if (mychannels.size() == 0)
        return;

    foreach(Channel *c, mychannels)
        c->printLine(line,false, false);
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
            c->printLine(line, false);
    }
}

/**********************************************************/
/****************** BATTLE FINDER *************************/
/**********************************************************/

BattleFinder::BattleFinder(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QSettings s;

    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setSpacing(10);
    setWindowFlags(Qt::Window);

    QPushButton *ok, *cancel;
    ml->addWidget(rated = new QCheckBox(tr("Force rated battles")));
    ml->addWidget(sameTier = new QCheckBox(tr("Force same tier")));
    QHBoxLayout *sub2 = new QHBoxLayout();
    ml->addLayout(sub2);
    sub2->addWidget(rangeOn = new QCheckBox(tr("Only battle players with a max rating difference of ")));
    sub2->addWidget(range = new QLineEdit());
    ml->addWidget(mode = new QComboBox());
    mode->addItems(QStringList() << tr("Singles") << tr("Doubles") << tr("Triples") /*<< tr("Rotation")*/);

    QHBoxLayout *hl = new QHBoxLayout();
    ml->addLayout(hl);
    hl->addWidget(ok = new QPushButton(tr("Find Battle")));
    hl->addWidget(cancel = new QPushButton(tr("Cancel")));

    range->setMaximumWidth(35);

    rated->setChecked(s.value("find_battle_force_rated").toBool());
    sameTier->setChecked(s.value("find_battle_same_tier").toBool());
    mode->setCurrentIndex(s.value("find_battle_mode").toInt());
    rangeOn->setChecked(s.value("find_battle_range_on").toBool());
    range->setText(QString::number(s.value("find_battle_range").toInt()));
    changeEnabled();

    connect(rated, SIGNAL(toggled(bool)), SLOT(changeEnabled()));
    connect(sameTier, SIGNAL(toggled(bool)), SLOT(changeEnabled()));

    connect(ok, SIGNAL(clicked()), SLOT(close()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));
    connect(ok, SIGNAL(clicked()), SLOT(throwChallenge()));
}

void BattleFinder::changeEnabled()
{
}

void BattleFinder::throwChallenge()
{
    FindBattleData d;
    d.rated = rated->isChecked();
    d.sameTier = sameTier->isChecked();
    d.range = std::max(range->text().toInt(), 100);
    d.ranged = rangeOn->isChecked();
    d.mode = mode->currentIndex();

    QSettings s;

    s.setValue("find_battle_force_rated", d.rated);
    s.setValue("find_battle_same_tier", d.sameTier);
    s.setValue("find_battle_range_on", d.ranged);
    s.setValue("find_battle_range", d.range);
    s.setValue("find_battle_mode", d.mode);

    emit findBattle(d);
}

