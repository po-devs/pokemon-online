#include "client.h"
#include "mainwindow.h"
#include "challenge.h"
#include "../Utilities/otherwidgets.h"

#include "../PokemonInfo/pokemonstructs.h"

Client::Client(TrainerTeam *t, const QString &url) : myteam(t), myrelay()
{
    mychallenge = NULL;

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
    connect(&relay(), SIGNAL(challengeCanceled(int)), SLOT(challengeCanceled(int)));
    connect(&relay(), SIGNAL(battleStarted(int)), SLOT(battleStarted(int)));
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
	    /* Warns the server that we are to busy to accept the challenge */
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

void Client::challengeCanceled(int id)
{
    if (playerExist(id))
    {
	printLine(tr("%1 is busy.").arg(name(id)));
    }
}

void Client::battleStarted(int id)
{
    printLine(tr("Yo! Fake Battle Started with that %1").arg(id));
}

bool Client::busy() const
{
    return challengeWindowOpen();
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
    myplayersinfo.remove(id);
}

void Client::playerReceived(const Player &p)
{
    myplayersinfo.insert(p.id, p.team);

    QIdListWidgetItem *item = new QIdListWidgetItem(p.id, p.team.name);

    myplayersitems.insert(p.id, item);

    myplayers->addItem(item);
}

void Client::printLine(const QString &line)
{
    mainChat()->insertPlainText(line + "\n");
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
