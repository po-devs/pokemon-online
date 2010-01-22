#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include <iostream>
#include "../Utilities/otherwidgets.h"

BattleWindow::BattleWindow(const QString &me, const QString &opponent, int idme, int idopp, const TeamBattle &team, const BattleConfiguration &_conf)
{
    conf() = _conf;

    this->idme() = idme;
    this->idopp() = idopp;

    info().currentIndex = -1;
    info().myteam = team;
    info().possible = false;
    info().name[0] = me;
    info().name[1] = opponent;
    info().opponentAlive = false;

    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Battling against %1").arg(opponent));
    QGridLayout *mylayout = new QGridLayout(this);

    mylayout->addWidget(mydisplay = new BattleDisplay(info()), 0, 0, 3, 1);
    mylayout->addWidget(mychat = new QScrollDownTextEdit(), 0, 1, 1, 2);
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

    mychat->setReadOnly(true);

    mypzone = new PokeZone(team);
    mystack->addWidget(mypzone);

    connect(myforfeit, SIGNAL(clicked()), SLOT(clickforfeit()));
    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchClicked(int)));
    connect(myattack, SIGNAL(clicked()), SLOT(attackButton()));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));

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
	return "the foe's " + info().opponent.nick();
}

QString BattleWindow::rnick(bool self) const
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

void BattleWindow::clickforfeit()
{
    if (QMessageBox::question(this, tr("Losing your battle"), tr("Do you mean to forfeit?"), QMessageBox::Yes | QMessageBox::No)
	    == QMessageBox::Yes)
	emit forfeit();
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
    updateChoices();
}

void BattleWindow::sendMessage()
{
    QString message = myline->text();

    if (message.size() != 0) {
	emit battleMessage(message);
	myline->clear();
    }
}

