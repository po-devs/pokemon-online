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


Client::Client(TrainerTeam *t, const QString &url , const quint16 port) : myteam(t), myrelay()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    mychallenge = NULL;
    mybattle = NULL;
    myteambuilder = NULL;
    resize(800, 600);

    QPushButton *findmatch;
    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QListWidget(), 0, 0, 3, 1, Qt::AlignLeft);
    layout->addWidget(mychat = new QScrollDownTextEdit(), 0, 1);
    mychat->setObjectName("MainChat");
    layout->addWidget(myline = new QLineEdit(), 1, 1);
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    layout->addLayout(buttonsLayout,2,1);
    buttonsLayout->addWidget(findmatch = new QPushButton(tr("&Find Battle")));
    buttonsLayout->addWidget(myregister = new QPushButton(tr("&Register")));
    buttonsLayout->addWidget(myexit = new QPushButton(tr("&Exit")));
    buttonsLayout->addWidget(mysender = new QPushButton(tr("&Send")));
    layout->setColumnStretch(1,100);

    myplayers->setMaximumWidth(200);
    myplayers->setContextMenuPolicy(Qt::CustomContextMenu);

    /*myplayers->setStyleSheet(
            "background: qradialgradient(cx:0.5, cy:0.5, radius: 0.8,"
            "stop:0 white, stop:1 #bfbfbd);"
            "border: 1px solid gray;"
            "border-radius: 10px"
        );*/
    QPalette pal = palette();
    pal.setColor(QPalette::AlternateBase, Qt::blue);
    pal.setColor(QPalette::Base, Qt::blue);
    setPalette(pal);
    myregister->setDisabled(true);
    mynick = t->trainerNick();

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myplayers, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(seeInfo(QListWidgetItem*)));
    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendText()));
    connect(mysender, SIGNAL(clicked()), SLOT(sendText()));
    connect(myregister, SIGNAL(clicked()), SLOT(sendRegister()));
    connect(findmatch, SIGNAL(clicked()), SLOT(openBattleFinder()));

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
    QIdListWidgetItem *item = dynamic_cast<QIdListWidgetItem*>(myplayers->itemAt(requested));

    if (item)
    {
	QMenu *menu = new QMenu(this);

        createIntMapper(menu->addAction(tr("&Challenge Window")), SIGNAL(triggered()), this, SLOT(seeInfo(int)), item->id());

        createIntMapper(menu->addAction(tr("&Ranking")), SIGNAL(triggered()), this, SLOT(seeRanking(int)), item->id());
        if (item->id() == ownId()) {
            if (away()) {
                createIntMapper(menu->addAction(tr("Go &Back")), SIGNAL(triggered()), this, SLOT(goAway(int)), false);
            } else {
                createIntMapper(menu->addAction(tr("Go &Away")), SIGNAL(triggered()), this, SLOT(goAway(int)), true);
            }
        } else {
            createIntMapper(menu->addAction(tr("&PM")), SIGNAL(triggered()), this, SLOT(startPM(int)), item->id());
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
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load Team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &TeamBuilder"),this,SLOT(openTeamBuilder()),Qt::CTRL+Qt::Key_T);
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    for(QStringList::iterator i = style.begin();i!=style.end();i++)
    {
        menuStyle->addAction(*i,w,SLOT(changeStyle()));
    }
    menuStyle->addSeparator();
    menuStyle->addAction(tr("Reload StyleSheet"), w, SLOT(loadStyleSheet()));
    QMenu * menuActions = menuBar->addMenu(tr("&Options"));
    goaway = menuActions->addAction(tr("Go &Away"));
    goaway->setCheckable(true);
    goaway->setChecked(this->away());
    connect(goaway, SIGNAL(triggered(bool)), this, SLOT(goAwayB(bool)));

    QSettings s;


    QAction * show = menuActions->addAction(tr("&Show Team"));
    show->setCheckable(true);
    connect(show, SIGNAL(triggered(bool)), SLOT(showTeam(bool)));
    show->setChecked(s.value("show_team").toBool());

    QAction * ladd = menuActions->addAction(tr("Enable &Ladder"));
    ladd->setCheckable(true);
    connect(ladd, SIGNAL(triggered(bool)), SLOT(enableLadder(bool)));
    ladd->setChecked(s.value("enable_ladder").toBool());

    QAction *show_events = menuActions->addAction(tr("Show Player &Events"));
    show_events->setCheckable(true);
    connect(show_events, SIGNAL(triggered(bool)), SLOT(showPlayerEvents(bool)));
    show_events->setChecked(s.value("show_player_events").toBool());
    showPEvents = show_events->isChecked();

    QAction * show_ts = menuActions->addAction(tr("&Show Timestamps"));
    show_ts->setCheckable(true);
    connect(show_ts, SIGNAL(triggered(bool)), SLOT(showTimeStamps(bool)));
    show_ts->setChecked(s.value("show_timestamps").toBool());
    showTS = show_ts->isChecked();

    mytiermenu = menuBar->addMenu(tr("&Tier"));

    QMenu *battleMenu = menuBar->addMenu(tr("&Battles", "Menu"));
    QAction * saveLogs = battleMenu->addAction(tr("Save &Battle Logs"));
    saveLogs->setCheckable(true);
    connect(saveLogs, SIGNAL(triggered(bool)), SLOT(saveBattleLogs(bool)));
    saveLogs->setChecked(s.value("save_battle_logs").toBool());

    QAction *animateHpBar = battleMenu->addAction(tr("Animate HP Bar"));
    animateHpBar->setCheckable(true);
    connect(animateHpBar, SIGNAL(triggered(bool)), SLOT(animateHpBar(bool)));
    animateHpBar->setChecked(s.value("animate_hp_bar").toBool());

    QAction *PlayMusic = battleMenu->addAction(tr("Play Music and Sounds"));
    PlayMusic->setCheckable(true);
    connect(PlayMusic, SIGNAL(triggered(bool)), SLOT(PlayMusic(bool)));
    PlayMusic->setChecked(s.value("play_music").toBool());

    mymenubar = menuBar;
    return menuBar;
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
    connect(relay, SIGNAL(battleStarted(int, TeamBattle, BattleConfiguration)), SLOT(battleStarted(int, TeamBattle, BattleConfiguration)));
    connect(relay, SIGNAL(battleStarted(int, int)), SLOT(battleStarted(int, int)));
    connect(relay, SIGNAL(battleFinished(int,int,int)), SLOT(battleFinished(int,int,int)));
    connect(relay, SIGNAL(passRequired(QString)), SLOT(askForPass(QString)));
    connect(relay, SIGNAL(notRegistered(bool)), myregister, SLOT(setEnabled(bool)));
    connect(relay, SIGNAL(playerKicked(int,int)),SLOT(playerKicked(int,int)));
    connect(relay, SIGNAL(playerBanned(int,int)),SLOT(playerBanned(int,int)));
    connect(relay, SIGNAL(PMReceived(int,QString)), SLOT(PMReceived(int,QString)));
    connect(relay, SIGNAL(awayChanged(int, bool)), SLOT(awayChanged(int, bool)));
    connect(relay, SIGNAL(spectatedBattle(QString,QString,int)), SLOT(watchBattle(QString,QString,int)));
    connect(relay, SIGNAL(spectatingBattleMessage(int,QByteArray)), SLOT(spectatingBattleMessage(int , QByteArray)));
    connect(relay, SIGNAL(spectatingBattleFinished(int)), SLOT(stopWatching(int)));
    connect(relay, SIGNAL(versionDiff(QString, QString)), SLOT(versionDiff(QString, QString)));
    connect(relay, SIGNAL(tierListReceived(QString)), SLOT(tierListReceived(QString)));
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

void Client::PlayMusic(bool save)
{
    QSettings s;
    s.setValue("play_music", save);
}

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
    if (a != b) {
        printHtml(toColor(tr("Your client version (%2) doesn't match with the server's (%1).").arg(a,b), QColor("#e37800")));
    }
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
    seeInfo(((QIdListWidgetItem*)(it))->id());
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
            mychallenge = new ChallengedWindow(player(c),c.clauses);
	    connect(mychallenge, SIGNAL(challenge(int)), SLOT(acceptChallenge(int)));
	    connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
	    connect(mychallenge, SIGNAL(cancel(int)), SLOT(refuseChallenge(int)));
	    connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
            mychallenge->activateWindow();
	}
    }
}

