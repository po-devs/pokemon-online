#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include <iostream>
#include "../Utilities/otherwidgets.h"

BattleWindow::BattleWindow(const QString &me, const QString &opponent, int idme, int idopp, const TeamBattle &team, const BattleConfiguration &_conf)
{
    blankMessage = false;
    battleEnded = false;
    conf() = _conf;

    this->idme() = idme;
    this->idopp() = idopp;

    info().currentIndex = -1;
    info().lastIndex = 0;
    info().myteam = team;
    info().possible = false;
    info().name[0] = me;
    info().name[1] = opponent;
    info().sub[0] = false;
    info().sub[1] = false;
    info().opponentAlive = false;

    setAttribute(Qt::WA_DeleteOnClose, true);

    setWindowTitle(tr("Battling against %1").arg(opponent));
    QGridLayout *mylayout = new QGridLayout(this);

    mylayout->addWidget(mydisplay = new BattleDisplay(info()), 0, 0, 3, 2);
    mylayout->addWidget(mychat = new QScrollDownTextEdit(), 0, 2, 1, 2);
    mylayout->addWidget(myline = new QLineEdit(), 1, 2, 1, 2);
    mylayout->addWidget(myforfeit = new QPushButton(tr("&Forfeit")), 2, 2);
    mylayout->addWidget(mysend = new QPushButton(tr("Sen&d")), 2, 3);
    mylayout->addWidget(mystack = new QStackedWidget(), 3, 0, 1, 4);
    mylayout->addWidget(mycancel = new QPushButton(tr("&Cancel")), 4,0);
    mylayout->addWidget(myattack = new QPushButton(tr("&Attack")), 4, 2);
    mylayout->addWidget(myswitch = new QPushButton(tr("&Switch Pokémon")), 4, 3);

    mycancel->setDisabled(true);

    for (int i = 0; i < 6; i++) {
	myazones[i] = new AttackZone(team.poke(i));
	mystack->addWidget(myazones[i]);

	connect(myazones[i], SIGNAL(clicked(int)), SLOT(attackClicked(int)));
    }

    mypzone = new PokeZone(team);
    mystack->addWidget(mypzone);

    connect(myforfeit, SIGNAL(clicked()), SLOT(clickforfeit()));
    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchClicked(int)));
    connect(myattack, SIGNAL(clicked()), SLOT(attackButton()));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));
    connect(mycancel, SIGNAL(clicked()), SLOT(emitCancel()));

    show();

    switchTo(0);

    printHtml(toBoldColor(tr("Battle between %1 and %2 started!"), Qt::blue).arg(name(true), name(false)));
    layout()->setSizeConstraint(QLayout::SetFixedSize);
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

void BattleWindow::closeEvent(QCloseEvent *)
{
    emit forfeit();
    close();
}

void BattleWindow::emitCancel()
{
    mycancel->setDisabled(true);
    emit battleCommand(BattleChoice(false, BattleChoice::Cancel));
}

void BattleWindow::switchTo(int pokezone)
{
    info().currentIndex = pokezone;
    mystack->setCurrentIndex(pokezone);
    mydisplay->updatePoke(true);
}

