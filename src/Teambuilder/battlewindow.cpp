#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"

BattleWindow::BattleWindow(const QString &opp, const TeamBattle &team)
{
    mycurrentpoke = 0;
    myteam = team;

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
    }

    mypzone = new PokeZone(team);
    mystack->addWidget(mypzone);

    connect(myforfeit, SIGNAL(clicked()), SIGNAL(forfeit()));
    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchTo(int)));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));

    show();

    switchTo(0);
}

void BattleWindow::switchTo(int pokezone)
{
    myview->switchTo(myteam.poke(pokezone));
    mystack->setCurrentIndex(pokezone);
}

void BattleWindow::switchToPokeZone()
{
    mystack->setCurrentIndex(ZoneOfPokes);
}

int BattleWindow::currentPoke() const
{
    return mycurrentpoke;
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
	default:
	    break;
    }
}

void BattleWindow::switchToNaught(bool self)
{
    myview->switchToNaught(self);
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

    for (int i = 0; i < 4; i++)
    {
	l->addWidget(attacks[i] = new QPushButton(MoveInfo::Name(poke.move(i).num())), i >= 2, i % 2);
    }
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
