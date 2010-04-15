#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include <iostream>
#include "../Utilities/otherwidgets.h"
#include "basebattlewindow.h"
#include "client.h"

BattleInfo::BattleInfo(const TeamBattle &team, const QString &me, const QString &opp)
    : BaseBattleInfo(me, opp)
{
    possible = false;
    myteam = team;

    for (int i = 0; i < 6; i++) {
        pokemons[Myself][i] = team.poke(i);
    }
}

const PokeBattle & BattleInfo::currentPoke() const
{
    return myteam.poke(currentIndex[Myself]);
}

PokeBattle & BattleInfo::currentPoke()
{
    return myteam.poke(currentIndex[Myself]);
}

BattleWindow::BattleWindow(const QString &me, const QString &opponent, int idme, int idopp, const TeamBattle &team, const BattleConfiguration &_conf)
{
    myInfo = new BattleInfo(team, me, opponent);
    mydisplay = new BattleDisplay(info());
    BaseBattleWindow::init();

    conf() = _conf;

    this->idme() = idme;
    this->idopp() = idopp;

    setWindowTitle(tr("Battling against %1").arg(opponent));

    myclose->setText(tr("&Forfeit"));
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

    mypzone = new PokeZone(info().myteam);
    mystack->addWidget(mypzone);

    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchClicked(int)));
    connect(myattack, SIGNAL(clicked()), SLOT(attackButton()));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));
    connect(mycancel, SIGNAL(clicked()), SLOT(emitCancel()));

    switchTo(0);
    show();
    printHtml(toBoldColor(tr("Battle between %1 and %2 started!"), Qt::blue).arg(name(true), name(false)));
}

QString BattleWindow::nick(int spot) const
{
    if (spot == Myself)
	return info().currentPoke().nick();
    else
        return tr("the foe's %1").arg(info().currentShallow(Opponent).nick());
}


void BattleWindow::closeEvent(QCloseEvent *)
{
    emit forfeit();

    QSettings s;

    if (s.value("save_battle_logs").toBool()) {
        QString directory = s.value("battle_logs_directory").toString();
        QString file = QFileDialog::getSaveFileName(0,QObject::tr("Saving the battle"),directory+info().name[0] + " vs " + info().name[1]
                                     + "--" + QDate::currentDate().toString("dd MMMM yyyy") + "_" +QTime::currentTime().toString("hh'h'mm")
                                     , QObject::tr("html (*.html)\ntxt (*.txt)"));
        if (file.length() != 0) {
            QFileInfo finfo (file);
            directory = finfo.dir().path() + "/";
            if (directory == "/") {
                directory = "./";
            }
            QFile out (file);
            out.open(QIODevice::WriteOnly);

            if (finfo.suffix() == "html") {
                out.write(mychat->toHtml().toUtf8());
            } else {
#ifdef WIN32
                out.write(mychat->toPlainText().replace("\n", "\r\n").toUtf8());
#else
                out.write(mychat->toPlainText().toUtf8());
#endif
            }
        }
    }
    close();
}

void BattleWindow::emitCancel()
{
    mycancel->setDisabled(true);
    emit battleCommand(BattleChoice(false, BattleChoice::Cancel));
}

void BattleWindow::switchTo(int pokezone, bool forced)
{
    if (info().currentIndex[Myself] != pokezone || forced) {
        info().currentIndex[Myself] = pokezone;
        info().currentShallow(Myself) = info().myteam.poke(pokezone);
        info().tempPoke() = info().myteam.poke(pokezone);
        info().pokeAlive[Myself] = info().currentShallow(Myself).status() != Pokemon::Koed;
        info().tempPoke() = info().currentPoke();
    }

    mystack->setCurrentIndex(pokezone);
    myattack->setText(tr("&Attack"));
    mydisplay->updatePoke(Myself);

    for (int i = 0; i<4; i++) {
        myazones[info().currentIndex[Myself]]->attacks[i]->updateAttack(info().tempPoke().move(i));
    }
}

