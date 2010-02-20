#include "client.h"
#include "mainwindow.h"
#include "challenge.h"
#include "teambuilder.h"
#include "battlewindow.h"
#include "pmwindow.h"
#include "controlpanel.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/functions.h"
#include "../PokemonInfo/pokemonstructs.h"

Client::Client(TrainerTeam *t, const QString &url) : myteam(t), myrelay()
{
    mychallenge = NULL;
    mybattle = NULL;
    myteambuilder = NULL;

    setFixedSize(800, 600);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QListWidget(), 0, 0, 3, 1);
    layout->addWidget(mychat = new QScrollDownTextEdit(), 0, 1, 1, 3);
    layout->addWidget(myline = new QLineEdit(), 1, 1, 1, 3);
    layout->addWidget(myregister = new QPushButton(tr("&Register")),2,1);
    layout->addWidget(myexit = new QPushButton(tr("&Exit")), 2, 2);
    layout->addWidget(mysender = new QPushButton(tr("&Send")), 2, 3);

    myplayers->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    myplayers->setContextMenuPolicy(Qt::CustomContextMenu);
    myplayers->setSortingEnabled(true);
    myregister->setDisabled(true);
    mynick = t->trainerNick();

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendText()));
    connect(mysender, SIGNAL(clicked()), SLOT(sendText()));
    connect(myregister, SIGNAL(clicked()), SLOT(sendRegister()));

    initRelay();

    relay().connectTo(url, 5080);
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

        createIntMapper(menu->addAction(tr("See &Info")), SIGNAL(triggered()), this, SLOT(seeInfo(int)), item->id());
        createIntMapper(menu->addAction(tr("&PM")), SIGNAL(triggered()), this, SLOT(startPM(int)), item->id());

        if (item->id() == ownId()) {
            if (away()) {
                createIntMapper(menu->addAction(tr("Go &Back")), SIGNAL(triggered()), this, SLOT(goAway(int)), false);
            } else {
                createIntMapper(menu->addAction(tr("Go &Away")), SIGNAL(triggered()), this, SLOT(goAway(int)), true);
            }
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

    if (mypms.contains(id)) {
        mypms[id]->raise();
        this->activateWindow();
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
    if (!playerExist(id)) {
        return;
    }
    if (!mypms.contains(id)) {
        startPM(id);
    }
    mypms[id]->raise();
    mypms[id]->activateWindow();
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
    if (myline->text().trimmed().length() > 0)
        relay().sendMessage(myline->text().trimmed());
    myline->clear();
}

QMenuBar * Client::createMenuBar(MainWindow *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&Load Team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("Open &TeamBuilder"),this,SLOT(openTeamBuilder()),Qt::CTRL+Qt::Key_T);
    //menuFichier->addAction(tr("&Quit"),w,SLOT(close()),Qt::CTRL+Qt::Key_Q);
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    for(QStringList::iterator i = style.begin();i!=style.end();i++)
    {
        menuStyle->addAction(*i,w,SLOT(changeStyle()));
    }

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
                                         tr("Enter your password.\n"
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

bool Client::battling() const
{
    return mybattle != NULL;
}

void Client::messageReceived(const QString &mess)
{
    printLine(mess);
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
            mychallenge = new ChallengedWindow(player(c),c.sleepClause());
	    connect(mychallenge, SIGNAL(challenge(int)), SLOT(acceptChallenge(int)));
	    connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
	    connect(mychallenge, SIGNAL(cancel(int)), SLOT(refuseChallenge(int)));
	    connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
	}
        mychallenge->raise();
        mychallenge->activateWindow();
    }
}

void Client::battleStarted(int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    mybattle = new BattleWindow(this->team()->trainerNick(),name(id), this->id(ownName()), id, team, conf);
    connect(mybattle, SIGNAL(forfeit()), SLOT(forfeitBattle()));
    connect(mybattle, SIGNAL(battleCommand(BattleChoice)), &relay(), SLOT(battleCommand(BattleChoice)));
    connect(mybattle, SIGNAL(battleMessage(QString)), &relay(), SLOT(battleMessage(QString)));
    connect(&relay(), SIGNAL(battleMessage(QByteArray)), mybattle, SLOT(receiveInfo(QByteArray)));
    connect(this, SIGNAL(destroyed()), mybattle, SLOT(close()));

    battleStarted(ownId(), id);
}

