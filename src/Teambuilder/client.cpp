#include "client.h"
#include "mainwindow.h"
#include "challenge.h"
#include "teambuilder.h"
#include "battlewindow.h"
#include "basebattlewindow.h"
#include "pmwindow.h"
#include "controlpanel.h"
#include "ranking.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/functions.h"
#include "../PokemonInfo/pokemonstructs.h"


Client::Client(TrainerTeam *t, const QString &url , const quint16 port) : myteam(t), myrelay(), findingBattle(false)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    mychallenge = NULL;
    mybattle = NULL;
    myteambuilder = NULL;
    resize(800, 600);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QTreeWidget(), 0, 0, 4, 1, Qt::AlignLeft);
    layout->addWidget(announcement = new QLabel(), 0, 1);
    announcement->setObjectName("Announcement");
    announcement->setOpenExternalLinks(true);
    announcement->setWordWrap(true);
    announcement->hide();
    layout->addWidget(mychat = new QScrollDownTextEdit(), 1, 1);
    mychat->setObjectName("MainChat");
    layout->addWidget(myline = new QLineEdit(), 2, 1);
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout,3,1);
    buttonsLayout->addWidget(findMatch = new QPushButton(tr("&Find Battle")));
    buttonsLayout->addWidget(myregister = new QPushButton(tr("&Register")));
    buttonsLayout->addWidget(myexit = new QPushButton(tr("&Exit")));
    buttonsLayout->addWidget(mysender = new QPushButton(tr("&Send")));
    buttonsLayout->addWidget(mybugs = new QPushButton(tr("&Report a Bug")));
    layout->setColumnStretch(1,100);


    myplayers->setMaximumWidth(200);
    myplayers->setContextMenuPolicy(Qt::CustomContextMenu);
    myplayers->setHeaderItem(new QCSTreeWidgetItem(0));
    myplayers->headerItem()->setText(0,"Players");

    QPalette pal = palette();
    pal.setColor(QPalette::AlternateBase, Qt::blue);
    pal.setColor(QPalette::Base, Qt::blue);
    setPalette(pal);
    myregister->setDisabled(true);
    mynick = t->trainerNick();

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myplayers, SIGNAL(itemActivated(QTreeWidgetItem*)), SLOT(seeInfo(QTreeWidgetItem*)));
    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendText()));
    connect(mysender, SIGNAL(clicked()), SLOT(sendText()));
    connect(myregister, SIGNAL(clicked()), SLOT(sendRegister()));
    connect(mybugs, SIGNAL(clicked()), SLOT(bugReport()));
    connect(findMatch, SIGNAL(clicked()), SLOT(openBattleFinder()));

    initRelay();

    relay().connectTo(url, port);

    QFile f("db/client/chat_colors.txt");
    f.open(QIODevice::ReadOnly);

    QStringList colors = QString::fromUtf8(f.readAll()).split('\n');

    if (colors.size() == 0) {
        chatColors << Qt::black << Qt::red << Qt::gray << Qt::darkBlue << Qt::cyan << Qt::darkMagenta << Qt::darkYellow;
    } else {
        foreach (QString c, colors) {
            chatColors << QColor(c);
        }
    }

    statusIcons << QIcon("db/client/Available.png") << QIcon("db/client/Away.png") << QIcon("db/client/Battling.png");
}

