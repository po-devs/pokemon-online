#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include <iostream>
#include "../Utilities/otherwidgets.h"
#include "basebattlewindow.h"
#include "client.h"
#include "theme.h"

BattleInfo::BattleInfo(const TeamBattle &team, const PlayerInfo &me, const PlayerInfo &opp, bool doubles, int my, int op)
    : BaseBattleInfo(me, opp, doubles, my, op)
{
    possible = false;
    myteam = team;
    sent = true;

    currentSlot = slot(myself);

    for (int i = 0; i < numberOfSlots/2; i++) {
        choices.push_back(BattleChoices());
        choice.push_back(BattleChoice());
        available.push_back(false);
        done.push_back(false);

        mystats.push_back(BattleStats());
        m_tempPoke.push_back(PokeBattle());
    }

    for (int i = 0; i < 6; i++) {
        pokemons[myself][i] = team.poke(i);
    }

    memset(lastMove, 0, sizeof(lastMove));
}

PokeBattle &BattleInfo::tempPoke(int spot)
{
    return m_tempPoke[number(spot)];
}

const PokeBattle & BattleInfo::currentPoke(int spot) const
{
    return myteam.poke(currentIndex[spot]);
}

PokeBattle & BattleInfo::currentPoke(int spot)
{
    return myteam.poke(currentIndex[spot]);
}

BattleWindow::BattleWindow(int battleId, const PlayerInfo &me, const PlayerInfo &opponent, const TeamBattle &team, const BattleConfiguration &_conf)
{
    this->battleId() = battleId;
    this->started() = false;

    conf() = _conf;

    myInfo = new BattleInfo(team, me, opponent, conf().doubles, conf().spot(me.id), conf().spot(opponent.id));
    info().gen = conf().gen;

    mydisplay = new BattleDisplay(info());
    BaseBattleWindow::init();

    QSettings s;
    saveLogs->setChecked(s.value("save_battle_logs").toBool());

    setWindowTitle(tr("Battling against %1").arg(name(info().opponent)));

    myclose->setText(tr("&Forfeit"));
    mylayout->addWidget(mytab = new QTabWidget(), 2, 0, 1, 3);
    mylayout->addWidget(mycancel = new QPushButton(tr("&Cancel")), 3,0);
    mylayout->addWidget(myattack = new QPushButton(tr("&Attack")), 3, 1);
    mylayout->addWidget(myswitch = new QPushButton(tr("&Switch Pokemon")), 3, 2);
    mytab->setObjectName("Modified");

    mytab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mytab->addTab(mystack = new QStackedWidget(), tr("&Moves"));
    mytab->addTab(mypzone = new PokeZone(info().myteam), tr("&Pokemon"));
    mytab->addTab(myspecs = new QListWidget(), tr("Spectators"));
    myspecs->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    for (int i = 0; i < 6; i++) {
        myazones[i] = new AttackZone(team.poke(i), gen());
	mystack->addWidget(myazones[i]);
        mybgroups.append(new QButtonGroup());
        for (int j = 0; j < 4; j ++) {
            myazones[i]->attacks[j]->setCheckable(true);
            mybgroups.value(i)->addButton(myazones[i]->attacks[j],j);
        }
	connect(myazones[i], SIGNAL(clicked(int)), SLOT(attackClicked(int)));
    }

    if (info().doubles) {
        mystack->addWidget(tarZone = new TargetSelection(info()));
        connect(tarZone, SIGNAL(targetSelected(int)), SLOT(targetChosen(int)));
    } else {
        mystack->addWidget(new QWidget());
    }

    mystack->addWidget(szone = new StruggleZone());
    connect(szone, SIGNAL(attackClicked()), SLOT(attackButton()));

    connect(mypzone, SIGNAL(switchTo(int)), SLOT(switchClicked(int)));
    connect(myattack, SIGNAL(clicked()), SLOT(attackButton()));
    connect(myswitch, SIGNAL(clicked()), SLOT(switchToPokeZone()));
    connect(mycancel, SIGNAL(clicked()), SLOT(emitCancel()));
    connect(mytab, SIGNAL(currentChanged(int)), SLOT(changeAttackText(int)));

    switchTo(0,info().slot(info().myself,0), false);

    show();
    printHtml(toBoldColor(tr("Battle between %1 and %2 started!"), Qt::blue).arg(name(1), name(0)));

    disableAll();
}

