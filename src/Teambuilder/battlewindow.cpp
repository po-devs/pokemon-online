#include "battlewindow.h"

BattleWindow::BattleWindow(const QString &opp)
{
    mycurrentpoke = 0;

    setWindowTitle(tr("Battling against %1").arg(opp));
    QGridLayout *mylayout = new QGridLayout(this);

    mylayout->addWidget(myview = new QGraphicsView(), 0, 0, 3, 1);
    mylayout->addWidget(mychat = new QTextEdit(), 0, 1, 1, 2);
    mylayout->addWidget(myline = new QLineEdit(), 1, 1, 1, 2);
    mylayout->addWidget(myforfeit = new QPushButton(tr("&Forfeit")), 2, 1);
    mylayout->addWidget(mysend = new QPushButton(tr("&Send")), 2, 2);
    mylayout->addWidget(mystack = new QStackedWidget(), 3, 0, 1, 3);
    mylayout->addWidget(myattack = new QPushButton(tr("&Attack")), 4, 1);
    mylayout->addWidget(myswitch = new QPushButton(tr("&Switch Pokémons")), 4, 2);

    for (int i = 0; i < 6; i++) {
	myazones[i] = new AttackZone();
	mystack->addWidget(myazones[i]);
    }

    mypzone = new PokeZone();
    mystack->addWidget(mypzone);

    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchTo(int)));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));

    show();
}

void BattleWindow::switchTo(int pokezone)
{
    mystack->setCurrentIndex(pokezone);
}

void BattleWindow::switchToPokeZone()
{
    mystack->setCurrentIndex(6);
}

int BattleWindow::currentPoke() const
{
    return mycurrentpoke;
}

TeamBattle &BattleWindow::team()
{
    return myteam;
}

const TeamBattle &BattleWindow::team() const
{
    return myteam;
}

AttackZone::AttackZone()
{
    QGridLayout *l = new QGridLayout(this);

    for (int i = 0; i < 4; i++)
    {
	l->addWidget(attacks[i] = new QPushButton(tr("Move %1").arg(i)), i >= 2, i % 2);
    }
}

PokeZone::PokeZone()
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 6; i++)
    {
	l->addWidget(pokes[i] = new QPushButton(tr("Pokémon %1").arg(i)), i >= 3, i % 3);

	mymapper->setMapping(pokes[i], i);
	connect(pokes[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(switchTo(int)));
}