void BattleWindow::clickforfeit()
{
    if (battleEnded) {
        emit forfeit();
        return;
    }

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
    mycancel->setEnabled(true);
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
                info().sub[0] = false;
		switchTo(poke);
	    } else {
		in >> info().opponent;
                std::cout << "Opp Sent out " << info().opponent.num() << std::endl;
		info().opponentAlive = true;
                info().sub[1] = false;
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
            mycancel->setDisabled(true);
	    updateChoices();
	    break;
	}
    case BeginTurn:
	{
	    int turn;
	    in >> turn;
            printLine("");
            printHtml("<span style='color:blue'><b>" + tr("Start of turn %1").arg(turn) + "</b></span>");
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
        if (self) {
            if ( info().currentIndex >= 0 && info().currentIndex < 6)
                mypzone->pokes[info().currentIndex]->setEnabled(false); //crash!!
            else
                mypzone->pokes[info().lastIndex]->setEnabled(false); //crash!!
        }
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
                printHtml(toColor(tr("It's not very effective..."), Qt::gray));
                break;
     case 8:
     case 16:
                printHtml(toColor(tr("It's super effective!"), Qt::blue));
     default:
                break;
	    }
	    break;
	}
    case CriticalHit:
        printHtml(toColor(tr("A critical hit!"), Qt::red));
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
                printHtml(toColor(escapeHtml(tu(tr("%1 fell asleep!").arg(nick(self)))), StatInfo::StatusColor(status)));
                break;
     case Pokemon::Burnt:
                printHtml(toColor(escapeHtml(tu(tr("%1 was burned!").arg(nick(self)))), StatInfo::StatusColor(status)));
                break;
     case Pokemon::Paralysed:
                printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It may be unable to move!").arg(nick(self)))), StatInfo::StatusColor(status)));
                break;
     case Pokemon::Poisoned:
                printHtml(toColor(escapeHtml(tu(tr("%1 is poisoned!").arg(nick(self)))), StatInfo::StatusColor(status)));
                break;
     case Pokemon::DeeplyPoisoned:
                printHtml(toColor(escapeHtml(tu(tr("%1 was badly poisoned!").arg(nick(self)))), StatInfo::StatusColor(status)));
                break;
     case Pokemon::Frozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 was frozen solid!").arg(nick(self)))), StatInfo::StatusColor(status)));
                break;
     case -1:
                printHtml(toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(self)))), TypeInfo::Color(Move::Ghost).name()));
                break;
	    }
            if (!self) {
                info().opponent.status() = status;
                mydisplay->updatePoke(self);
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
                printHtml(toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(self)))), TypeInfo::Color(Move::Ghost).name()));
                break;
     case HurtConfusion:
                printHtml(toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), TypeInfo::Color(Move::Ghost).name()));
                break;
     case FreeConfusion:
                printHtml(toColor(escapeHtml(tu(tr("%1 snapped out its confusion!").arg(nick(self)))), TypeInfo::Color(Move::Dark).name()));
                break;
     case PrevParalysed:
                printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(self)))), StatInfo::StatusColor(Pokemon::Paralysed)));
                break;
     case FeelAsleep:
                printHtml(toColor(escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(self)))), StatInfo::StatusColor(Pokemon::Asleep)));
                break;
     case FreeAsleep:
                printHtml(toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(self)))), TypeInfo::Color(Move::Dark).name()));
                break;
     case HurtBurn:
                printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(self)))), StatInfo::StatusColor(Pokemon::Burnt)));
                break;
     case HurtPoison:
                printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(self)))), StatInfo::StatusColor(Pokemon::Poisoned)));
                break;
     case PrevFrozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(self)))), StatInfo::StatusColor(Pokemon::Frozen)));
                break;
     case FreeFrozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(self)))), TypeInfo::Color(Move::Dark).name()));
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
            printHtml(toColor(escapeHtml(tu(mess)), TypeInfo::Color(type)));
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
            qint16 berry = 0;
            in >> item >> part >> foe >> berry;
	    QString mess = ItemInfo::Message(item, part);
	    mess.replace("%s", nick(self));
	    mess.replace("%f", nick(!self));
            mess.replace("%i", ItemInfo::Name(berry));
            mess.replace("%m", MoveInfo::Name(berry));
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
            QColor c = (weather == Hail ? TypeInfo::Color(Type::Ice) : (weather == Sunny ? TypeInfo::Color(Type::Fire) : (weather == SandStorm ? TypeInfo::Color(Type::Rock) : TypeInfo::Color(Type::Water))));
	    switch(wstatus) {
     case EndWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("The hail stopped!"),c)); break;
                case SandStorm: printHtml(toColor(tr("The sandstorm stopped!"),c)); break;
                case Sunny: printHtml(toColor(tr("The sunlight faded!"),c)); break;
                case Rain: printHtml(toColor(tr("The rain stopped!"),c)); break;
                } break;
		case HurtWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("%1 is buffeted by the hail!").arg(tu(nick(self))),c)); break;
                case SandStorm: printHtml(toColor(tr("%1 is buffeted by the sandstorm!").arg(tu(nick(self))),c)); break;
                } break;
		case StartWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("A hailstorm whipped up!"),c)); break;
                case SandStorm: printHtml(toColor(tr("A sandstorm whipped up!"),c)); break;
                case Sunny: printHtml(toColor(tr("The sunlight became harsh!"),c)); break;
                case Rain: printHtml(toColor(tr("It's started to rain!"),c)); break;
                } break;
		case ContinueWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("Hail continues to fall!"),c)); break;
                case SandStorm: printHtml(toColor(tr("The sandstorm rages!"),c)); break;
                case Sunny: printHtml(toColor(tr("The sunlight is strong!"),c)); break;
                case Rain: printHtml(toColor(tr("Rain continues to fall!"),c)); break;
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
            printHtml(toColor(escapeHtml(tu(mess)),TypeInfo::Color(type)));
        }
        break;
    }
    case AbsStatusChange:
        {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        mydisplay->changeStatus(self,poke,status);

        if (self) {
            info().myteam.poke(poke).status() = status;
            if (info().currentIndex == poke) {
                mydisplay->updatePoke(self);
            }
        }
        break;
    }
    case Substitute:
        in >> info().sub[!self];
        mydisplay->updatePoke(self);
        break;
    case BattleEnd:
    {
            printLine("");
            qint8 res;
            in >> res;
            myforfeit->setText(tr("&Close"));
            battleEnded = true;
            mycancel->setDisabled(true);
            if (res == Tie) {
                printHtml(toBoldColor(tr("Tie between %1 and %2!").arg(name(true), name(false)), Qt::blue));
            } else {
                printHtml(toBoldColor(tr("%1 won the battle!").arg(name(self)), Qt::blue));
            }
            break;
    }
    case BlankMessage:
            printLine("");
            break;
    case CancelMove:
        {
            info().possible = true;
            mycancel->setDisabled(true);
            updateChoices();
            break;
        }
    case SleepClause:
        {
            bool inBattle;
            in >> inBattle;

            if (inBattle) {
                printLine(tr("Sleep Clause prevented %1 from falling asleep!").arg(nick(self)));
            } else {
                printHtml(toBoldColor("Rule: ", Qt::blue) + "Sleep Clause Enabled.");
            }
            break;
        }
    default:
        break;
    }
}