void BattleWindow::changeAttackText(int i)
{
    if (i == MoveTab)
        myattack->setText(tr("&Attack"));
    else
        myattack->setText(tr("&Go Back"));
}

QString BattleWindow::nick(int spot) const
{
    if (player(spot) == info().myself)
        return info().currentPoke(spot).nick();
    else
        return tr("the foe's %1").arg(info().currentShallow(spot).nick());
}


void BattleWindow::closeEvent(QCloseEvent *)
{
    checkAndSaveLog();
    emit forfeit(battleId());
    close();
}

void BattleWindow::cancel()
{
    info().possible = true;

    for (int i = 0; i < info().available.size(); i++) {
        if (info().available[i]) {
            info().done[i] = false;
        }
    }

    goToNextChoice();
}

void BattleWindow::emitCancel()
{
    /* In doubles, you might want to cancel your current selection and everything
       without sending the server a notice. That's why we send the server a notice only when
       choices were sent, i.e. info().possible is false */
    if (info().possible) {
        cancel();
    } else {
        emit battleCommand(battleId(), BattleChoice(false, BattleChoice::Cancel, ownSlot()));
    }
}

void BattleWindow::switchTo(int pokezone, int spot, bool forced)
{
    if (info().currentIndex[spot] != pokezone || forced) {
        info().currentIndex[spot] = pokezone;
        info().currentShallow(spot) = info().myteam.poke(pokezone);
        info().pokeAlive[spot] = info().currentShallow(spot).status() != Pokemon::Koed;
        info().tempPoke(spot) = info().currentPoke(spot);
    }

    mystack->setCurrentIndex(pokezone);
    mytab->setCurrentIndex(MoveTab);

    mydisplay->updatePoke(spot);

    for (int i = 0; i<4; i++) {
        myazones[info().currentIndex[spot]]->tattacks[i]->updateAttack(info().tempPoke(spot).move(i), info().tempPoke(spot), gen());
    }
}

void BattleWindow::targetChosen(int i)
{
    int n = info().number(info().currentSlot);

    info().choice[n].targetPoke = i;
    info().done[n] = true;

    goToNextChoice();
}

