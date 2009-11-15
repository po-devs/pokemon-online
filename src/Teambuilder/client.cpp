#include "client.h"
#include "mainwindow.h"
#include "challenge.h"
#include "battlewindow.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/functions.h"
#include "../PokemonInfo/pokemonstructs.h"

Client::Client(TrainerTeam *t, const QString &url) : myteam(t), myrelay()
{
    mychallenge = NULL;
    mybattle = NULL;

    setFixedSize(800, 600);

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(myplayers = new QListWidget(), 0, 0, 3, 1);
    layout->addWidget(mychat = new QTextEdit(), 0, 1, 1, 2);
    layout->addWidget(myline = new QLineEdit(), 1, 1, 1, 2);
    layout->addWidget(myexit = new QPushButton(tr("&Exit")), 2, 1);
    layout->addWidget(mysender = new QPushButton(tr("&Send")), 2, 2);

    myplayers->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    myplayers->setContextMenuPolicy(Qt::CustomContextMenu);
    myplayers->setSortingEnabled(true);
    mychat->setReadOnly(true);

    connect(myplayers, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(myexit, SIGNAL(clicked()), SIGNAL(done()));
    connect(myline, SIGNAL(returnPressed()), SLOT(sendText()));
    connect(mysender, SIGNAL(clicked()), SLOT(sendText()));

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

	menu->exec(mapToGlobal(requested));
    }
}

void Client::sendText()
{
    relay().sendMessage(myline->text());
    myline->clear();
}

QMenuBar * Client::createMenuBar(MainWindow *w)
{
    (void) w;
    return NULL;
}

void Client::initRelay()
{
    connect(&relay(), SIGNAL(connectionError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(&relay(), SIGNAL(protocolError(int, QString)), SLOT(errorFromNetwork(int, QString)));
    connect(&relay(), SIGNAL(connected()), SLOT(connected()));
    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(messageReceived(QString)), SLOT(messageReceived(QString)));
    connect(&relay(), SIGNAL(playerReceived(Player)), SLOT(playerReceived(Player)));
    connect(&relay(), SIGNAL(playerLogin(Player)), SLOT(playerLogin(Player)));
    connect(&relay(), SIGNAL(playerLogout(int)), SLOT(playerLogout(int)));
    connect(&relay(), SIGNAL(challengeReceived(int)), SLOT(seeChallenge(int)));
    connect(&relay(), SIGNAL(challengeRefused(int)), SLOT(challengeRefused(int)));
    connect(&relay(), SIGNAL(challengeBusied(int)), SLOT(challengeBusied(int)));
    connect(&relay(), SIGNAL(challengeCanceled(int)), SLOT(challengeCanceled(int)));
    connect(&relay(), SIGNAL(battleStarted(int, TeamBattle)), SLOT(battleStarted(int, TeamBattle)));
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
    Player ret = {id, info(id)};
    return ret;
}

BasicInfo Client::info(int id) const
{
    return myplayersinfo[id];
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
	    relay().busyForChallenge(id);
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

void Client::battleStarted(int id, const TeamBattle &team)
{
    mybattle = new BattleWindow(name(id), team);
    connect(mybattle, SIGNAL(destroyed()), this, SLOT(clearBattle()));
    connect(mybattle, SIGNAL(forfeit()), SLOT(forfeitBattle()));
    connect(this, SIGNAL(destroyed()), mychallenge, SLOT(close()));
}

void Client::forfeitBattle()
{
    relay().sendBattleResult(Forfeit);
    removeBattleWindow();
}

void Client::removeBattleWindow()
{
    mybattle->close();
}

QString Client::name(int id) const
{
    return info(id).name;
}

void Client::challengeRefused(int id)
{
    if (playerExist(id))
    {
	printLine(tr("%1 refused your challenge.").arg(name(id)));
    }
}

void Client::challengeBusied(int id)
{
    if (playerExist(id))
    {
	printLine(tr("%1 is busy.").arg(name(id)));
    }
}

void Client::challengeCanceled(int id)
{
    if (playerExist(id))
    {
	printLine(tr("%1 canceled their challenge").arg(name(id)));
	if (mychallenge->id() == id) {
	    mychallenge->close();
	}
    }
}

bool Client::busy() const
{
    return challengeWindowOpen() || battling();
}

bool Client::challengeWindowOpen() const
{
    return mychallenge != 0;
}

void Client::acceptChallenge(int id)
{
    relay().acceptChallenge(id);
}

void Client::refuseChallenge(int id)
{
    relay().refuseChallenge(id);
}

void Client::sendChallenge(int id)
{
    relay().sendChallenge(id);
}

void Client::clearChallenge()
{
    mychallenge = NULL;
}

void Client::errorFromNetwork(int errnum, const QString &errorDesc)
{
    QMessageBox::critical(this, tr("Error while connected to server"), tr("Received error n°%1: %2").arg(errnum).arg(errorDesc));
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

QTextEdit *Client::mainChat()
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
    QString name = myplayersinfo[id].name;

    printLine(tr("%1 logged out.").arg(name));

    /* removes the item in the playerlist */
    delete myplayers->takeItem(myplayers->row(myplayersitems[id]));

    myplayersitems.remove(id);
    mynames.remove(name);
    myplayersinfo.remove(id);
}

QString Client::ownName() const
{
    return myteam->trainerNick();
}

void Client::playerReceived(const Player &p)
{
    myplayersinfo.insert(p.id, p.team);

    QIdListWidgetItem *item = new QIdListWidgetItem(p.id, p.team.name);

    myplayersitems.insert(p.id, item);
    mynames.insert(name(p.id), p.id);

    myplayers->addItem(item);
}

void Client::printLine(const QString &line)
{
    /* Let's add colors */
    int pos = line.indexOf(':');
    if ( pos != -1 ) {
	QString beg = line.left(pos);
	QString end = line.right(line.length()-pos-1);
	if (id(beg) == -1) {
	    mainChat()->insertHtml("<span style='color:cyan'><b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
	} else if (beg == ownName()) {
	    mainChat()->insertHtml("<span style='color:#5811b1'><b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
	 } else {
	    mainChat()->insertHtml("<span style='color:green'><b>" + escapeHtml(beg) + ":</b></span>" + escapeHtml(end) + "<br />");
	}
    } else {
	mainChat()->insertPlainText(line + "\n");
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

QDataStream & operator >> (QDataStream &in, Player &p)
{
    in >> p.id;
    in >> p.team;

    return in;
}

QDataStream & operator << (QDataStream &out, const Player &p)
{
    out << p.id;
    out << p.team;

    return out;
}