void Client::initRelay()
{
    Analyzer *relay = &this->relay();
    connect(relay, SIGNAL(connectionError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(relay, SIGNAL(protocolError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(relay, SIGNAL(connected()), SLOT(connected()));
    connect(relay, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(relay, SIGNAL(messageReceived(QString)), SLOT(messageReceived(QString)));
    connect(relay, SIGNAL(playerReceived(PlayerInfo)), SLOT(playerReceived(PlayerInfo)));
    connect(relay, SIGNAL(teamChanged(PlayerInfo)), SLOT(teamChanged(PlayerInfo)));
    connect(relay, SIGNAL(playerLogin(PlayerInfo)), SLOT(playerLogin(PlayerInfo)));
    connect(relay, SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(relay, SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(relay, SIGNAL(battleStarted(int, int, TeamBattle, BattleConfiguration, bool )),
            SLOT(battleStarted(int, int, TeamBattle, BattleConfiguration, bool)));
    connect(relay, SIGNAL(battleStarted(int,int, int)), SLOT(battleStarted(int, int, int)));
    connect(relay, SIGNAL(battleFinished(int, int,int,int)), SLOT(battleFinished(int, int,int,int)));
    connect(relay, SIGNAL(battleMessage(int, QByteArray)), this, SLOT(battleCommand(int, QByteArray)));
    connect(relay, SIGNAL(passRequired(QString)), SLOT(askForPass(QString)));
    connect(relay, SIGNAL(notRegistered(bool)), myregister, SLOT(setEnabled(bool)));
    connect(relay, SIGNAL(playerKicked(int,int)),SLOT(playerKicked(int,int)));
    connect(relay, SIGNAL(playerBanned(int,int)),SLOT(playerBanned(int,int)));
    connect(relay, SIGNAL(PMReceived(int,QString)), SLOT(PMReceived(int,QString)));
    connect(relay, SIGNAL(awayChanged(int, bool)), SLOT(awayChanged(int, bool)));
    connect(relay, SIGNAL(spectatedBattle(QString,QString,int,bool)), SLOT(watchBattle(QString,QString,int,bool)));
    connect(relay, SIGNAL(spectatingBattleMessage(int,QByteArray)), SLOT(spectatingBattleMessage(int , QByteArray)));
    connect(relay, SIGNAL(spectatingBattleFinished(int)), SLOT(stopWatching(int)));
    connect(relay, SIGNAL(versionDiff(QString, QString)), SLOT(versionDiff(QString, QString)));
    connect(relay, SIGNAL(tierListReceived(QString)), SLOT(tierListReceived(QString)));
    connect(relay, SIGNAL(announcement(QString)), SLOT(announcementReceived(QString)));
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
    return id(ownName());
}

void Client::showContextMenu(const QPoint &requested)
{
    QIdTreeWidgetItem *item = dynamic_cast<QIdTreeWidgetItem*>(myplayers->itemAt(requested));

    if (item && item->id() != 0)
    {
	QMenu *menu = new QMenu(this);

        createIntMapper(menu->addAction(tr("&Challenge")), SIGNAL(triggered()), this, SLOT(seeInfo(int)), item->id());

        createIntMapper(menu->addAction(tr("&View Ranking")), SIGNAL(triggered()), this, SLOT(seeRanking(int)), item->id());
        if (item->id() == ownId()) {
            if (away()) {
                createIntMapper(menu->addAction(tr("Go &Back")), SIGNAL(triggered()), this, SLOT(goAway(int)), false);
            } else {
                createIntMapper(menu->addAction(tr("Go &Away")), SIGNAL(triggered()), this, SLOT(goAway(int)), true);
            }
        } else {
            createIntMapper(menu->addAction(tr("&Send Message")), SIGNAL(triggered()), this, SLOT(startPM(int)), item->id());
            if (player(item->id()).battling())
                createIntMapper(menu->addAction(tr("&Watch Battle")), SIGNAL(triggered()), this, SLOT(watchBattleRequ(int)), item->id());
            if (myIgnored.contains(item->id()))
                createIntMapper(menu->addAction(tr("&Remove Ignore")), SIGNAL(triggered()), this, SLOT(removeIgnore(int)), item->id());
            else
                createIntMapper(menu->addAction(tr("&Ignore")), SIGNAL(triggered()), this, SLOT(ignore(int)), item->id());
        }

        int myauth = ownAuth();

        if (myauth > 0) {
            createIntMapper(menu->addAction(tr("&Control Panel")), SIGNAL(triggered()), this, SLOT(controlPanel(int)), item->id());
        }

        int otherauth = player(item->id()).auth;

        if (otherauth < myauth) {
            menu->addSeparator();            
            createIntMapper(menu->addAction(tr("&Kick")), SIGNAL(triggered()), this, SLOT(kick(int)), item->id());

            /* If you're an admin, you can ban */
            if (myauth >= 2) {
                menu->addSeparator();
                createIntMapper(menu->addAction(tr("&Ban")), SIGNAL(triggered()), this, SLOT(ban(int)), item->id());
            }
        }

	menu->exec(mapToGlobal(requested));
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

void Client::startPM(int id)
{
    if (id == this->id(ownName()) || !playerExist(id)) {
        return;
    }

    activateWindow();

    if (mypms.contains(id)) {    
        return;
    }

    PMWindow *p = new PMWindow(id, ownName(), name(id));
    p->setParent(this);
    p->setWindowFlags(Qt::Window);
    p->show();

    connect(p, SIGNAL(challengeSent(int)), this, SLOT(seeInfo(int)));
    connect(p, SIGNAL(messageEntered(int,QString)), &relay(), SLOT(sendPM(int,QString)));
    connect(p, SIGNAL(destroyed(int)), this, SLOT(removePM(int)));

    mypms[id] = p;
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

void Client::showPlayerEvents(bool b)
{
    QSettings s;
    s.setValue("show_player_events", b);
    showPEvents = b;
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
    connect(myCP, SIGNAL(getBanList()), &relay(), SLOT(getBanList()));
    connect(myCP, SIGNAL(banRequested(QString)), SLOT(requestBan(QString)));
    connect(myCP, SIGNAL(unbanRequested(QString)), &relay(), SLOT(CPUnban(QString)));
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
    relay().notify(NetworkCli::FindMatch,data);
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
    if (!playerExist(id) || myIgnored.contains(id)) {
        return;
    }
    if (!mypms.contains(id)) {
        startPM(id);
    }

    mypms[id]->printLine(pm);
}

void Client::removePM(int id)
{
    mypms.remove(id);
}

void Client::loadTeam()
{
    QSettings settings;
    QString newLocation;

    if (loadTTeamDialog(*myteam, settings.value("team_location").toString(), &newLocation))
    {
        settings.setValue("team_location", newLocation);
        changeTeam();
    }
}

void Client::sendText()
{
    QString text = myline->text().trimmed();
    if (text.length() > 0) {
        QStringList s = text.split('\n');
        foreach(QString s1, s) {
            if (s1.length() > 0) {
                relay().sendMessage(s1);
            }
        }
    }

    myline->clear();
}

QMenuBar * Client::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    menuBar->setObjectName("MainChat");

    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &teamBuilder"),this,SLOT(openTeamBuilder()),Qt::CTRL+Qt::Key_T);
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    for(QStringList::iterator i = style.begin();i!=style.end();i++)
    {
        menuStyle->addAction(*i,w,SLOT(changeStyle()));
    }
    menuStyle->addSeparator();
    menuStyle->addAction(tr("Reload StyleSheet"), w, SLOT(loadStyleSheet()));
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

    QAction *show_events = menuActions->addAction(tr("&Enable player events"));
    show_events->setCheckable(true);
    connect(show_events, SIGNAL(triggered(bool)), SLOT(showPlayerEvents(bool)));
    show_events->setChecked(s.value("show_player_events").toBool());
    showPEvents = show_events->isChecked();

    QAction * show_ts = menuActions->addAction(tr("Enable &timestamps"));
    show_ts->setCheckable(true);
    connect(show_ts, SIGNAL(triggered(bool)), SLOT(showTimeStamps(bool)));
    show_ts->setChecked(s.value("show_timestamps").toBool());
    showTS = show_ts->isChecked();

    QAction * show_ts2 = menuActions->addAction(tr("Enable timestamps in &PMs"));
    show_ts2->setCheckable(true);
    connect(show_ts2, SIGNAL(triggered(bool)), SLOT(showTimeStamps2(bool)));
    show_ts2->setChecked(s.value("show_timestamps2").toBool());

    QAction *sortByTier = menuActions->addAction(tr("Sort players by &tiers"));
    sortByTier->setCheckable(true);
    connect(sortByTier, SIGNAL(triggered(bool)), SLOT(sortPlayersCountingTiers(bool)));
    sortByTier->setChecked(s.value("sort_players_by_tier").toBool());
    sortBT = sortByTier->isChecked();

    mytiermenu = menuBar->addMenu(tr("&Tiers"));

    QMenu *battleMenu = menuBar->addMenu(tr("&Battle Options", "Menu"));
    QAction * saveLogs = battleMenu->addAction(tr("Save &Battle Logs"));
    saveLogs->setCheckable(true);
    connect(saveLogs, SIGNAL(triggered(bool)), SLOT(saveBattleLogs(bool)));
    saveLogs->setChecked(s.value("save_battle_logs").toBool());

    battleMenu->addAction(tr("Change &log folder"), this, SLOT(changeBattleLogFolder()));
/*
    QAction *playMusic = battleMenu->addAction(tr("&Enable sounds (Risky!)"));
    playMusic->setCheckable(true);
    connect(playMusic, SIGNAL(triggered(bool)), SLOT(playMusic(bool)));
    playMusic->setChecked(s.value("play_battle_music").toBool());

    battleMenu->addAction(tr("Change &sound folder"), this, SLOT(changeMusicFolder()));
*/
    QAction *animateHpBar = battleMenu->addAction(tr("Animate HP Bar"));
    animateHpBar->setCheckable(true);
    connect(animateHpBar, SIGNAL(triggered(bool)), SLOT(animateHpBar(bool)));
    animateHpBar->setChecked(s.value("animate_hp_bar").toBool());

    QAction *oldStyleButtons = battleMenu->addAction(tr("Old School buttons"));
    oldStyleButtons->setCheckable(true);
    connect(oldStyleButtons, SIGNAL(triggered(bool)), SLOT(changeButtonStyle(bool)));
    oldStyleButtons->setChecked(s.value("old_attack_buttons").toBool());

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
    bool ok;

    QString pass = QInputDialog::getText(this, tr("Enter your password"),
                                         tr("Enter the password for your current name.\n"
                                            "\nIt is advised to use a slightly different password for each server."
                                            " (The server only sees the encrypted form of the pass, but still...)"),
                                         QLineEdit::Password,"", &ok);

    if (ok) {
        QString hash = QString(md5_hash(md5_hash(pass)+salt));
        relay().notify(NetworkCli::AskForPass, hash);
    }
}

void Client::sendRegister() {
    relay().notify(NetworkCli::Register);
    myregister->setDisabled(true);
}
/*
void Client::changeMusicFolder()
{
    QSettings s;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Battle Music Directory"), s.value("battle_music_directory").toString());

    if (dir != "") {
        s.setValue("battle_music_directory", dir + "/");
    }
}
*/
void Client::changeBattleLogFolder()
{
    QSettings s;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Battle Logs Directory"), s.value("battle_logs_directory").toString());

    if (dir != "") {
        s.setValue("battle_logs_directory", dir + "/");
    }
}

void Client::changeButtonStyle(bool old)
{
    QSettings s;
    s.setValue("old_attack_buttons",old);
}

void Client::saveBattleLogs(bool save)
{
    QSettings s;
    s.setValue("save_battle_logs",save);
}

void Client::animateHpBar(bool save)
{
    QSettings s;
    s.setValue("animate_hp_bar", save);
}
/*
void Client::playMusic(bool save)
{
    QSettings s;
    s.setValue("play_battle_music", save);

    emit musicPlayingChanged(save);
}
*/
void Client::spectatingBattleMessage(int battleId, const QByteArray &command)
{
    if (mySpectatingBattles.contains(battleId)) {
        mySpectatingBattles[battleId]->receiveInfo(command);
    }
}

bool Client::battling() const
{
    return mybattle != NULL;
}

void Client::messageReceived(const QString &mess)
{
    printLine(mess);
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

void Client::announcementReceived(const QString &ann)
{
    if (ann.length() == 0)
        return;

    announcement->setText(ann);
    announcement->setAlignment(Qt::AlignCenter);
    announcement->show();
}

void Client::tierListReceived(const QString &tl)
{
    mytiermenu->clear();
    mytiers.clear();

    tierList = tl.split('\n', QString::SkipEmptyParts);

    if (tierList.empty())
        tierList.push_back("All");

    foreach(QString t, tierList) {
        mytiers.push_back(mytiermenu->addAction(t,this,SLOT(changeTier())));
        mytiers.back()->setCheckable(true);
    }

    changeTierChecked(player(ownId()).tier);

    QSettings s;

    if (s.value("sort_players_by_tier").toBool()) {
        sortAllPlayersByTier();
    }
}

void Client::sortPlayersCountingTiers(bool byTier)
{
    sortBT = byTier;
    QSettings s;
    s.setValue("sort_players_by_tier", sortBT);

    if (sortBT) {
        sortAllPlayersByTier();
    } else {
        sortAllPlayersNormally();
    }
}

void Client::sortAllPlayersByTier()
{
    foreach(QIdTreeWidgetItem *it, mytiersitems) {
        myplayers->headerItem()->removeChild(it);
    }

    mytiersitems.clear();


    foreach(QString tier, tierList) {
        QIdTreeWidgetItem *it = new QIdTreeWidgetItem(0, tier, 0);
        //it->setBackgroundColor("#0CA0DD");
        //it->setColor("white");
        QFont f = it->font(0);
        f.setPixelSize(15);
        f.setBold(true);
        it->setFont(0,f);
        it->setText(0,tier);
        myplayers->addTopLevelItem(it);
        mytiersitems.insert(tier, it);

    }
    myplayers->expandAll();

    QHash<int, QIdTreeWidgetItem *>::iterator iter;

    for (iter = myplayersitems.begin(); iter != myplayersitems.end(); ++iter) {
        QString tier = player(iter.key()).tier;

        if (mytiersitems.contains(tier)) {
            placeItem(iter.value(), mytiersitems.value(tier));
        } else {
            myplayers->addTopLevelItem(iter.value());
        }
    }
}

void Client::sortAllPlayersNormally()
{
    myplayers->headerItem()->setText(0,"Players");
        myplayers->clear();

    mytiersitems.clear();

    QHash<int, QIdTreeWidgetItem *>::iterator iter;

    for (iter = myplayersitems.begin(); iter != myplayersitems.end(); ++iter) {
        myplayers->addTopLevelItem(iter.value());
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
    return myplayersinfo[id];
}

BasicInfo Client::info(int id) const
{
    if (myplayersinfo.contains(id))
        return myplayersinfo[id].team;
    else
        return BasicInfo();
}

void Client::seeInfo(QListWidgetItem *it)
{
    seeInfo(((QIdTreeWidgetItem*)(it))->id());
}

void Client::seeInfo(int id)
{
    if (playerExist(id))
    {
	if (mychallenge != NULL) {
	    mychallenge->raise();
	    mychallenge->activateWindow();
	} else {
	    mychallenge = new ChallengeWindow(player(id));
	    connect(mychallenge, SIGNAL(challenge(int)), SLOT(sendChallenge(int)));
	    connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
	    connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
	}
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
            relay().sendChallengeStuff(c);
        } else {
            mychallenge = new ChallengedWindow(player(c),c);
	    connect(mychallenge, SIGNAL(challenge(int)), SLOT(acceptChallenge(int)));
	    connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
	    connect(mychallenge, SIGNAL(cancel(int)), SLOT(refuseChallenge(int)));
	    connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
            mychallenge->activateWindow();
	}
    }
}

void Client::battleStarted(int battleId, int id, const TeamBattle &team, const BattleConfiguration &conf, bool doubles)
{
    cancelFindBattle(false);
    mybattle = new BattleWindow(battleId, player(ownId()), player(id), team, conf, doubles);
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

    battleStarted(battleId, ownId(), id);
}

void Client::battleStarted(int, int id1, int id2)
{
    if (showPEvents || id1 == ownId() || id2 == ownId())
        printLine(tr("Battle between %1 and %2 started.").arg(name(id1), name(id2)));

    myplayersinfo[id1].flags |= PlayerInfo::Battling;
    myplayersinfo[id2].flags |= PlayerInfo::Battling;

    if (id1 != 0) {
        item(id1)->setToolTip(0,tr("Battling against %1").arg(name(id2)));
        updateState(id1);
    }
    if (id2 != 0) {
        item(id2)->setToolTip(0,tr("Battling against %1").arg(name(id1)));
        updateState(id2);
    }
}

void Client::watchBattle(const QString &name0, const QString &name1, int battleId, bool doubles)
{
    BaseBattleWindow *battle = new BaseBattleWindow(player(id(name0)), player(id(name1)), doubles);
    battle->setParent(this);
    battle->setWindowFlags(Qt::Window);
    battle->show();

    connect(this, SIGNAL(destroyed()), battle, SLOT(close()));
    //connect(this, SIGNAL(musicPlayingChanged(bool)), battle, SLOT(playMusic(bool)));
    connect(battle, SIGNAL(closedBW(int)), SLOT(stopWatching(int)));
    connect(battle, SIGNAL(battleMessage(int, QString)), &relay(), SLOT(battleMessage(int, QString)));

    battle->battleId() = battleId;
    battle->client() = this;
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

QIdTreeWidgetItem *Client::item(int id) {
    return myplayersitems.value(id);
}

void Client::forfeitBattle(int id)
{
    relay().sendBattleResult(id, Forfeit);
    removeBattleWindow();
}

void Client::battleFinished(int, int res, int winner, int loser)
{
    if (showPEvents || winner == ownId() || loser == ownId()) {
        if (res == Forfeit) {
            printLine(tr("%1 forfeited against %2.").arg(name(loser), name(winner)));
        } else if (res == Tie) {
            printLine(tr("%1 and %2 tied.").arg(name(loser), name(winner)));
        } else if (res == Win) {
            printLine(tr("%1 won against %2.").arg(name(winner), name(loser)));
        }
    }

    if ((res == Close || res == Forfeit) && (winner == ownId() || loser == ownId()))
        removeBattleWindow();

    myplayersinfo[winner].flags &= 0xFF ^ PlayerInfo::Battling;
    myplayersinfo[loser].flags &= 0xFF ^ PlayerInfo::Battling;

    updateState(winner);
    updateState(loser);
}

void Client::battleCommand(int , const QByteArray &command)
{
    if (!mybattle)
        return;

    mybattle->receiveInfo(command);
}

void Client::removeBattleWindow()
{
    if (mybattle)
	mybattle->close();
}

QString Client::name(int id) const
{
    return info(id).name;
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
            if (c.desc() == ChallengeInfo::Refused) {
                printLine(tr("%1 refused your challenge.").arg(name(c)));
                if (challengeWindowOpen() && challengeWindowPlayer()== c) {
                    closeChallengeWindow();
                }
            } else if (c.desc() == ChallengeInfo::Busy) {
                printLine(tr("%1 is busy.").arg(name(c)));
                if (challengeWindowOpen() && challengeWindowPlayer()== c) {
                    closeChallengeWindow();
                }
            } else if (c.desc() == ChallengeInfo::Cancelled) {
                printLine(tr("%1 cancelled their challenge.").arg(name(c)));
                if (challengeWindowOpen() && challengeWindowPlayer()== c) {
		    closeChallengeWindow();
		}
            } else if (c.desc() == ChallengeInfo::InvalidTeam) {
                printLine(tr("%1 has an invalid team.").arg(name(c)));
                if (challengeWindowOpen() && challengeWindowPlayer()== c) {
                    closeChallengeWindow();
                }
            }
	}
    }
}

void Client::awayChanged(int id, bool away)
{
    if (showPEvents) {
        if (away) {
            printLine(tr("%1 is idling.").arg(name(id)));
        } else {
            printLine(tr("%1 is active and ready for battles.").arg(name(id)));
        }
    }

    playerInfo(id).changeState(PlayerInfo::Away, away);
    updateState(id);
}

bool Client::busy() const
{
    return challengeWindowOpen() || battling() || myteambuilder || away() || myBattleFinder;
}

bool Client::away() const
{
    return playerInfo(ownId()).away();
}

bool Client::challengeWindowOpen() const
{
    return mychallenge != 0;
}

void Client::acceptChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Accepted, id));
}

void Client::refuseChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Refused, id));
}

void Client::sendChallenge(int id)
{
    QSettings s;

    quint32 clauses = 0;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses |= s.value("clause_"+ChallengeInfo::clause(i)).toBool() ? (1 << i) : 0;
    }

    bool doubles = s.value("challenge_with_doubles").toBool();

    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Sent, id, clauses,doubles));
}