void BattleWindow::clickClose()
{
    if (battleEnded) {
        emit forfeit(battleId());
        return;
    }

    if (QMessageBox::question(this, tr("Losing your battle"), tr("Do you mean to forfeit?"), QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes)
        emit forfeit(battleId());
}

void BattleWindow::switchToPokeZone()
{
    int slot = info().currentSlot;

    if (info().currentIndex[slot] < 0 || info().currentIndex[slot] > 5) {
        mytab->setCurrentIndex(PokeTab);
    }
    else {
	// Go back to the attack zone if the window is on the switch zone
        if (mytab->currentIndex() == PokeTab) {
            mytab->setCurrentIndex(MoveTab);
	} else {
            mytab->setCurrentIndex(PokeTab);
	}
    }
}

int BattleWindow::ownSlot() const {
    return conf().slot(conf().spot(idme()));
}

void BattleWindow::attackClicked(int zone)
{
    int slot = info().currentSlot;

    if (zone != -1) //struggle
        info().lastMove[info().currentIndex[slot]] = zone;
    if (info().possible) {
        info().choice[info().number(slot)] = BattleChoice(false, zone, slot);
        info().choice[info().number(slot)].targetPoke = info().slot(info().opponent);
        if (!info().doubles) {
            info().done[info().number(slot)] = true;
            goToNextChoice();
        } else {
            int move = zone == -1 ? int(Move::Struggle) : info().tempPoke(slot).move(zone);
            int target = MoveInfo::Target(move, gen());
            if (target == Move::ChosenTarget || target == Move::PartnerOrUser) {
                tarZone->updateData(info(), move, gen());
                mystack->setCurrentIndex(TargetTab);
            } else {
                info().done[info().number(slot)] = true;
                goToNextChoice();
            }
        }
    }
}

void BattleWindow::switchClicked(int zone)
{
    int slot = info().currentSlot;

    if (!info().possible)
    {
	switchToPokeZone();
    } else {
        if (!info().choices[info().number(slot)].switchAllowed)
            return;
        if (zone == info().currentIndex[slot]) {
            switchTo(info().currentIndex[slot], slot, false);
	} else {
            info().choice[info().number(slot)] = BattleChoice(true, zone, slot);
            info().done[info().number(slot)] = true;
            goToNextChoice();
	}
    }
}

void BattleWindow::goToNextChoice()
{
    for (int i =0; i < info().available.size(); i++)  {
        int slot = info().slot(info().myself, i);
        int n = i;

        if (info().available[n] && !info().done[n]) {
            enableAll();

            info().currentSlot = slot;

            if (info().choices[n].attacksAllowed == false && info().choices[n].switchAllowed == true)
                mytab->setCurrentIndex(PokeTab);
            else {
                switchTo(info().currentIndex[slot], slot, false);
            }

            /* moves first */
            if (info().pokeAlive[slot])
            {
                if (info().choices[n].attacksAllowed == false) {
                    myattack->setEnabled(false);
                    for (int i = 0; i < 4; i ++) {
                        myazones[info().currentIndex[slot]]->attacks[i]->setEnabled(false);
                    }
                } else {
                    myattack->setEnabled(true);
                    for (int i = 0; i < 4; i ++) {
                        myazones[info().currentIndex[slot]]->attacks[i]->setEnabled(info().choices[n].attackAllowed[i]);
                    }

                    if (info().choices[n].struggle()) {
                        mystack->setCurrentWidget(szone);
                    } else {
                        mystack->setCurrentWidget(myazones[info().currentIndex[slot]]);
                    }
                }
            }
            /* Then pokemon */
            if (info().choices[n].switchAllowed == false) {
                myswitch->setEnabled(false);
                mypzone->setEnabled(false);
            } else {
                myswitch->setEnabled(true);
                for (int i = 0; i < 6; i++) {
                    mypzone->pokes[i]->setEnabled(team().poke(i).num() != 0 && team().poke(i).lifePoints() > 0);
                }

                /* In doubles, whatever happens, you can't switch to your partner */
                if (info().doubles) {
                    int rev = !n;
                    int oslot = info().slot(info().myself, rev);
                    if (info().currentIndex[oslot] >= 0 && info().currentIndex[oslot] < 5) {
                        mypzone->pokes[info().currentIndex[oslot]]->setEnabled(false);
                    }

                    /* Also, you can't switch to a pokemon you've chosen before */
                    for (int i = 0; i < info().available.size(); i++) {
                        if (info().available[i] && info().done[i] && info().choice[i].poke()) {
                            mypzone->pokes[info().choice[i].numSwitch]->setEnabled(false);
                        }
                    }
                }
            }

            return;
        }
    }

    myattack->setEnabled(false);
    myswitch->setEnabled(false);

    disableAll();

    for (int i =0; i < info().available.size(); i++)  {
        if (info().available[i]) {
            sendChoice(info().choice[i]);
        }
    }
}

void BattleWindow::disableAll()
{
    mypzone->setEnabled(false);
    for (int i = 0; i < 6; i++)
        myazones[i]->setEnabled(false);
    if (info().doubles)
        tarZone->setEnabled(false);
}

void BattleWindow::enableAll()
{
    mypzone->setEnabled(true);
    for (int i = 0; i < 6; i++)
        myazones[i]->setEnabled(true);
    if (info().doubles)
        tarZone->setEnabled(true);
}

void BattleWindow::attackButton()
{
    if (mytab->currentIndex() == PokeTab) {
        switchToPokeZone();
        return;
    }

    int slot = info().currentSlot;
    int n = info().number(slot);

    if (info().possible) {
        if (mystack->currentIndex() == TargetTab) {
            /* Doubles, move selection */
            if (info().choices[n].struggle() || MoveInfo::Target(info().lastMove[info().currentIndex[slot]], gen()) == Move::ChosenTarget) {
                return; //We have to wait for the guy to choose a target
            }
            info().done[n] = true;
            goToNextChoice();
        } else {
            //We go with the last move, struggle, or the first possible move
            if (info().choices[n].struggle()) {
                /* Struggle! */
                if (info().doubles) {
                    attackClicked(-1);
                } else {
                    info().choice[n] = BattleChoice(false, -1, slot, info().slot(info().opponent));
                    info().done[n] = true;
                    goToNextChoice();
                }
            } else {
                if (info().choices[n].attackAllowed[info().lastMove[info().currentIndex[slot]]]) {
                    attackClicked(info().lastMove[info().currentIndex[slot]]);
                }
                else
                    for (int i = 0; i < 4; i++) {
                        if (info().choices[n].attackAllowed[i]) {
                            attackClicked(i);
                            break;
                        }
                    }
            }
        }
    }
}

void BattleWindow::sendChoice(const BattleChoice &b)
{
    emit battleCommand(battleId(), b);
    info().possible = false;
}

void BattleWindow::sendMessage()
{
    QString message = myline->text();

    if (message.size() != 0) {
        QStringList s = message.split('\n');
        foreach(QString s1, s) {
            if (s1.length() > 0) {
                emit battleMessage(battleId(), s1);
            }
        }
	myline->clear();
    }
}

void BattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot, int truespot)
{
    int player = info().player(spot);
    switch (command)
    {
    case SendOut:
	{
            info().sub[spot] = false;
            info().specialSprite[spot] = Pokemon::NoPoke;
            bool silent;

            in >> silent;
            in >> info().currentIndex[spot];

            if (player == info().myself) {
                switchTo(info().currentIndex[spot], spot, true);
	    } else {
                in >> info().currentShallow(spot);
                info().pokeAlive[spot] = true;
                mydisplay->updatePoke(spot);
            }

            //Plays the battle cry when a pokemon is switched in
            if (musicPlayed())
            {
                playCry(info().currentShallow(spot).num().pokenum);
            }

            if (!silent) {
                QString pokename = PokemonInfo::Name(info().currentShallow(spot).num());
                if (pokename != rnick(spot))
                    printLine(tr("%1 sent out %2! (%3)").arg(name(info().player(spot)), rnick(spot), pokename));
                else
                    printLine(tr("%1 sent out %2!").arg(name(info().player(spot)), rnick(spot)));
            }

	    break;
	}
    case ChangeHp:
        {
            quint16 newHp;
            in >> newHp;

            animatedHpSpot() = spot;
            animatedHpGoal() = newHp;
            animateHPBar();
            break;
        }

    case ChangePP:
	{
	    quint8 move, PP;
	    in  >> move >> PP;

	    //Think to check for crash if currentIndex != -1, move > 3
            info().currentPoke(spot).move(move).PP() = PP;
            info().tempPoke(spot).move(move).PP() = PP;
            myazones[info().currentIndex[spot]]->tattacks[move]->updateAttack(info().tempPoke(spot).move(move), info().tempPoke(spot), gen());
            mypzone->pokes[info().currentIndex[spot]]->updateToolTip();

            break;
	}
    case OfferChoice:
	{
            if (info().sent) {

                info().sent = false;
                for (int i = 0; i < info().available.size(); i++) {
                    info().available[i] = false;
                    info().done[i] = false;
                }
            }

            BattleChoices c;
            in >> c;
            info().choices[c.numSlot/2] = c;
            info().available[c.numSlot/2] = true;

	    break;
        }
    case MakeYourChoice:
        {
            info().possible = true;
            info().sent = true;

            goToNextChoice();

            break;
        }
    case Ko:
        {
           if (player==info().myself) {
                mypzone->pokes[info().currentIndex[spot]]->setEnabled(false); //crash!!
            }
            BaseBattleWindow::dealWithCommandInfo(in, command, spot, truespot);
            break;
        }

    case StraightDamage :
	{
        qint16 damage;
        in >> damage;
        if (player == info().myself) {
            printLine(tr("%1 lost %2 HP! (%3% of its health)").arg(nick(spot)).arg(damage).arg(damage*100/info().currentPoke(spot).totalLifePoints()));
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

        if (player == info().myself) {
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
            cancel();
            break;
        }

    case DynamicStats:
        {
            in >> info().mystats[info().number(spot)];
            mydisplay->updateToolTip(spot);
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
                info().tempPoke(spot).move(slot).num() = move;
                info().tempPoke(spot).move(slot).load(gen());
                myazones[info().currentIndex[spot]]->tattacks[slot]->updateAttack(info().tempPoke(spot).move(slot), info().tempPoke(spot), gen());
            } else {
                if (type == TempSprite) {
                    Pokemon::uniqueId old = info().specialSprite[spot];
                    in >> info().specialSprite[spot];
                    if (info().specialSprite[spot] == -1) {
                        info().lastSeenSpecialSprite[spot] = old;
                    } else if (info().specialSprite[spot] == Pokemon::NoPoke) {
                        info().specialSprite[spot] = info().lastSeenSpecialSprite[spot];
                    }
                    mydisplay->updatePoke(spot);
                } else if (type == DefiniteForme) {
                    quint8 poke;
                    quint16 newform;
                    in >> poke >> newform;
                    if (spot == info().myself) {
                        info().myteam.poke(poke).num() = newform;
                    }
                    info().pokemons[spot][poke].num() = newform;
                    if (poke == info().currentIndex[spot]) {
                        info().currentShallow(spot).num() = newform;
                    }
                } else if (type == AestheticForme)
                {
                    quint16 newforme;
                    in >> newforme;
                    info().currentShallow(spot).num().subnum = newforme;
                    mydisplay->updatePoke(spot);
                }
            }
            break;
        }
    case PointEstimate:
        {
            qint8 first, second;
            in >> first >> second;

            printHtml(toBoldColor(tr("Variation: "), Qt::blue) + QString("+%1, %2").arg(int(first)).arg(int(second)));
            break;
        }
    default:
        BaseBattleWindow::dealWithCommandInfo(in, command, spot, truespot);
        break;
    }
}

void BattleWindow::addSpectator(bool add, int id)
{
    BaseBattleWindow::addSpectator(add,id);
    if (add) {
        myspecs->addItem(new QIdListWidgetItem(id, client()->name(id)));
    } else {
        for (int i =0 ; i < myspecs->count(); i++) {
            if ( ((QIdListWidgetItem*)(myspecs->item(i)))->id() == id) {
                delete myspecs->takeItem(i);
                return;
            }
        }
    }
}

void BattleWindow::animateHPBar()
{
    int spot = animatedHpSpot();

    if (info().player(spot) != info().myself) {
        BaseBattleWindow::animateHPBar();
        return;
    }

    const int goal = animatedHpGoal();

    QSettings s;
    if (!s.value("animate_hp_bar").toBool()) {
        info().currentPoke(spot).lifePoints() = goal;
        info().tempPoke(spot).lifePoints() = goal;
        info().currentShallow(spot).lifePercent() = info().tempPoke(spot).lifePercent();
        mypzone->pokes[info().currentIndex[spot]]->update();
        mydisplay->updatePoke(spot);
        undelay();
        return;
    }


    int life = info().currentPoke(spot).lifePoints();
    /* We deal with true HP. 30 msec per 3 hp */
    if (goal == life) {
        delay(120,false);
        return;
    }

    //To stop the commands from being processed
    delay(0,false);

    int newHp = goal < life ? std::max(goal, life - 3) : std::min(goal, life+3);
    info().currentPoke(spot).lifePoints() = newHp;
    info().tempPoke(spot).lifePoints() = newHp;
    info().currentShallow(spot).lifePercent() = info().tempPoke(spot).lifePercent();
    mypzone->pokes[info().currentIndex[spot]]->update();

    //Recursive call to update the hp bar 30msecs later
    QTimer::singleShot(30, this, SLOT(animateHPBar()));

    mydisplay->updatePoke(spot);
}

void BattleWindow::switchToNaught(int spot)
{
    if (info().player(spot) == info().myself) {
	switchToPokeZone();
    }

    info().pokeAlive[spot] = false;
    mydisplay->updatePoke(spot);
}

void BattleWindow::updateChoices()
{
    if (info().choices[0].attacksAllowed == false && info().choices[0].switchAllowed == true)
        mytab->setCurrentIndex(PokeTab);

    /* moves first */
    if (info().pokeAlive[info().slot(info().myself, 0)])
    {
        if (info().choices[0].attacksAllowed == false) {
	    myattack->setEnabled(false);
            for (int i = 0; i < 4; i ++) {
                myazones[info().currentIndex[info().player(info().myself)]]->attacks[i]->setEnabled(false);
	    }
	} else {
	    myattack->setEnabled(true);
	    for (int i = 0; i < 4; i ++) {
                myazones[info().currentIndex[info().player(info().myself)]]->attacks[i]->setEnabled(info().choices[0].attackAllowed[i]);
	    }
	}
    }
    /* Then pokemon */
    if (info().choices[0].switchAllowed == false) {
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

AttackZone::AttackZone(const PokeBattle &poke, int gen)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    QSettings s;

    bool old = s.value("old_attack_buttons").toBool();

    if (!old) {
        l->setSpacing(2);
        l->setMargin(5);
    }

    for (int i = 0; i < 4; i++)
    {
        if (old)
            attacks[i] = new OldAttackButton(poke.move(i), poke, gen);
        else
            attacks[i] = new ImageAttackButton(poke.move(i), poke, gen);

        tattacks[i] = dynamic_cast<AbstractAttackButton*>(attacks[i]);

        l->addWidget(attacks[i], i >= 2, i % 2);

	mymapper->setMapping(attacks[i], i);
        connect(dynamic_cast<QAbstractButton*>(attacks[i]), SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(clicked(int)));
}

OldAttackButton::OldAttackButton(const BattleMove &b, const PokeBattle &p, int gen)/* : QImageButton("db/BattleWindow/Buttons/0D.png", "db/BattleWindow/Buttons/0H.png")*/
{
    QVBoxLayout *l = new QVBoxLayout(this);

    l->addWidget(name = new QLabel(), 0, Qt::AlignCenter);
    l->addWidget(pp = new QLabel(), 0, Qt::AlignRight | Qt::AlignVCenter);
    name->setObjectName("AttackName");
    pp->setObjectName("AttackPP");
    //setMinimumWidth(200);

    updateAttack(b,p,gen);
}

void OldAttackButton::updateAttack(const BattleMove &b, const PokeBattle &p, int gen)
{
    name->setText(MoveInfo::Name(b.num()));
    pp->setText(tr("PP %1/%2").arg(b.PP()).arg(b.totalPP()));

    QString power;
    if (b.num() == Move::Return) {
        power = QString("%1").arg(std::max((p.happiness() * 2 / 5),1));
    } else if (b.num() == Move::Frustration) {
        power = QString("%1").arg(std::max(( (255-p.happiness()) * 2 / 5),1));
    } else if (b.num() == Move::HiddenPower) {
        power = QString("%1").arg(HiddenPowerInfo::Power(p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]));
    } else {
        power = MoveInfo::PowerS(b.num(), gen);
    }

    QString ttext = tr("%1\n\nPower: %2\nAccuracy: %3\n\nDescription: %4\n\nEffect: %5").arg(MoveInfo::Name(b.num()), power,
                                                                        MoveInfo::AccS(b.num(), gen), MoveInfo::Description(b.num()),
                                                                        MoveInfo::DetailedDescription(b.num()));

    int type = b.num() == Move::HiddenPower ?
               HiddenPowerInfo::Type(p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]) : MoveInfo::Type(b.num());
    /*QString model = QString("db/BattleWindow/Buttons/%1%2.png").arg(type);
    changePics(model.arg("D"), model.arg("H"), model.arg("C"));*/
    setStyleSheet(QString("background: %1;").arg(Theme::TypeColor(type).name()));

    setToolTip(ttext);
}

ImageAttackButton::ImageAttackButton(const BattleMove &b, const PokeBattle &p, int gen)
    : QImageButton(Theme::path("BattleWindow/Buttons/0D.png"), Theme::path("BattleWindow/Buttons/0H.png"))
{
    QVBoxLayout *l = new QVBoxLayout(this);

    l->addWidget(name = new QLabel(), 0, Qt::AlignCenter);
    l->addWidget(pp = new QLabel(), 0, Qt::AlignRight | Qt::AlignVCenter);
    name->setObjectName("AttackName");
    pp->setObjectName("AttackPP");

    updateAttack(b,p,gen);
}

void ImageAttackButton::updateAttack(const BattleMove &b, const PokeBattle &p, int gen)
{
    name->setText(MoveInfo::Name(b.num()));
    pp->setText(tr("PP %1/%2").arg(b.PP()).arg(b.totalPP()));

    QString power;
    if (b.num() == Move::Return) {
        power = QString("%1").arg(std::max((p.happiness() * 2 / 5),1));
    } else if (b.num() == Move::Frustration) {
        power = QString("%1").arg(std::max(( (255-p.happiness()) * 2 / 5),1));
    } else if (b.num() == Move::HiddenPower) {
        power = QString("%1").arg(HiddenPowerInfo::Power(p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]));
    } else {
        power = MoveInfo::PowerS(b.num(), gen);
    }

    QString ttext = tr("%1\n\nPower: %2\nAccuracy: %3\n\nDescription: %4\n\nEffect: %5").arg(MoveInfo::Name(b.num()), power,
                                                                        MoveInfo::AccS(b.num(), gen), MoveInfo::Description(b.num()),
                                                                        MoveInfo::DetailedDescription(b.num()));

    int type = b.num() == Move::HiddenPower ?
               HiddenPowerInfo::Type(p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]) : MoveInfo::Type(b.num());
    QString model = QString("BattleWindow/Buttons/%1%2.png").arg(type);
    changePics(Theme::path(model.arg("D")), Theme::path(model.arg("H")), Theme::path(model.arg("C")));

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
    setIconSize(QSize(32,32));
    setIcon(PokemonInfo::Icon(p.num()));
    update();

    updateToolTip();
}