void BattleWindow::receiveInfo(QByteArray inf)
{
    QDataStream in (&inf, QIODevice::ReadOnly);

    uchar command;
    qint8 player;

    in >> command >> player;

    bool self = conf().ids[player] == idme();

    std::cout << "Command received! num #" << int(command) << " self is " << self << std::endl;

    switch (command)
    {
	case SendOut:
	{
            std::cout << "Poke sent out!" << std::endl;
	    if (self) {
		quint8 poke;
		in >> poke;
                std::cout << "Sent out " << int(poke) << std::endl;
		switchTo(poke);
	    } else {
		in >> info().opponent;
                std::cout << "Opp Sent out " << info().opponent.num() << std::endl;
		info().opponentAlive = true;
		mydisplay->updatePoke(false);
	    }

	    printLine(tr("%1 sent out %2!").arg(name(self), rnick(self)));

	    break;
	}
	case SendBack:
            std::cout << "Poke sent back!" << std::endl;
	    printLine(tr("%1 called %2 back!").arg(name(self), rnick(self)));
	    switchToNaught(self);
	    break;
	case UseAttack:
	{
            std::cout << "Attack used!" << std::endl;
	    qint16 attack;
	    in >> attack;

	    //Think to check for crash if attack is invalid
	    printHtml(tr("%1 used <span style='color:%2'><b>%3</b></span>!").arg(escapeHtml(tu(nick(self))), TypeInfo::Color(MoveInfo::Type(attack)).name(), MoveInfo::Name(attack)));
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
	    printHtml("<b>" + escapeHtml(tu(tr("%1 fainted!").arg(nick(self)))) + "</b>");
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
	    printHtml(tr("<span style='color:red'>A critical hit!</span>"));
	    break;
	case Miss:
	    printLine(tr("The attack of %1 missed!").arg(nick(self)));
	    break;
	case StatChange:
	    qint8 stat, boost;
	    in >> stat >> boost;

	    printLine(tu(tr("%1's %2 %3%4!").arg(nick(self), StatInfo::Stat(stat), abs(boost) > 1 ? tr("sharply ") : "", boost > 0 ? tr("rose") : tr("fell"))));
	    break;
	case StatusChange:
	{
	    qint8 status;
	    in >> status;
	    switch(status)
	    {
		case Pokemon::Asleep:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Psychic).name() + "'>" + escapeHtml(tu(tr("%1 fell asleep!").arg(nick(self)))) + "</span>");
		    break;
		case Pokemon::Burnt:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Fire).name() + "'>" + escapeHtml(tu(tr("%1 was burned!").arg(nick(self)))) + "</span>");
		    break;
		case Pokemon::Paralysed:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Electric).name() + "'>" + escapeHtml(tu(tr("%1 is paralyzed! It may be unable to move!").arg(nick(self)))) + "</span>");
		    break;
		case Pokemon::Poisoned:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Poison).name() + "'>" + escapeHtml(tu(tr("%1 was poisoned!").arg(nick(self)))) + "</span>");
		    break;
		case Pokemon::DeeplyPoisoned:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Poison).name() + "'>" + escapeHtml(tu(tr("%1 was badly poisoned!").arg(nick(self)))) + "</span>");
		    break;
		case Pokemon::Frozen:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Ice).name() + "'>" + escapeHtml(tu(tr("%1 was frozen solid!").arg(nick(self)))) + "</span>");
		    break;
		case -1:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Ghost).name() + "'>" + escapeHtml(tu(tr("%1 became confused!").arg(nick(self)))) + "</span>");
		    break;
	    }
	    break;
	}
	case StatusMessage:
	{
	    qint8 status;
	    in >> status;
	    switch(status)
	    {
		case FeelConfusion:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Ghost).name() + "'>" + escapeHtml(tu(tr("%1 is confused!").arg(nick(self)))) + "</span>");
		    break;
		case HurtConfusion:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Ghost).name() + "'>" + escapeHtml("It hurt itself in its confusion!") + "</span>");
		    break;
		case FreeConfusion:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Dark).name() + "'>" + escapeHtml(tu(tr("%1 snapped out of its confusion!").arg(nick(self)))) + "</span>");
		    break;
		case PrevParalysed:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Electric).name() + "'>" + escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(self)))) + "</span>");
		    break;
		case FeelAsleep:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Psychic).name() + "'>" + escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(self)))) + "</span>");
		    break;
		case FreeAsleep:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Dark).name() + "'>" + escapeHtml(tu(tr("%1 woke up!").arg(nick(self)))) + "</span>");
		    break;
		case HurtBurn:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Fire).name() + "'>" + escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(self)))) + "</span>");
		    break;
		case HurtPoison:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Poison).name() + "'>" + escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(self)))) + "</span>");
		    break;
		case PrevFrozen:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Ice).name() + "'>" + escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(self)))) + "</span>");
		    break;
		case FreeFrozen:
		    printHtml("<span style='color:" + TypeInfo::Color(Move::Dark).name() + "'>" + escapeHtml(tu(tr("%1 thawed out!").arg(nick(self)))) + "</span>");
		    break;
	    }
	}
	    break;
	case Failed:
	    printLine(tr("But it failed!"));
	    break;
	case BattleChat:
	{
	    QString message;
	    in >> message;
	    printHtml(QString("<span style='color:") + (self?"#5811b1":"green") + "'><b>" + escapeHtml(name(self)) + ": </b></span>" + escapeHtml(message));
	    break;
	}
	case MoveMessage:
	{
	    quint16 move=0;
	    uchar part=0;
	    qint8 type(0), foe(0);
	    qint16 other(0);
	    QString q;
	    in >> move >> part >> type >> foe >> other >> q;
	    QString mess = MoveInfo::MoveMessage(move,part);
	    mess.replace("%s", nick(self));
	    mess.replace("%ts", name(self));
	    mess.replace("%tf", name(!self));
	    mess.replace("%t", TypeInfo::Name(type));
	    mess.replace("%f", nick(!self));
	    mess.replace("%m", MoveInfo::Name(other));
	    mess.replace("%d", QString::number(other));
	    mess.replace("%q", q);
	    mess.replace("%i", ItemInfo::Name(other));
	    mess.replace("%a", AbilityInfo::Name(other));
	    mess.replace("%p", PokemonInfo::Name(other));
	    printHtml("<span style='color:" + TypeInfo::Color(type).name() + "'>" + escapeHtml(tu(mess)) + "</span>");
	    break;
	}
	case NoOpponent:
	    printLine(tr("But there is no target pokémon!"));
	    break;
	case ItemMessage:
	{
	    quint16 item=0;
	    uchar part=0;
	    qint8 foe = 0;
	    in >> item >> part >> foe;
	    QString mess = ItemInfo::Message(item, part);
	    mess.replace("%s", nick(self));
	    mess.replace("%f", nick(!self));
	    printLine(tu(mess));
	    break;
	}
	case Flinch:
	    printLine(tu(tr("%1 flinched!").arg(nick(self))));
	    break;
	case Recoil:
	    printLine(tu(tr("%1 is hit with recoil!").arg(nick(self))));
	    break;
        case WeatherMessage: {
	    qint8 wstatus, weather;
	    in >> wstatus >> weather;
	    if (weather == NormalWeather)
		break;
	    QString beg = "<span style='color:" + (weather == Hail ? TypeInfo::Color(Type::Ice) : (weather == Sunny ? TypeInfo::Color(Type::Fire) : (weather == SandStorm ? TypeInfo::Color(Type::Rock) : TypeInfo::Color(Type::Water)))).name()
			  + "'>";
	    QString end = "</span>";
	    switch(wstatus) {
		case EndWeather:
		    switch(weather) {
			case Hail: printHtml(beg + tr("The hail stopped!") + end); break;
			case SandStorm: printHtml(beg + tr("The sandstorm stopped!") + end); break;
			case Sunny: printHtml(beg + tr("The sunlight faded!") + end); break;
			case Rain: printHtml(beg + tr("The rain stopped!") + end); break;
		    } break;
		case HurtWeather:
		    switch(weather) {
			case Hail: printHtml(beg + tu(tr("%1 is buffeted by the hail!").arg(nick(self)) + end)); break;
			case SandStorm: printHtml(beg + tu(tr("%1 is buffeted by the sandstorm!").arg(nick(self))) + end); break;
		    } break;
		case StartWeather:
		    switch(weather) {
			case Hail: printHtml(beg + tr("A hailstorm whipped up!") + end); break;
			case SandStorm: printHtml(beg + tr("A sandstorm whipped up!") + end); break;
			case Sunny: printHtml(beg + tr("The sunlight became harsh!") + end); break;
			case Rain: printHtml(beg + tr("It's started to rain!") + end); break;
		    } break;
		case ContinueWeather:
		    switch(weather) {
			case Hail: printHtml(beg + tr("Hail continues to fall!") + end); break;
			case SandStorm: printHtml(beg + tr("The sandstorm rages!") + end); break;
			case Sunny: printHtml(beg + tr("The sunlight is strong!") + end); break;
			case Rain: printHtml(beg + tr("Rain continues to fall!") + end); break;
		    } break;
	    }
        } break;
	case StraightDamage :
	{
	    qint16 damage;
	    in >> damage;
	    if (self) {
		printLine(tr("%1 lost %2 HP! (%3% of its health)").arg(nick(self)).arg(damage).arg(damage*100/info().currentPoke().totalLifePoints()));
	    } else {
		printLine(tu(tr("%1 lost %2% of its health!").arg(nick(self)).arg(damage)));
	    }
	    break;
	}
	default:
	    break;
    }