void BattleWindow::switchToNaught(bool self)
{
    if (self) {
        if (info().currentIndex != -1)
            info().lastIndex = info().currentIndex;
	info().currentIndex = -1;
	switchToPokeZone();
    } else {
	info().opponentAlive = false;
    }

    mydisplay->updatePoke(self);
}

void BattleWindow::printLine(const QString &str)
{
    if (str == "" && blankMessage) {
        return;
    }
    blankMessage = str == "";

    mychat->insertPlainText(str + "\n");
}

void BattleWindow::printHtml(const QString &str)
{
    blankMessage = false;
    mychat->insertHtml(str + "<br />");
}

void BattleWindow::updateChoices()
{
    if (info().choices.attacksAllowed == false && info().choices.switchAllowed == true)
        mystack->setCurrentIndex(ZoneOfPokes);

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

    QHBoxLayout *foeteam = new QHBoxLayout();
    l->addLayout(foeteam);
    for (int i = 0; i < 6; i++) {
        advpokeballs[i] = new QLabel();
        advpokeballs[i]->setPixmap(StatInfo::Icon(Pokemon::Fine));
        foeteam->addWidget(advpokeballs[i]);
    }

    status[Opponent] = new QLabel();
    foeteam->addWidget(status[Opponent], 100, Qt::AlignCenter);

    gender[Opponent] = new QLabel();
    foeteam->addWidget(gender[Opponent]);

    nick[Opponent] = new QLabel(info.name[Opponent]);
    foeteam->addWidget(nick[Opponent], 0, Qt::AlignRight);
    foeteam->setSpacing(1);

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

    QHBoxLayout *team = new QHBoxLayout();
    team->setSpacing(1);

    l->addLayout(team);

    gender[Myself] = new QLabel();
    team->addWidget(gender[Myself]);

    nick[Myself] = new QLabel(info.name[Myself]);
    team->addWidget(nick[Myself], 0, Qt::AlignLeft);

    status[Myself] = new QLabel();
    team->addWidget(status[Myself], 100, Qt::AlignCenter);

    team->addWidget(new QLabel(), 100);
    for (int i = 0; i < 6; i++) {
        mypokeballs[i] = new QLabel();
        mypokeballs[i]->setPixmap(StatInfo::Icon(Pokemon::Fine));
        mypokeballs[i]->setToolTip(info.myteam.poke(i).nick());
        team->addWidget(mypokeballs[i]);
    }

    updatePoke(true);
    updatePoke(false);
}