void PokeButton::update()
{
    setText(p->nick() + "\n" + QString::number(p->lifePoints()) + "/" + QString::number(p->totalLifePoints()));
    int status = p->status();
    if (status == Pokemon::Koed || status == Pokemon::Fine) {
        setStyleSheet("");
    } else {
        setStyleSheet("background: " + Theme::StatusColor(status).name() + ";");
    }
    
    updateToolTip();
}

void PokeButton::updateToolTip()
{
    const PokeBattle &p = *(this->p);
    QString tooltip = tr("%1 lv %2\n\nItem:%3\nAbility:%4\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                      .arg(PokemonInfo::Name(p.num()), QString::number(p.level()), ItemInfo::Name(p.item()),
                      AbilityInfo::Name(p.ability()), MoveInfo::Name(p.move(0).num()), MoveInfo::Name(p.move(1).num()),
                      MoveInfo::Name(p.move(2).num()), MoveInfo::Name(p.move(3).num())).arg(p.move(0).PP()).arg(p.move(1).PP())
                      .arg(p.move(2).PP()).arg(p.move(3).PP());
    setToolTip(tooltip);
}


BattleDisplay::BattleDisplay(BattleInfo &i)
    : BaseBattleDisplay(i)
{
    for (int i = 0; i < info().numberOfSlots; i++) {
        if (info().player(i) == info().myself) {
            percentageMode.push_back(false);
            bars[i]->setRange(0,100);
            bars[i]->setFormat("%v / %m");
            connect(bars[i], SIGNAL(clicked()), SLOT(changeBarMode()));
        } else {
            percentageMode.push_back(true);
        }
    }


    for (int i = 0; i < 6; i++) {
        mypokeballs[i]->setToolTip(info().myteam.poke(i).nick());
    }

    updatePoke(info().slot(info().myself));
    if (info().doubles) {
        updatePoke(info().slot(info().myself, 1));
    }
}

