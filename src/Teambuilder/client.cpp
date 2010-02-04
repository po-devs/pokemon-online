#include "client.h"
#include "mainwindow.h"
#include "challenge.h"
#include "teambuilder.h"
#include "battlewindow.h"
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
    mychat->setReadOnly(true);
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

void Client::showContextMenu(const QPoint &requested)
{
    QIdListWidgetItem *item = dynamic_cast<QIdListWidgetItem*>(myplayers->itemAt(requested));

    if (item)
    {
	QMenu *menu = new QMenu(this);

	QSignalMapper *mymapper = new QSignalMapper(menu);
	QAction *viewinfo = menu->addAction("See &Info", mymapper, SLOT(map()));
	mymapper->setMapping(viewinfo, item->id());
	QSignalMapper *mymapper2 = new QSignalMapper(menu);
	QAction *challenge = menu->addAction("&Challenge", mymapper2, SLOT(map()));
	mymapper2->setMapping(challenge, item->id());

        connect(mymapper, SIGNAL(mapped(int)), SLOT(seeInfo(int)));
        connect(mymapper2, SIGNAL(mapped(int)), SLOT(sendChallenge(int)));

        int myauth = player(id(ownName())).auth;
        int otherauth = player(item->id()).auth;

        if (otherauth < myauth) {
            menu->addSeparator();
            QSignalMapper *mymapper3 = new QSignalMapper(menu);
            QAction *kick = menu->addAction("&Kick", mymapper3, SLOT(map()));
            mymapper3->setMapping(kick, item->id());
            connect(mymapper3, SIGNAL(mapped(int)), SLOT(kick(int)));

            /* If you're an admin, you can ban */
            if (myauth >= 2) {
                menu->addSeparator();
                QSignalMapper *mymapper4 = new QSignalMapper(menu);
                QAction *ban = menu->addAction("&Ban", mymapper4, SLOT(map()));
                mymapper4->setMapping(ban, item->id());
                connect(mymapper4, SIGNAL(mapped(int)), SLOT(ban(int)));
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
    QMenu *menuFichier = menuBar->addMenu("&File");
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
    connect(&relay(), SIGNAL(connectionError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(&relay(), SIGNAL(protocolError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(&relay(), SIGNAL(connected()), SLOT(connected()));
    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(messageReceived(QString)), SLOT(messageReceived(QString)));
    connect(&relay(), SIGNAL(playerReceived(Player)), SLOT(playerReceived(Player)));
    connect(&relay(), SIGNAL(teamChanged(Player)), SLOT(teamChanged(Player)));
    connect(&relay(), SIGNAL(playerLogin(Player)), SLOT(playerLogin(Player)));
    connect(&relay(), SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(&relay(), SIGNAL(challengeStuff(int,int)), SLOT(challengeStuff(int,int)));
    connect(&relay(), SIGNAL(battleStarted(int, TeamBattle, BattleConfiguration)), SLOT(battleStarted(int, TeamBattle, BattleConfiguration)));
    connect(&relay(), SIGNAL(battleFinished(int,int,int)), SLOT(battleFinished(int,int,int)));
    connect(&relay(), SIGNAL(passRequired(QString)), SLOT(askForPass(QString)));
    connect(&relay(), SIGNAL(notRegistered(bool)), myregister, SLOT(setEnabled(bool)));
    connect(&relay(), SIGNAL(playerKicked(int,int)),SLOT(playerKicked(int,int)));
    connect(&relay(), SIGNAL(playerBanned(int,int)),SLOT(playerBanned(int,int)));
}

void Client::playerKicked(int dest, int src) {
    QString mess;

    if (src == 0) {
        mess = QString("%1 was kicked by the server!").arg(name(dest));
    } else {
        mess = QString("%1 kicked %2!").arg(name(src), name(dest));
    }
    printHtml(toBoldColor(mess, Qt::red));
}

void Client::playerBanned(int dest, int src) {
    QString mess;

    if (src == 0) {
        mess = QString("%1 was banned by the server!").arg(name(dest));
    } else {
        mess = QString("%1 banned %2!").arg(name(src), name(dest));
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

void Client::clearBattle()
{
    mybattle = NULL;
}

void Client::messageReceived(const QString &mess)
{
    printLine(mess);
}

bool Client::playerExist(int id) const
{
    return myplayersinfo.contains(id);
}

Player Client::player(int id) const
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

void Client::seeChallenge(int id)
{
    if (playerExist(id))
    {
	if (busy()) {
	    /* Warns the server that we are too busy to accept the challenge */
	    relay().sendChallengeStuff(ChallengeStuff::Busy, id);
	    mychallenge->raise();
	    mychallenge->activateWindow();
	} else {
	    mychallenge = new ChallengedWindow(player(id));
	    connect(mychallenge, SIGNAL(challenge(int)), SLOT(acceptChallenge(int)));
	    connect(mychallenge, SIGNAL(destroyed()), SLOT(clearChallenge()));
	    connect(mychallenge, SIGNAL(cancel(int)), SLOT(refuseChallenge(int)));
	    connect(this, SIGNAL(destroyed()),mychallenge, SLOT(close()));
	}
    }
}

void Client::battleStarted(int id, const TeamBattle &team, const BattleConfiguration &conf)
{
    mybattle = new BattleWindow(this->team()->trainerNick(),name(id), this->id(ownName()), id, team, conf);
    connect(mybattle, SIGNAL(destroyed()), this, SLOT(clearBattle()));
    connect(mybattle, SIGNAL(forfeit()), SLOT(forfeitBattle()));
    connect(mybattle, SIGNAL(battleCommand(BattleChoice)), &relay(), SLOT(battleCommand(BattleChoice)));
    connect(mybattle, SIGNAL(battleMessage(QString)), &relay(), SLOT(battleMessage(QString)));
    connect(&relay(), SIGNAL(battleMessage(QByteArray)), mybattle, SLOT(receiveInfo(QByteArray)));
    connect(this, SIGNAL(destroyed()), mybattle, SLOT(close()));
}

void Client::forfeitBattle()
{
    relay().sendBattleResult(Forfeit);
    removeBattleWindow();
}

void Client::battleFinished(int res, int winner, int loser)
{
    bool self = winner == id(ownName());

    if (res == Forfeit) {
        if (self)
            printLine(QString("%1 forfeited.").arg(name(loser)));
        else
            printLine(QString("You forfeited against %1").arg(name(winner)));
    } else if (res == Tie) {
        printLine(QString("You tied against %1").arg(name(self ? loser : winner)));
    } else if (res == Win) {
        if (self)
            printLine(QString("You won against %1.").arg(name(loser)));
        else
            printLine(QString("You lost against %1").arg(name(winner)));
    }

    if (res == Close || res == Forfeit)
        removeBattleWindow();
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

void Client::challengeStuff(int desc, int id)
{
    if (desc == ChallengeStuff::Sent) {
	seeChallenge(id);
    } else {
	if (playerExist(id)) {
	    if (desc == ChallengeStuff::Refused) {
		printLine(tr("%1 refused your challenge.").arg(name(id)));
	    } else if (desc == ChallengeStuff::Busy) {
		printLine(tr("%1 is busy.").arg(name(id)));
	    } else if (desc == ChallengeStuff::Canceled) {
		printLine(tr("%1 canceled their challenge").arg(name(id)));
		if (challengeWindowOpen() && challengeWindowPlayer()== id) {
		    closeChallengeWindow();
		}
	    }
	}
    }
}

bool Client::busy() const
{
    return challengeWindowOpen() || battling() || myteambuilder;
}

bool Client::challengeWindowOpen() const
{
    return mychallenge != 0;
}

void Client::acceptChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeStuff::Accepted, id);
}

void Client::refuseChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeStuff::Refused, id);
}

void Client::sendChallenge(int id)
{
    relay().sendChallengeStuff(ChallengeStuff::Sent, id);
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

void Client::playerLogin(const Player& p)
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
}

QString Client::ownName() const
{
    return mynick;
}

void Client::playerReceived(const Player &p)
{
    if (myplayersinfo.contains(p.id)) {
        removePlayer(p.id);
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

    myplayers->addItem(item);
}

void Client::teamChanged(const Player &p) {
    if (name(p.id) != p.team.name) {
        printLine(tr("%1 changed team and is now known as %2.").arg(name(p.id), p.team.name));
    } else {
        printLine(tr("%1 changed team.").arg(name(p.id)));
    }
    playerReceived(p);
}

void Client::printLine(const QString &line)
{
    /* Let's add colors */
    int pos = line.indexOf(':');
    if ( pos != -1 ) {
	QString beg = line.left(pos);
	QString end = line.right(line.length()-pos-1);
	if (id(beg) == -1) {
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
    mychallenge->close();
}

QDataStream & operator >> (QDataStream &in, Player &p)
{
    in >> p.id;
    in >> p.team;
    in >> p.auth;

    return in;
}

QDataStream & operator << (QDataStream &out, const Player &p)
{
    out << p.id;
    out << p.team;
    out << p.auth;

    return out;
}

void Client::openTeamBuilder()
{
    if (myteambuilder) {
        myteambuilder->activateWindow();
        myteambuilder->raise();
        return;
    }

    if (busy()) {
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