void BattleDisplay::updatePoke(bool self)
{
    if (self) {
	if (info.currentIndex != -1) {
            zone->switchTo(mypoke(), self, info.sub[Myself]);
            nick[Myself]->setText(tr("%1 Lv.%2").arg(mypoke().nick()).arg(mypoke().level()));
            bars[Myself]->setRange(0,mypoke().totalLifePoints());
            bars[Myself]->setValue(mypoke().lifePoints());
            bars[Myself]->setStyleSheet(health(mypoke().lifePoints()*100/mypoke().totalLifePoints()));
            gender[Myself]->setPixmap(GenderInfo::Picture(info.currentPoke().gender()));
            int status = info.myteam.poke(info.currentIndex).status();
            this->status[Myself]->setText(toBoldColor(StatInfo::ShortStatus(status), StatInfo::StatusColor(status)));
        } else {
            zone->switchToNaught(self);
            nick[Myself]->setText("");
            gender[Myself]->setPixmap(QPixmap());
            bars[Myself]->setValue(0);
        }
    }
    else {
	if (info.opponentAlive) {
            zone->switchTo(foe(), self, info.sub[Opponent]);
            nick[Opponent]->setText(tr("%1 Lv.%2").arg(foe().nick()).arg(foe().level()));
            bars[Opponent]->setValue(foe().lifePercent());
            bars[Opponent]->setStyleSheet(health(foe().lifePercent()));
            gender[Opponent]->setPixmap(GenderInfo::Picture(info.opponent.gender()));
            int status = info.opponent.status();
            this->status[Opponent]->setText(toBoldColor(StatInfo::ShortStatus(status), StatInfo::StatusColor(status)));
        }  else {
            zone->switchToNaught(self);
            nick[Opponent]->setText("");
            gender[Opponent]->setPixmap(QPixmap());
            bars[Opponent]->setValue(0);
        }
    }
}

void BattleDisplay::changeStatus(bool self, int poke, int status) {
    if (self) {
        mypokeballs[poke]->setPixmap(StatInfo::Icon(status));
    } else {
        advpokeballs[poke]->setPixmap(StatInfo::Icon(status));
    }
}

QString BattleDisplay::health(int lifePercent)
{
    return lifePercent > 50 ? "::chunk{background-color: #05B8CC;}" : (lifePercent >= 34 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
}

GraphicsZone::GraphicsZone()
{
    setScene(&scene);

    scene.setSceneRect(0,0,257,145);
    scene.addItem(new QGraphicsPixmapItem(QPixmap(QString("db/battle_fields/%1.png").arg((rand()%11)+1))));

    mine = new QGraphicsPixmapItem();
    foe = new QGraphicsPixmapItem();

    scene.addItem(mine);
    mine->setPos(10, 145-79);

    scene.addItem(foe);
    foe->setPos(257-105, 16);
}

void GraphicsZone::switchToNaught(bool self)
{
    if (self)
	mine->setPixmap(QPixmap());
    else
	foe->setPixmap(QPixmap());
}

QPixmap GraphicsZone::loadPixmap(quint16 num, bool shiny, bool back, quint8 gender, bool sub)
{
    qint32 key = this->key(num, shiny, back, gender, sub);

    if (!graphics.contains(key)) {
        if (sub) {
            graphics.insert(key, PokemonInfo::Sub(back));
        } else {
            graphics.insert(key, PokemonInfo::Picture(num, gender, shiny, back));
        }
    }

    return graphics[key];
}

qint32 GraphicsZone::key(quint16 num, bool shiny, bool back, quint8 gender, bool sub) const
{
    return sub ? ((1 << 31) + (back << 30)) : (num + (gender << 16) + (back << 24) + (shiny<<25));
}

const PokeBattle & BattleInfo::currentPoke() const
{
    if (currentIndex == -1) {
	qDebug() << "Error! Call for pokémon info while none on the field. Returning a safe value instead";
        return myteam.poke(lastIndex);
    } else {
	return myteam.poke(currentIndex);
    }
}

PokeBattle & BattleInfo::currentPoke()
{
    if (currentIndex == -1) {
	qDebug() << "Error! Call for pokémon info while none on the field. Returning a safe value instead";
        return myteam.poke(lastIndex);
    } else {
	return myteam.poke(currentIndex);
    }
}