void BattleDisplay::updateHp(int spot)
{
    if (percentageMode[spot])
        BaseBattleDisplay::updateHp(spot);
    else {
        bars[spot]->setRange(0, mypoke(spot).totalLifePoints());
        bars[spot]->setValue(mypoke(spot).lifePoints());
    }
}

void BattleDisplay::changeBarMode()
{
    int i;
    for (i = 0; i < info().numberOfSlots; i++) {
        if (bars[i] == sender()) {
            break;
        }
    }

    bars[i]->setFormat(percentageMode[i] ? "%v / %m" : "%p%");
    percentageMode[i] = !percentageMode[i];

    if (percentageMode[i])
        bars[i]->setRange(0,100);

    updateHp(i);
}

void BattleDisplay::updateToolTip(int spot)
{
    if (info().player(spot) == info().opponent) {
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

    tooltip += info().currentPoke(spot).nick() + "\n";
    Pokemon::uniqueId num = info().currentPoke(spot).num();
    tooltip += TypeInfo::Name(PokemonInfo::Type1(num, info().gen));
    int type2 = PokemonInfo::Type2(num);
    if (type2 != Pokemon::Curse) {
        tooltip += " " + TypeInfo::Name(PokemonInfo::Type2(num, info().gen));
    }
    tooltip += "\n";

    for (int i = 0; i < 5; i++) {
        tooltip += "\n" + stats[i] + ": " + QString::number(info().mystats[info().number(spot)].stats[i]);
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


/******************************************************************************/
/******************** TARGET TAB **********************************************/
/******************************************************************************/


TargetSelection::TargetSelection(const BattleInfo &info)
{
    QGridLayout *gl = new QGridLayout(this);

    QButtonGroup *bg = new QButtonGroup(this);
    bg->setExclusive(false);

    for (int i = 0; i < 4; i++) {
        bool opp = info.player(i) == info.opponent;

        gl->addWidget(pokes[i] = new QPushButton(), !opp, info.number(i));
        pokes[i]->setCheckable(true);
        pokes[i]->setObjectName("PokemonTargetButton");
        pokes[i]->setIconSize(QSize(32,32));

        bg->addButton(pokes[i], i);
    }

    connect(bg, SIGNAL(buttonClicked(int)), SIGNAL(targetSelected(int)));
}

void TargetSelection::updateData(const BattleInfo &info, int move, int gen)
{
    int slot = info.currentSlot;

    for (int i = 0; i < 4; i++) {
        pokes[i]->setText(info.currentShallow(slot).status() == Pokemon::Koed ? "" : info.currentShallow(i).nick());
        pokes[i]->setIcon(PokemonInfo::Icon(info.currentShallow(i).num()));
        pokes[i]->setDisabled(true);
        pokes[i]->setChecked(false);
        pokes[i]->setStyleSheet("");
    }

    switch (Move::Target(MoveInfo::Target(move, gen))) {
    case Move::All:
        for (int i = 0; i < 4; i++) {
            if (info.currentShallow(i).status() != Pokemon::Koed) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::AllButSelf:
        for (int i = 0; i < 4; i++) {
            if (info.currentShallow(i).status() != Pokemon::Koed && i != slot) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::Opponents:
        for (int i = 0; i < 4; i++) {
            if (info.currentShallow(i).status() != Pokemon::Koed && info.player(i) == info.opponent) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::ChosenTarget:
        for (int i = 0; i < 4; i++) {
            if (info.currentShallow(i).status() != Pokemon::Koed && i != slot)
                pokes[i]->setEnabled(true);
        }
        break;
    case Move::PartnerOrUser:
        for (int i = 0; i < 4; i++) {
            if (info.currentShallow(i).status() != Pokemon::Koed && info.player(i) == info.player(slot)) {
                pokes[i]->setEnabled(true);
            }
        }
        break;
    case Move::User:
    case Move::RandomTarget:
    case Move::None:
        pokes[slot]->setEnabled(true);
        pokes[slot]->setStyleSheet("background: #07a7c9; color: white;");
    default:
        return;
    }
}

StruggleZone::StruggleZone()
{
    QHBoxLayout *l = new QHBoxLayout(this);

    QImageButton *b = Theme::Button("attack");
    l->addWidget(b, 0, Qt::AlignCenter);

    b->setCheckable(true);

    connect(b, SIGNAL(clicked()), this, SIGNAL(attackClicked()));
}