void Client::battleStarted(int id1, int id2)
{
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
    if (res == Forfeit) {
        printLine(tr("%1 forfeited against %2.").arg(name(loser), name(winner)));
    } else if (res == Tie) {
        printLine(tr("%1 and %2 tied.").arg(name(loser), name(winner)));
    } else if (res == Win) {
        printLine(tr("%1 won against %2.").arg(name(winner), name(loser)));
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
    if (away) {
        printLine(tr("%1 is away.").arg(name(id)));
    } else {
        printLine(tr("%1 has returned.").arg(name(id)));
    }

    playerInfo(id).changeState(PlayerInfo::Away, away);
    updateState(id);
}

bool Client::busy() const
{
    return challengeWindowOpen() || battling() || myteambuilder || away();
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
    relay().sendChallengeStuff(ChallengeInfo(ChallengeInfo::Sent, id, s.value("sleep_clause").toBool()));
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

    relay().login(*team());
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
    printLine(tr("%1 logged in.").arg(p.team.name));
}

void Client::playerLogout(int id)
{
    QString name = info(id).name;

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
        delete mypms[id];
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

    myplayersitems.insert(p.id, item);
    mynames.insert(name(p.id), p.id);
    if (mypms.contains(p.id)) {
        mypms[p.id]->changeName(p.team.name);
    }

    myplayers->addItem(item);

    updateState(p.id);
}

void Client::teamChanged(const PlayerInfo &p) {
    if (name(p.id) != p.team.name) {
        printLine(tr("%1 changed teams and is now known as %2.").arg(name(p.id), p.team.name));
    } else {
        printLine(tr("%1 changed teams.").arg(name(p.id)));
    }
    playerReceived(p);
}

void Client::printLine(const QString &line)
{
    if (line.length() == 0) {
        mainChat()->insertPlainText("\n");
        return;
    }
    if (line.leftRef(3) == "***") {
        mainChat()->insertHtml("<span style='color:magenta'>(" + QTime::currentTime().toString() + ") " + escapeHtml(line) + "</span><br />");
        return;
    }
    /* Let's add colors */
    int pos = line.indexOf(':');
    if ( pos != -1 ) {
	QString beg = line.left(pos);
	QString end = line.right(line.length()-pos-1);
        if (beg == "~~Server~~") {
            mainChat()->insertHtml("<span style='color:orange'>(" + QTime::currentTime().toString() + ") <b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
        } else if (id(beg) == -1) {
            mainChat()->insertHtml("<span style='color:blue'>(" + QTime::currentTime().toString() + ") <b>" + escapeHtml(beg)  + ":</b></span>" + escapeHtml(end) + "<br />");
	} else if (beg == ownName()) {
            mainChat()->insertHtml("<span style='color:#5811b1'>(" + QTime::currentTime().toString() + ") <b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
	 } else {
            mainChat()->insertHtml("<span style='color:green'>(" + QTime::currentTime().toString() + ") <b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
	}
    } else {
        mainChat()->insertPlainText("(" + QTime::currentTime().toString() + ") " + line + "\n");
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

    if (challengeWindowOpen() || battling() || myteambuilder) {
        printHtml("<i>" + tr("You're already in the middle of something!") + "</i>");
    }

    myteambuilder = new QMainWindow(this);
    myteambuilder->setAttribute(Qt::WA_DeleteOnClose, true);

    TeamBuilder *t = new TeamBuilder(myteam);
    myteambuilder->setCentralWidget(t);
    myteambuilder->show();
    myteambuilder->setMenuBar(t->createMenuBar(myteambuilder));
    myteambuilder->layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(t, SIGNAL(done()), this, SLOT(changeTeam()));
    connect(t, SIGNAL(done()), myteambuilder, SLOT(close()));
    connect(t, SIGNAL(showDockAdvanced(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)), this, SLOT(showDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)));
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
            item(id)->setColor(Qt::blue);
        } else if (playerInfo(id).away()) {
            item(id)->setColor(Qt::darkGray);
        } else {
            item(id)->setColor(Qt::black);
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