void Client::clearChallenge()
{
    mychallenge = NULL;
}

void Client::errorFromNetwork(int errnum, const QString &errorDesc)
{
    printHtml("<i>"+tr("Error while connected to server -- Received error n°%1: %2").arg(errnum).arg(errorDesc) + "</i>");
}

void Client::connected()
{
    printLine(tr("Connected to Server!"));

    QSettings s;

    FullInfo f = {*team(), s.value("enable_ladder").toBool(), s.value("show_team").toBool(), s.value("trainer_color").value<QColor>()};
    relay().login(f);
}

void Client::disconnected()
{
    printLine(tr("Disconnected from Server!"));
}

TrainerTeam* Client::team()
{
    return myteam;
}

Analyzer &Client::relay()
{
    return myrelay;
}

QScrollDownTextEdit *Client::mainChat()
{
    return mychat;
}

void Client::playerLogin(const PlayerInfo& p)
{
    playerReceived(p);
    if (showPEvents)
        printLine(tr("%1 logged in.").arg(p.team.name));
}

void Client::playerLogout(int id)
{
    QString name = info(id).name;
    if (myIgnored.contains(id))
        removeIgnore(id);
    if (showPEvents)
        printLine(tr("%1 logged out.").arg(name));

    /* removes the item in the playerlist */
    removePlayer(id);
}