void BattleWindow::clickClose()
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
    if (info().currentIndex[Myself] < 0 || info().currentIndex[Myself] > 5) {
	mystack->setCurrentIndex(ZoneOfPokes);
        myattack->setText("&Go back");
    }
    else {
	// Go back to the attack zone if the window is on the switch zone
	if (mystack->currentIndex() == ZoneOfPokes) {
            switchTo(info().currentIndex[Myself]);
	} else {
	    mystack->setCurrentIndex(ZoneOfPokes);
            myattack->setText("&Go back");
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
        if (zone == info().currentIndex[Myself]) {
            switchTo(info().currentIndex[Myself]);
	} else {
	    /* DO MESSAGE */
	    sendChoice(BattleChoice(true, zone));
	}
    }
}

void BattleWindow::attackButton()
{
    if (mystack->currentIndex() == ZoneOfPokes) {
        switchToPokeZone();
        return;
    }
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

void BattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot, int truespot)
{
    if (spot < 2) {
        if (conf().ids[spot] == idme()) {
            spot = Myself;
        } else {
            spot = Opponent;
        }
    }

    switch (command)
    {
    case SendOut:
	{
            info().sub[spot] = false;
            info().specialSprite[spot] = 0;
            bool silent;

            in >> silent;
            in >> info().currentIndex[spot];

            if (spot == Myself) {
                switchTo(info().currentIndex[Myself], true);
	    } else {
                in >> info().currentShallow(spot);
                info().pokeAlive[Opponent] = true;
                mydisplay->updatePoke(spot);
	    }

            if (!silent)
                printLine(tr("%1 sent out %2!").arg(name(spot), rnick(spot)));

	    break;
	}

    case ChangePP:
	{
	    quint8 move, PP;
	    in  >> move >> PP;

	    //Think to check for crash if currentIndex != -1, move > 3
	    info().currentPoke().move(move).PP() = PP;
            info().tempPoke().move(move).PP() = PP;
            myazones[info().currentIndex[Myself]]->attacks[move]->updateAttack(info().tempPoke().move(move));
	}
    case OfferChoice:
	{
	    info().possible = true;
	    in >> info().choices;
            mycancel->setDisabled(true);
	    updateChoices();
	    break;
        }
    case Ko:
        if (spot==Myself) {
            mypzone->pokes[info().currentIndex[spot]]->setEnabled(false); //crash!!
        }
        BaseBattleWindow::dealWithCommandInfo(in, command, spot, truespot);
        break;

    case StraightDamage :
	{
        qint16 damage;
        in >> damage;
        if (spot == Myself) {
            printLine(tr("%1 lost %2 HP! (%3% of its health)").arg(nick(spot)).arg(damage).arg(damage*100/info().currentPoke().totalLifePoints()));
        } else {
            printLine(tu(tr("%1 lost %2% of its health!").arg(nick(spot)).arg(damage)));
        }
        break;
    }

    case AbsStatusChange:
        {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        mydisplay->changeStatus(spot,poke,status);

        if (spot == Myself) {
            info().myteam.poke(poke).status() = status;
            mypzone->pokes[poke]->update();
        }

        info().currentShallow(spot).status() = status;
        if (poke == info().currentIndex[spot])
            mydisplay->updatePoke(spot);

        break;
    }

    case BattleEnd:
        {
            myclose->setText(tr("&Close"));
            BaseBattleWindow::dealWithCommandInfo(in, command,spot, truespot);
            break;
        }

    case CancelMove:
        {
            info().possible = true;
            mycancel->setDisabled(true);
            updateChoices();
            break;
        }

    case DynamicStats:
        {
            in >> info().mystats;
            mydisplay->updateToolTip(Myself);
            break;
        }
    case TempPokeChange:
        {
            quint8 type;
            in >> type;
            if (type == TempMove) {
                quint8 slot;
                quint16 move;
                in >> slot >> move;
                info().tempPoke().move(slot).num() = move;
                info().tempPoke().move(slot).load();
                myazones[info().currentIndex[Myself]]->attacks[slot]->updateAttack(info().tempPoke().move(slot));
            } else {
                if (type == TempSprite) {
                    in >> info().specialSprite[spot];
                    mydisplay->updatePoke(spot);
                } else {
                    if (type == DefiniteForm) {
                        quint8 poke;
                        quint16 newform;
                        in >> poke >> newform;
                        if (spot == Myself) {
                            info().myteam.poke(poke).num() = newform;
                        }
                        info().pokemons[spot][poke].num() = newform;
                        if (poke == info().currentIndex[spot]) {
                            info().currentShallow(spot).num() = newform;
                        }
                    }
                }
            }
            break;
        }
    default:
        BaseBattleWindow::dealWithCommandInfo(in, command, spot, truespot);
        break;
    }
}