<<<<<<< .mine
    case AbilityMessage:
        {
            quint16 ab=0;
            uchar part=0;
            qint8 type(0), foe(0);
            qint16 other(0);
            in >> ab >> part >> type >> foe >> other;
            QString mess = AbilityInfo::Message(ab,part);
            mess.replace("%s", nick(self));
//            mess.replace("%ts", name(self));
//            mess.replace("%tf", name(!self));
            mess.replace("%t", TypeInfo::Name(type));
            mess.replace("%f", nick(!self));
            mess.replace("%m", MoveInfo::Name(other));
//            mess.replace("%d", QString::number(other));
            mess.replace("%i", ItemInfo::Name(other));
            mess.replace("%a", AbilityInfo::Name(other));
//            mess.replace("%p", PokemonInfo::Name(other));
            if (type == Pokemon::Normal) {
                printLine(escapeHtml(tu(mess)));
            } else {
                printHtml("<span style='color:" + TypeInfo::Color(type).name() + "'>" + escapeHtml(tu(mess)) + "</span>");
            }
            break;
        }
    default:
        break;
    }
=======
>>>>>>> .r267
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
    
    if (!info().possible) {
	myattack->setEnabled(false);
	myswitch->setEnabled(false);
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
    setMinimumHeight(30);

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
    return lifePercent > 50 ? "::chunk{background-color: #05B8CC;}" : (lifePercent >= 34 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
}

GraphicsZone::GraphicsZone()
{
    setScene(&scene);

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
    if (currentIndex == -1) {
	qDebug() << "Error! Call for pokémon info while none on the field. Returning a safe value instead";
	return myteam.poke(0);
    } else {
	return myteam.poke(currentIndex);
    }
}

PokeBattle & BattleInfo::currentPoke()
{
    if (currentIndex == -1) {
	qDebug() << "Error! Call for pokémon info while none on the field. Returning a safe value instead";
	return myteam.poke(0);
    } else {
	return myteam.poke(currentIndex);
    }
}
