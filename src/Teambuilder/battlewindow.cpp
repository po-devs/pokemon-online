#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"

BattleWindow::BattleWindow(const QString & my, const QString &opp, const TeamBattle &team)
{
    info().currentIndex = -1;
    info().myteam = team;
    info().possible = false;
    info().name[0] = my;
    info().name[1] = opp;
    info().opponentAlive = false;

    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Battling against %1").arg(opp));
    QGridLayout *mylayout = new QGridLayout(this);

    mylayout->addWidget(mydisplay = new BattleDisplay(info()), 0, 0, 3, 1);
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

QString BattleWindow::name(bool self) const
{
    return info().name[!self];
}

QString BattleWindow::nick(bool self) const
{
    if (self)
	return info().currentPoke().nick();
    else
	return info().opponent.nick();
}

void BattleWindow::switchTo(int pokezone)
{
    info().currentIndex = pokezone;
    mystack->setCurrentIndex(pokezone);
    mydisplay->updatePoke(true);
}

void BattleWindow::switchToPokeZone()
{
    if (info().currentIndex < 0 || info().currentIndex > 5)
	mystack->setCurrentIndex(ZoneOfPokes);
    else {
	// Go back to the attack zone if the window is on the switch zone
	if (mystack->currentIndex() == ZoneOfPokes) {
	    switchTo(info().currentIndex);
	} else {
	    mystack->setCurrentIndex(ZoneOfPokes);
	}
    }
}

void BattleWindow::attackClicked(int zone)
{
    if (info().possible)
	sendChoice(BattleChoice(false, zone));
}

void BattleWindow::switchClicked(int zone)
{
    if (!info().possible)
    {
	switchToPokeZone();
    } else {
	if (zone == info().currentIndex) {
	    switchTo(info().currentIndex);
	} else {
	    /* DO MESSAGE */
	    sendChoice(BattleChoice(true, zone));
	}
    }
}

void BattleWindow::attackButton()
{
    if (info().possible) {
	//We go with the first attack, duh
	if (info().choices.struggle()) {
	    /* DO STRUGGLE */
	    sendChoice(BattleChoice(false, -1));
	} else {
	    for (int i = 0; i < 4; i++) {
		if (info().choices.attackAllowed[i]) {
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
    info().possible = false;
}

void BattleWindow::receiveInfo(QByteArray inf)
{
    QDataStream in (&inf, QIODevice::ReadOnly);

    uchar command;
    bool self;

    in >> command >> self;

    switch (command)
    {
	case SendOut:
	{
	    if (self) {
		quint8 poke;
		in >> poke;
		switchTo(poke);
	    } else {
		in >> info().opponent;
		info().opponentAlive = true;
		mydisplay->updatePoke(false);
	    }

	    printLine(tr("%1 sent out %2!").arg(name(self), nick(self)));

	    break;
	}
	case SendBack:
	    printLine(tr("%1 called %2 back!").arg(name(self), nick(self)));
	    switchToNaught(self);
	    break;
	case UseAttack:
	{
	    qint16 attack;
	    in >> attack;

	    //Think to check for crash if attack is invalid
	    printHtml(tr("%1 used <span style='color:%2'><b>%3</b></span>!").arg(escapeHtml(nick(self)), TypeInfo::Color(MoveInfo::Type(attack)).name(), MoveInfo::Name(attack)));
	    break;
	}
	case ChangePP:
	{
	    quint8 move, PP;
	    in  >> move >> PP;

	    //Think to check for crash if currentIndex != -1, move > 3
	    info().currentPoke().move(move).PP() = PP;
	    myazones[info().currentIndex]->attacks[move]->updateAttack(info().currentPoke().move(move));
	}
	case OfferChoice:
	{
	    info().possible = true;
	    in >> info().choices;
	    updateChoices();
	    break;
	}
	case BeginTurn:
	{
	    int turn;
	    in >> turn;
	    printHtml("<br /><span style='color:blue'><b>" + tr("Start of turn %1").arg(turn) + "</b></span>");
	    break;
	}
	case ChangeHp:
	{
	    quint16 newHp;
	    in >> newHp;
	    if (self) {
		/* Think to check for crash */
		info().currentPoke().lifePoints() = newHp;
	    } else {
		info().opponent.lifePercent() = newHp;
	    }
	    mydisplay->updatePoke(self);
	    break;
	}
	case Ko:
	    printLine(tr("%1 fainted!").arg(nick(self)));
	    switchToNaught(self);
	    break;
	case Hit:
	    printLine(tr("Hit!"));
	    break;
	case Effective:
	{
	    quint8 eff;
	    in >> eff;
	    switch (eff) {
		case 0:
		    printLine(tr("It had no effect!"));
		    break;
		case 1:
		case 2:
		    printHtml("<span style='color:grey'>" + tr("It's not very effective...") + "</span>");
		    break;
		case 8:
		case 16:
		    printHtml("<span style='color:blue'>" + tr("It's super effective!") + "</span>");
		default:
		    break;
	    }
	    break;
	}
	case CriticalHit:
	    printHtml(tr("<span style='color:red'>It's a critical hit!</span>"));
	    break;
	case Miss:
	    printLine(tr("It missed!"));
	    break;
	default:
	    break;
    }
}

void BattleWindow::switchToNaught(bool self)
{
    if (self) {
	info().currentIndex = -1;
	switchToPokeZone();
    } else {
	info().opponentAlive = false;
    }

    mydisplay->updatePoke(self);
}

void BattleWindow::printLine(const QString &str)
{
    mychat->insertPlainText(str + "\n");
}

void BattleWindow::printHtml(const QString &str)
{
    mychat->insertHtml(str + "<br />");
}

void BattleWindow::updateChoices()
{
    /* moves first */
    if (info().currentIndex != -1)
    {
	if (info().choices.attacksAllowed == false) {
	    myattack->setEnabled(false);
	    for (int i = 0; i < 4; i ++) {
		myazones[info().currentIndex]->attacks[i]->setEnabled(false);
	    }
	} else {
	    myattack->setEnabled(true);
	    for (int i = 0; i < 4; i ++) {
		myazones[info().currentIndex]->attacks[i]->setEnabled(info().choices.attackAllowed[i]);
	    }
	}
    }
    /* Then pokemon */
    if (info().choices.switchAllowed == false) {
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
    return info().myteam;
}

const TeamBattle &BattleWindow::team() const
{
    return info().myteam;
}

AttackZone::AttackZone(const PokeBattle &poke)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 4; i++)
    {
	l->addWidget(attacks[i] = new AttackButton(poke.move(i)), i >= 2, i % 2);

	mymapper->setMapping(attacks[i], i);
	connect(attacks[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(clicked(int)));
}

AttackButton::AttackButton(const BattleMove &b)
{
    QHBoxLayout *l = new QHBoxLayout(this);

    l->addWidget(name = new QLabel());
    l->addWidget(pp = new QLabel(), 0, Qt::AlignRight);

    updateAttack(b);
}

void AttackButton::updateAttack(const BattleMove &b)
{
    name->setText(MoveInfo::Name(b.num()));
    pp->setText(tr("%1/%2").arg(b.PP()).arg(b.totalPP()));
    this->setStyleSheet("background: " + TypeInfo::Color(MoveInfo::Type(b.num())).name() + ";");
}

PokeZone::PokeZone(const TeamBattle &team)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 6; i++)
    {
	l->addWidget(pokes[i] = new QPushButton(PokemonInfo::Icon(team.poke(i).num()), team.poke(i).nick()), i >= 3, i % 3);

	mymapper->setMapping(pokes[i], i);
	connect(pokes[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(switchTo(int)));
}

BattleDisplay::BattleDisplay(const BattleInfo &i)
	: info(i)
{
    QVBoxLayout *l=  new QVBoxLayout(this);

    /* As anyway the graphicsZone is a fixed size, it's useless to
       resize that part, might as well let  the chat be resized */
    l->setSizeConstraint(QLayout::SetFixedSize);

    nick[Opponent] = new QLabel(info.name[Opponent]);
    l->addWidget(nick[Opponent]);

    bars[Opponent] = new QProgressBar();
    bars[Opponent]->setObjectName("LifePoints"); /* for stylesheets */
    bars[Opponent]->setRange(0, 100);
    l->addWidget(bars[Opponent]);

    zone = new GraphicsZone();
    l->addWidget(zone);

    bars[Myself] = new QProgressBar();
    bars[Myself]->setObjectName("LifePoints"); /* for stylesheets */
    bars[Myself]->setRange(0,100);
    bars[Myself]->setFormat("%v / %m");
    l->addWidget(bars[Myself]);

    nick[Myself] = new QLabel(info.name[Myself]);
    l->addWidget(nick[Myself]);

    updatePoke(true);
    updatePoke(false);
}

void BattleDisplay::updatePoke(bool self)
{
    if (self)
	if (info.currentIndex != -1) {
	    zone->switchTo(mypoke(), self);
	    nick[Myself]->setText(tr("%1 (Lv. %2)").arg(mypoke().nick()).arg(mypoke().level()));
	    bars[Myself]->setRange(0,mypoke().totalLifePoints());
	    bars[Myself]->setValue(mypoke().lifePoints());
	    bars[Myself]->setStyleSheet(health(mypoke().lifePoints()*100/mypoke().totalLifePoints()));
	} else {
	    zone->switchToNaught(self);
	    nick[Myself]->setText("");
	    bars[Myself]->setValue(0);
	}
    else
	if (info.opponentAlive) {
	    zone->switchTo(foe(), self);
	    nick[Opponent]->setText(tr("%1 (Lv. %2)").arg(foe().nick()).arg(foe().level()));
	    bars[Opponent]->setValue(foe().lifePercent());
	    bars[Opponent]->setStyleSheet(health(foe().lifePercent()));
	}  else {
	    zone->switchToNaught(self);
	    nick[Opponent]->setText("");
	    bars[Opponent]->setValue(0);
	}
}

QString BattleDisplay::health(int lifePercent)
{
    return lifePercent > 50 ? "::chunk{background-color: #05B8CC;}" : (lifePercent >= 34 ? "::chunk{background-color: yellow;}" : "::chunk{background-color: red;}");
}

GraphicsZone::GraphicsZone()
{
    setScene(&scene);

    setFixedSize(255,255);

    scene.setSceneRect(0,0,250,250);

    mine = new QGraphicsPixmapItem();
    foe = new QGraphicsPixmapItem();

    scene.addItem(mine);
    mine->setPos(10, 250-79);

    scene.addItem(foe);
    foe->setPos(250-100, 0);
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

const PokeBattle & BattleInfo::currentPoke() const
{
    return myteam.poke(currentIndex);
}

PokeBattle & BattleInfo::currentPoke()
{
    return myteam.poke(currentIndex);
}
