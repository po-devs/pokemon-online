#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"

BattleWindow::BattleWindow(const QString &opp, const TeamBattle &team)
{
    currentPoke() = 0;
    myteam = team;
    possible = false;

    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Battling against %1").arg(opp));
    QGridLayout *mylayout = new QGridLayout(this);

    mylayout->addWidget(myview = new GraphicsZone(), 0, 0, 3, 1);
    mylayout->addWidget(mychat = new QTextEdit(), 0, 1, 1, 2);
    mylayout->addWidget(myline = new QLineEdit(), 1, 1, 1, 2);
    mylayout->addWidget(myforfeit = new QPushButton(tr("&Forfeit")), 2, 1);
    mylayout->addWidget(mysend = new QPushButton(tr("&Send")), 2, 2);
    mylayout->addWidget(mystack = new QStackedWidget(), 3, 0, 1, 3);
    mylayout->addWidget(myattack = new QPushButton(tr("&Attack")), 4, 1);
    mylayout->addWidget(myswitch = new QPushButton(tr("&Switch Pokémons")), 4, 2);

    for (int i = 0; i < 6; i++) {
	myazones[i] = new AttackZone(team.poke(i));
	mystack->addWidget(myazones[i]);

	connect(myazones[i], SIGNAL(clicked(int)), SLOT(attackClicked(int)));
    }

    mypzone = new PokeZone(team);
    mystack->addWidget(mypzone);

    connect(myforfeit, SIGNAL(clicked()), SIGNAL(forfeit()));
    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchClicked(int)));
    connect(myattack, SIGNAL(clicked()), SLOT(attackButton()));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));

    show();

    switchTo(0);
}

void BattleWindow::switchTo(int pokezone)
{
    myview->switchTo(myteam.poke(pokezone));
    currentPoke() = pokezone;
    mystack->setCurrentIndex(pokezone);
}

void BattleWindow::switchToPokeZone()
{
    if (currentPoke() < 0 && currentPoke() > 5)
	mystack->setCurrentIndex(ZoneOfPokes);
    else {
	// Go back to the attack zone if the window is on the switch zone
	if (mystack->currentIndex() == ZoneOfPokes) {
	    switchTo(currentPoke());
	} else {
	    mystack->setCurrentIndex(ZoneOfPokes);
	}
    }
}

void BattleWindow::attackClicked(int zone)
{
    if (possible)
	sendChoice(BattleChoice(false, zone));
}

void BattleWindow::switchClicked(int zone)
{
    if (!possible)
    {
	switchToPokeZone();
    } else {
	if (zone == currentPoke()) {
	    switchTo(currentPoke());
	} else {
	    switchTo(zone);
	    /* DO MESSAGE */
	    sendChoice(BattleChoice(true, zone));
	}
    }
}

void BattleWindow::attackButton()
{
    if (possible) {
	//We go with the first attack, duh
	if (possibilities.struggle()) {
	    /* DO STRUGGLE */
	    sendChoice(BattleChoice(false, -1));
	} else {
	    for (int i = 0; i < 4; i++) {
		if (possibilities.attackAllowed[i]) {
		    /* DO MESSAGE AND BREAK */
		    sendChoice(BattleChoice(false, i));
		    break;
		}
	    }
	}
    }
}

void BattleWindow::sendChoice(const BattleChoice &b)
{
    emit battleCommand(b);
    possible = false;
}

void BattleWindow::receiveInfo(QByteArray info)
{
    QDataStream in (&info, QIODevice::ReadOnly);

    uchar command;
    bool self;

    in >> command >> self;

    switch (command)
    {
	case SendPoke:
	{
	    if (self) {
		quint8 poke;
		in >> poke;
		switchTo(poke);
	    } else {
		ShallowBattlePoke poke;
		in >> poke;
		myview->switchTo(poke, false);
	    }
	    break;
	}
	case OfferChoice:
	{
	    possible = true;
	    in >> possibilities;
	    updatePossibilities();
	    break;
	}
	case BeginTurn:
	{
	    int turn;
	    in >> turn;
	    printLine(tr("Start of turn %i").arg(turn));
	    break;
	}
	default:
	    break;
    }
}

void BattleWindow::switchToNaught(bool self)
{
    myview->switchToNaught(self);

    if (self) {
	switchToPokeZone();
    }
}

void BattleWindow::printLine(const QString &str)
{
    mychat->insertPlainText(str + "\n");
}

void BattleWindow::updatePossibilities()
{
    /* moves first */
    if (currentPoke() >= 0 && currentPoke() < 6)
    {
	if (possibilities.attacksAllowed == false) {
	    myattack->setEnabled(false);
	    for (int i = 0; i < 4; i ++) {
		myazones[currentPoke()]->attacks[i]->setEnabled(false);
	    }
	} else {
	    myattack->setEnabled(true);
	    for (int i = 0; i < 4; i ++) {
		myazones[currentPoke()]->attacks[i]->setEnabled(possibilities.attackAllowed[i]);
	    }
	}
    }
    /* Then pokemon */
    if (possibilities.switchAllowed == false) {
	myswitch->setEnabled(false);
    } else {
	myswitch->setEnabled(true);
	for (int i = 0; i < 6; i++) {
	    mypzone->pokes[i]->setEnabled(team().poke(i).num() != 0 && team().poke(i).lifePoints() > 0);
	}
    }
}

TeamBattle &BattleWindow::team()
{
    return myteam;
}

const TeamBattle &BattleWindow::team() const
{
    return myteam;
}

AttackZone::AttackZone(const PokeBattle &poke)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 4; i++)
    {
	l->addWidget(attacks[i] = new QPushButton(MoveInfo::Name(poke.move(i).num())), i >= 2, i % 2);

	mymapper->setMapping(attacks[i], i);
	connect(attacks[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(clicked(int)));
}

PokeZone::PokeZone(const TeamBattle &team)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 6; i++)
    {
	l->addWidget(pokes[i] = new QPushButton(PokemonInfo::Icon(team.poke(i).num()), PokemonInfo::Name(team.poke(i).num())), i >= 3, i % 3);

	mymapper->setMapping(pokes[i], i);
	connect(pokes[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(switchTo(int)));
}

GraphicsZone::GraphicsZone()
{
    setScene(&scene);

    setFixedSize(305,305);

    scene.setSceneRect(0,0,300,300);

    mine = new QGraphicsPixmapItem();
    foe = new QGraphicsPixmapItem();

    scene.addItem(mine);
    mine->setPos(10, 300-79);

    scene.addItem(foe);
    foe->setPos(200, 0);
}

void GraphicsZone::switchToNaught(bool self)
{
    if (self)
	mine->setPixmap(QPixmap());
    else
	foe->setPixmap(QPixmap());
}

QPixmap GraphicsZone::loadPixmap(quint16 num, bool shiny, bool back, quint8 gender)
{
    qint32 key = this->key(num, shiny, back, gender);

    if (!graphics.contains(key))
	graphics.insert(key, PokemonInfo::Picture(num, gender, shiny, back));

    return graphics[key];
}

qint32 GraphicsZone::key(quint16 num, bool shiny, bool back, quint8 gender) const
{
    return num + (gender << 16) + (back << 24) + (shiny<<25);
}