void BattleWindow::animateHPBar()
{
    if (animatedHpSpot() != Myself) {
        BaseBattleWindow::animateHPBar();
        return;
    }

    const int goal = animatedHpGoal();

    //To stop the commands from being processed
    delay();

    int life = info().currentPoke().lifePoints();
    /* We deal with true HP. 30 msec per 3 hp */
    if (goal == life) {
        delay(200);
        return;
    }

    int newHp = goal < life ? std::max(goal, life - 3) : std::min(goal, life+3);
    info().currentPoke().lifePoints() = newHp;
    info().tempPoke().lifePoints() = newHp;
    info().currentShallow(Myself).lifePercent() = info().tempPoke().lifePercent();
    mypzone->pokes[info().currentIndex[Myself]]->update();

    //Recursive call to update the hp bar 30msecs later
    QTimer::singleShot(30, this, SLOT(animateHPBar()));

    mydisplay->updatePoke(Myself);
}

void BattleWindow::switchToNaught(int spot)
{
    if (spot == Myself) {
	switchToPokeZone();
    }

    info().pokeAlive[spot] = false;
    mydisplay->updatePoke(spot);
}

void BattleWindow::updateChoices()
{
    if (info().choices.attacksAllowed == false && info().choices.switchAllowed == true)
        mystack->setCurrentIndex(ZoneOfPokes);

    /* moves first */
    if (info().pokeAlive[Myself])
    {
	if (info().choices.attacksAllowed == false) {
	    myattack->setEnabled(false);
	    for (int i = 0; i < 4; i ++) {
                myazones[info().currentIndex[Myself]]->attacks[i]->setEnabled(false);
	    }
	} else {
	    myattack->setEnabled(true);
	    for (int i = 0; i < 4; i ++) {
                myazones[info().currentIndex[Myself]]->attacks[i]->setEnabled(info().choices.attackAllowed[i]);
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

    QString ttext = tr("%1\n\nPower: %2\nAccuracy: %3\n\nDescription: %4\n\nEffect: %5").arg(MoveInfo::Name(b.num()), MoveInfo::PowerS(b.num()),
                                                                        MoveInfo::AccS(b.num()), MoveInfo::Description(b.num()),
                                                                        MoveInfo::DetailedDescription(b.num()));
    setToolTip(ttext);
}

PokeZone::PokeZone(const TeamBattle &team)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 6; i++)
    {
        l->addWidget(pokes[i] = new PokeButton(team.poke(i)), i >= 3, i % 3);

	mymapper->setMapping(pokes[i], i);
	connect(pokes[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(switchTo(int)));
}


PokeButton::PokeButton(const PokeBattle &p)
    : p(&p)
{
    setIcon(PokemonInfo::Icon(p.num()));
    update();

    QString tooltip = tr("%1 lv %2\n\nItem:%3\nAbility:%4\n\nMoves:\n--%5\n--%6\n--%7\n--%8")
                      .arg(PokemonInfo::Name(p.num()), QString::number(p.level()), ItemInfo::Name(p.item()),
                      AbilityInfo::Name(p.ability()), MoveInfo::Name(p.move(0).num()), MoveInfo::Name(p.move(1).num()),
                      MoveInfo::Name(p.move(2).num()), MoveInfo::Name(p.move(3).num()));
    setToolTip(tooltip);
}

void PokeButton::update()
{
    setText(p->nick() + "\n" + QString::number(p->lifePoints()) + "/" + QString::number(p->totalLifePoints()));
    int status = p->status();
    if (status == Pokemon::Koed || status == Pokemon::Fine) {
        setStyleSheet("");
    } else {
        setStyleSheet("background: " + StatInfo::StatusColor(status).name() + ";");
    }
}


BattleDisplay::BattleDisplay(BattleInfo &i)
    : BaseBattleDisplay(i)
{
    percentageMode = false;
    bars[Myself]->setRange(0,100);
    bars[Myself]->setFormat("%v / %m");

    for (int i = 0; i < 6; i++) {
        mypokeballs[i]->setToolTip(info().myteam.poke(i).nick());
    }

    trainers[Myself]->setText("");
    trainers[Opponent]->setText("");

    updatePoke(Myself);

    connect(bars[Myself], SIGNAL(clicked()), SLOT(changeBarMode()));
}

void BattleDisplay::updateHp(int spot)
{
    if (spot == Opponent || percentageMode)
        BaseBattleDisplay::updateHp(spot);
    else {
        bars[Myself]->setRange(0, mypoke().totalLifePoints());
        bars[Myself]->setValue(mypoke().lifePoints());
    }
}

void BattleDisplay::changeBarMode()
{
    bars[Myself]->setFormat(percentageMode ? "%v / %m" : "%p%");
    percentageMode = !percentageMode;

    if (percentageMode)
        bars[Myself]->setRange(0,100);

    updateHp(Myself);
}

void BattleDisplay::updateToolTip(int spot)
{
    if (spot == Opponent) {
        BaseBattleDisplay::updateToolTip(spot);
        return;
    }

    QString tooltip;

    QString stats[7] = {
        tu(StatInfo::Stat(1)),
        tu(StatInfo::Stat(2)),
        tu(StatInfo::Stat(3)),
        tu(StatInfo::Stat(4)),
        tu(StatInfo::Stat(5)),
        tu(StatInfo::Stat(6)),
        tu(StatInfo::Stat(7))
    };
    int max = 0;
    for (int i = 0; i < 7; i++) {
        max = std::max(max, stats[i].length());
    }
    for (int i = 0; i < 7; i++) {
        stats[i] = stats[i].leftJustified(max, '.', false);
    }

    tooltip += info().currentPoke().nick() + "\n";

    for (int i = 0; i < 5; i++) {
        tooltip += "\n" + stats[i] + ": " + QString::number(info().mystats.stats[i]);
        int boost = info().statChanges[spot].boosts[i];
        if (boost > 0) {
            tooltip += QString("(+%1)").arg(boost);
        } else if (boost < 0) {
            tooltip += QString("(%1)").arg(boost);
        }
    }
    for (int i = 5; i < 7; i++) {
        int boost = info().statChanges[spot].boosts[i];

        if (boost != 0) {
            tooltip += "\n" + stats[i] + " ";

            if (boost > 0) {
                tooltip += QString("+%1").arg(boost);
            } else {
                tooltip += QString("%1").arg(boost);
            }
        }
    }

    tooltip += "\n";

    int flags = info().statChanges[spot].flags;

    int spikes[3] = {BattleDynamicInfo::Spikes, BattleDynamicInfo::SpikesLV2 ,BattleDynamicInfo::SpikesLV3};
    for (int i = 0; i < 3; i++) {
        if (flags & spikes[i]) {
            tooltip += "\n" + tr("Spikes level %1").arg(i+1);
            break;
        }
    }

    int tspikes[2] = {BattleDynamicInfo::ToxicSpikes, BattleDynamicInfo::ToxicSpikesLV2};
    for (int i = 0; i < 2; i++) {
        if (flags & tspikes[i]) {
            tooltip += "\n" + tr("Toxic Spikes level %1").arg(i+1);
            break;
        }
    }

    if (flags & BattleDynamicInfo::StealthRock) {
        tooltip += "\n" + tr("Stealth Rock");
    }

    zone->tooltips[spot] = tooltip;
}