void Client::removePlayer(int id)
{
    QString name = info(id).name;

    /* removes the item in the playerlist */
    if(sortBT) {
        for(int i = 0; i < myplayers->topLevelItemCount(); i++)
        {
            myplayers->topLevelItem(i)->removeChild(myplayersitems[id]);
        }
    } else {
        myplayers->removeItemWidget(myplayersitems[id],0);
    }

    myplayersitems.remove(id);
    mynames.remove(name);
    myplayersinfo.remove(id);
    if (mypms.contains(id)) {
        mypms[id]->disable();
        mypms.remove(id);
    }
}

QString Client::ownName() const
{
    return mynick;
}

QString Client::authedNick(int id) const
{
    PlayerInfo p = player(id);

    QString nick = p.team.name;

    if (p.auth > 0 && p.auth < 4) {
        nick += ' ';

        for (int i = 0; i < p.auth; i++) {
            nick += '*';
        }
    }

    return nick;
}

void Client::playerReceived(const PlayerInfo &p)
{
    QIdTreeWidgetItem *item = NULL;

    if (myplayersinfo.contains(p.id)) {
        QString name = info(p.id).name;

        /* removes the item in the playerlist */
        if(sortBT) {
            for(int i = 0; i < myplayers->topLevelItemCount(); i++)
            {
                myplayers->topLevelItem(i)->removeChild(myplayersitems[p.id]);
            }
        } else {
            myplayers->removeItemWidget(myplayersitems[p.id],0);
        }
        item = myplayersitems[p.id];

        myplayersitems.remove(p.id);
        mynames.remove(name);
        myplayersinfo.remove(p.id);
    }

    myplayersinfo.insert(p.id, p);

    QString nick = authedNick(p.id);

    if (!item) {
        item = new QIdTreeWidgetItem(p.id, nick, 0);

        QFont f = item->font(item->level());
        f.setBold(true);
        item->setFont(item->level(),f);
    }
    item->setText(item->level(),nick);

    item->setColor(color(p.id));

    myplayersitems.insert(p.id, item);
    mynames.insert(name(p.id), p.id);
    if (mypms.contains(p.id)) {
        mypms[p.id]->changeName(p.team.name);
    }

    if (sortBT && mytiersitems.contains(p.tier)) {
        placeItem(item, mytiersitems.value(p.tier));
    } else {
        myplayers->addTopLevelItem(item);
    }

    updateState(p.id);

    if (p.id == ownId()) {
        changeTierChecked(p.tier);
    }
}