void Client::battleStarted(int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    mybattle = new BattleWindow(player(ownId()), player(id), team, conf);
    connect(this, SIGNAL(destroyed()), mybattle, SLOT(deleteLater()));
    mybattle->setWindowFlags(Qt::Window);
    mybattle->client() = this;
    mybattle->show();
    mybattle->activateWindow();

    connect(mybattle, SIGNAL(forfeit()), SLOT(forfeitBattle()));
    connect(mybattle, SIGNAL(battleCommand(BattleChoice)), &relay(), SLOT(battleCommand(BattleChoice)));
    connect(mybattle, SIGNAL(battleMessage(QString)), &relay(), SLOT(battleMessage(QString)));
    connect(&relay(), SIGNAL(battleMessage(QByteArray)), mybattle, SLOT(receiveInfo(QByteArray)));
    connect(this, SIGNAL(destroyed()), mybattle, SLOT(close()));

    battleStarted(ownId(), id);
}

void Client::watchBattle(const QString &name0, const QString &name1, int battleId)
{
    BaseBattleWindow *battle = new BaseBattleWindow(player(id(name0)), player(id(name1)));
    battle->setParent(this);
    battle->setWindowFlags(Qt::Window);
    battle->show();

    connect(this, SIGNAL(destroyed()), battle, SLOT(close()));
    connect(battle, SIGNAL(closedBW(int)), SLOT(stopWatching(int)));
    connect(battle, SIGNAL(battleMessage(QString, int)), &relay(), SLOT(battleMessage(QString, int)));

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

void Client::battleStarted(int id1, int id2)
{
    if (showPEvents || id1 == ownId() || id2 == ownId())
        printLine(tr("Battle between %1 and %2 started.").arg(name(id1), name(id2)));

    myplayersinfo[id1].flags |= PlayerInfo::Battling;
    myplayersinfo[id2].flags |= PlayerInfo::Battling;

    updateState(id1);
    updateState(id2);
}

QIdListWidgetItem *Client::item(int id) {
    return myplayersitems.value(id);
}

void Client::forfeitBattle()
{
    relay().sendBattleResult(Forfeit);
    removeBattleWindow();
}

void Client::battleFinished(int res, int winner, int loser)
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

void Client::removeBattleWindow()
{
    if (mybattle)
	mybattle->close();
}

QString Client::name(int id) const
{
    return info(id).name;
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
            printLine(tr("%1 is away.").arg(name(id)));
        } else {
            printLine(tr("%1 has returned.").arg(name(id)));
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
    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Sent, id, clauses));
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
    delete myplayers->takeItem(myplayers->row(myplayersitems[id]));

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

void Client::playerReceived(const PlayerInfo &p)
{
    if (myplayersinfo.contains(p.id)) {
        QString name = info(p.id).name;

        /* removes the item in the playerlist */
        delete myplayers->takeItem(myplayers->row(myplayersitems[p.id]));

        myplayersitems.remove(p.id);
        mynames.remove(name);
        myplayersinfo.remove(p.id);
    }

    myplayersinfo.insert(p.id, p);

    QString nick = p.team.name;

    if (p.auth > 0 && p.auth < 4) {
        nick += ' ';

        for (int i = 0; i < p.auth; i++) {
            nick += '*';
        }
    }

    QIdListWidgetItem *item = new QIdListWidgetItem(p.id, nick);

    item->setColor(color(p.id));

    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);

    myplayersitems.insert(p.id, item);
    mynames.insert(name(p.id), p.id);
    if (mypms.contains(p.id)) {
        mypms[p.id]->changeName(p.team.name);
    }

    /* To sort the people in case insensitive order */
    bool inserted = false;
    for(int i = 0; i < myplayers->count(); i++) {
        if (item->text().compare(myplayers->item(i)->text(), Qt::CaseInsensitive) < 0) {
            inserted = true;
            myplayers->insertItem(i,item);
        }
    }

    if (!inserted) {
        myplayers->addItem(item);
    }

    updateState(p.id);

    if (p.id == ownId()) {
        changeTierChecked(p.tier);
    }
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
            mainChat()->insertHtml("<span style='color:#74F099'>" + timeStr + "<b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else {
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

    myteambuilder = new QMainWindow(this);
    myteambuilder->setAttribute(Qt::WA_DeleteOnClose, true);

    TeamBuilder *t = new TeamBuilder(myteam);
    myteambuilder->resize(t->size());
    myteambuilder->setCentralWidget(t);
    myteambuilder->show();
    myteambuilder->setMenuBar(t->createMenuBar((MainEngine*)parent()));

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
    mynick = myteam->trainerNick();
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
            item(id)->setIcon(statusIcons[Battling]);
        } else if (playerInfo(id).away()) {
            item(id)->setIcon(statusIcons[Away]);
        } else {
            item(id)->setIcon(statusIcons[Available]);
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

/**********************************************************/
/****************** BATTLE FINDER *************************/
/**********************************************************/

BattleFinder::BattleFinder(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setSpacing(10);
    setWindowFlags(Qt::Window);

    QPushButton *ok, *cancel;
    ml->addWidget(rated = new QCheckBox(tr("Force rated battles")));
    ml->addWidget(sameTier = new QCheckBox(tr("Force same tier")));
    QHBoxLayout *sub2 = new QHBoxLayout();
    ml->addLayout(sub2);
    sub2->addWidget(rangeOn = new QCheckBox(tr("Only battle players in the rating range")));
    sub2->addWidget(range = new QLineEdit());

    QHBoxLayout *hl = new QHBoxLayout();
    ml->addLayout(hl);
    hl->addWidget(ok = new QPushButton(tr("Find Battle")));
    hl->addWidget(cancel = new QPushButton(tr("Cancel")));

    range->setMaximumWidth(35);

    QSettings s;
    rated->setChecked(s.value("find_battle_force_rated").toBool());
    sameTier->setChecked(s.value("find_battle_same_tier").toBool());
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

    FindBattleData d;
    d.rated = rated->isChecked();
    d.sameTier = sameTier->isChecked();
    d.range = range->text().toInt();
    d.ranged = rangeOn->isChecked();

    emit findBattle(d);
}