void Client::placeItem(QIdTreeWidgetItem *item, QTreeWidgetItem *parent)
{
    /* To sort the people in case insensitive order */

             parent->addChild(item);
             parent->sortChildren(0,Qt::AscendingOrder);
             return;

     
}

void Client::teamChanged(const PlayerInfo &p) {
    if (showPEvents) {
        if (name(p.id) != p.team.name) {
            printLine(tr("%1 changed teams and is now known as %2.").arg(name(p.id), p.team.name));
        } else {
            printLine(tr("%1 changed teams.").arg(name(p.id)));
        }
    }
    playerReceived(p);
}

void Client::printLine(const QString &line)
{
    QString timeStr = "";
    if(showTS)
        timeStr = "(" + QTime::currentTime().toString() + ") ";
    if (line.length() == 0) {
        mainChat()->insertPlainText("\n");
        return;
    }
    if (line.leftRef(3) == "***") {
        mainChat()->insertHtml("<span style='color:magenta'>" + timeStr + escapeHtml(line) + "</span><br />");
        return;
    }
    /* Let's add colors */
    int pos = line.indexOf(':');
    if ( pos != -1 ) {
	QString beg = line.left(pos);
	QString end = line.right(line.length()-pos-1);
        if (beg == "~~Server~~") {
            mainChat()->insertHtml("<span style='color:orange'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else if (beg == "Welcome Message") {
            mainChat()->insertHtml("<span style='color:blue'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else if (id(beg) == -1) {
            mainChat()->insertHtml("<span style='color:#3daa68'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else {
            if (myIgnored.contains(id(beg)))
                return;
            mainChat()->insertHtml("<span style='color:" + color(id(beg)).name() + "'>" + timeStr + "<b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
	}
    } else {
        mainChat()->insertPlainText( timeStr + line + "\n");
    }
}

QColor Client::color(int id) const
{
    if (player(id).color.name() == "#000000") {
        return chatColors[id % chatColors.size()];
    } else {
        return player(id).color;
    }
}

void Client::printHtml(const QString &line)
{
    mainChat()->insertHtml(line + "<br />");
}

int Client::id(const QString &name) const
{
    if (mynames.contains(name)) {
	return mynames[name];
    } else {
	return -1;
    }
}

int Client::challengeWindowPlayer() const
{
    return mychallenge->id();
}

void Client::closeChallengeWindow()
{
    mychallenge->forcedClose();
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

    connect(this, SIGNAL(destroyed()), myteambuilder, SLOT(close()));
    connect(t, SIGNAL(done()), this, SLOT(changeTeam()));
    connect(t, SIGNAL(done()), myteambuilder, SLOT(close()));
    connect(t, SIGNAL(showDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)), this, SLOT(showDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)));
}

void Client::showDock(Qt::DockWidgetArea areas, QDockWidget *dock, Qt::Orientation orientation)
{
    if (myteambuilder) {
        myteambuilder->addDockWidget(areas, dock, orientation);
    }
}

void Client::changeTeam()
{
    if (battling()) {
        printLine(tr("You can't change teams while battling, so your nick was kept."));
        myteam->trainerNick() = mynick;
    } else {
        mynick = myteam->trainerNick();
    }
    cancelFindBattle(false);
    relay().sendTeam(*myteam);
}

PlayerInfo Client::playerInfo(int id) const
{
    return myplayersinfo.value(id);
}

PlayerInfo &Client::playerInfo(int id)
{
    return myplayersinfo[id];
}

void Client::updateState(int id)
{
    if (item(id)) {
        if (playerInfo(id).battling()) {
            item(id)->setIcon(0, statusIcons[Battling]);
        } else if (playerInfo(id).away()) {
            item(id)->setIcon(0, statusIcons[Away]);
            item(id)->setToolTip(0, "");
        } else {
            item(id)->setIcon(0, statusIcons[Available]);
            item(id)->setToolTip(0, "");
        }
    }
}

void Client::requestBan(const QString &name)
{
    if (id(name) == -1) {
        relay().notify(NetworkCli::CPBan, name);
    } else {
        ban(id(name));
    }
}

void Client::ignore(int id)
{
    printLine(tr("You ignored %1.").arg(name(id)));
    myIgnored.append(id);
}

void Client::removeIgnore(int id)
{
    printLine(tr("You stopped ignoring %1.").arg(name(id)));
    myIgnored.removeOne(id);
}

void Client::bugReport()
{
    QDesktopServices::openUrl(QUrl("http://pokemon-online.eu/forums/forumdisplay.php?9-Bugs-and-Fixes"));
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
    ml->addWidget(doubles = new QCheckBox(tr("Double battle")));
    QHBoxLayout *sub2 = new QHBoxLayout();
    ml->addLayout(sub2);
    sub2->addWidget(rangeOn = new QCheckBox(tr("Only battle players with a max rating difference of ")));
    sub2->addWidget(range = new QLineEdit());
/*
    QGroupBox *gb = new QGroupBox(tr("Clauses"));
    ml->addWidget(gb);

    QGridLayout *gbl = new QGridLayout(gb);
    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i] = new QCheckBox(ChallengeInfo::clause(i));
        clauses[i]->setToolTip(ChallengeInfo::description(i));
        clauses[i]->setTristate();
        clauses[i]->setCheckState(Qt::CheckState(s.value(QString("clause_%1_state").arg(ChallengeInfo::clause(i))).toInt()));
        gbl->addWidget(clauses[i], i/2, i%2);
    }
*/
    QHBoxLayout *hl = new QHBoxLayout();
    ml->addLayout(hl);
    hl->addWidget(ok = new QPushButton(tr("Find Battle")));
    hl->addWidget(cancel = new QPushButton(tr("Cancel")));

    range->setMaximumWidth(35);

    rated->setChecked(s.value("find_battle_force_rated").toBool());
    sameTier->setChecked(s.value("find_battle_same_tier").toBool());
    doubles->setChecked(s.value("find_battle_mode").toBool());
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
    // I found out its annoying to have it on so I turned it off
    /*sameTier->setDisabled(rated->isChecked());
    rangeOn->setDisabled(!sameTier->isChecked() && !rated->isChecked());
    range->setDisabled(!sameTier->isChecked() && !rated->isChecked());*/
}

void BattleFinder::throwChallenge()
{
    QSettings s;

    s.setValue("find_battle_force_rated", rated->isChecked());
    s.setValue("find_battle_same_tier", sameTier->isChecked());
    s.setValue("find_battle_range_on", rangeOn->isChecked());
    s.setValue("find_battle_range", range->text().toInt());
    s.setValue("find_battle_mode", doubles->isChecked());

    FindBattleData d;
    d.rated = rated->isChecked();
    d.sameTier = sameTier->isChecked();
    d.range = range->text().toInt();
    d.ranged = rangeOn->isChecked();
    d.mode = doubles->isChecked();
/*
    d.forcedClauses = 0;
    d.bannedClauses = 0;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        switch(clauses[i]->checkState()) {
        case Qt::Checked:
            d.forcedClauses |= 1 << i;
            break;
        case Qt::Unchecked:
            d.bannedClauses |= 1 << i;
            break;
        default:
            break;
        }
        s.setValue(QString("clause_%1_state").arg(ChallengeInfo::clause(i)), clauses[i]->checkState());
    }
*/
    emit findBattle(d);
}
