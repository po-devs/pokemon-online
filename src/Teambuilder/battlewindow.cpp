#include "battlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "basebattlewindow.h"
#include "logmanager.h"
#include "client.h"
#include "theme.h"
#include <cstdlib>

BattleInfo::BattleInfo(const TeamBattle &team, const PlayerInfo &me, const PlayerInfo &opp, int mode, int my, int op)
    : BaseBattleInfo(me, opp, mode, my, op)
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

bool BattleInfo::areAdjacent(int poke1, int poke2) const
{
    return std::abs(slotNum(poke1)-slotNum(poke2)) <= 1;
}

PokeBattle &BattleInfo::tempPoke(int spot)
{
    return m_tempPoke[number(spot)];
}

const PokeBattle & BattleInfo::currentPoke(int spot) const
{
    return myteam.poke(slotNum(spot));
}

PokeBattle & BattleInfo::currentPoke(int spot)
{
    return myteam.poke(slotNum(spot));
}

BattleWindow::BattleWindow(int battleId, const PlayerInfo &me, const PlayerInfo &opponent, const TeamBattle &team, const BattleConfiguration &_conf,
                           Client *client)
{
    hasLoggedWifiClause = false;
    question = NULL;
    this->battleId() = battleId;
    this->started() = false;
    ownid() = me.id;
    _mclient = client;

    conf() = _conf;

    myInfo = new BattleInfo(team, me, opponent, conf().mode, conf().spot(me.id), conf().spot(opponent.id));
    info().gen = conf().gen;

    mydisplay = new BattleDisplay(info());
    BaseBattleWindow::init();

    QSettings s;
    saveLogs->setChecked(s.value("save_battle_logs").toBool());
    log->override = saveLogs->isChecked() ? Log::OverrideYes : Log::OverrideNo;

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

    for (int i = 0; i < 3; i++) {
        myazones[i] = new AttackZone(team.poke(i), gen());
        mystack->addWidget(myazones[i]);
        mybgroups.append(new QButtonGroup());
        for (int j = 0; j < 4; j ++) {
            myazones[i]->attacks[j]->setCheckable(true);
            mybgroups.value(i)->addButton(myazones[i]->attacks[j],j);
        }
        connect(myazones[i], SIGNAL(clicked(int)), SLOT(attackClicked(int)));
    }

    if (info().multiples()) {
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

    mysend->disconnect(this);
    mysend->setText(tr("Suggest draw"));
    mysend->setCheckable(true);
    connect(mysend, SIGNAL(clicked()), SLOT(offerTie()));

    switchTo(0,info().slot(info().myself,0), false);

    show();

    log->pushHtml("<!DOCTYPE html>");
    log->pushHtml("<!-- Pokemon Online battle log (version 1.0) -->");
    log->pushHtml(QString("<!-- Log belonging to %1-->").arg(info().name(info().myself)));
    log->pushHtml(QString("<head>\n\t<title>%1 vs %2</title>\n</head>").arg(info().name(info().myself), info().name(info().opponent)));
    log->pushHtml("<body>");

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
        return rnick(spot);
    else
        return tr("the foe's %1").arg(rnick(spot));
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
        emit battleCommand(battleId(), BattleChoice(ownSlot(), CancelChoice()));
    }
}

void BattleWindow::switchTo(int pokezone, int spot, bool forced)
{
    int snum = info().slotNum(spot);

    if (snum != pokezone || forced) {
        info().switchPoke(spot, pokezone, true);
        mypzone->pokes[snum]->changePokemon(info().myteam.poke(snum));
        mypzone->pokes[pokezone]->changePokemon(info().myteam.poke(pokezone));
    }

    mystack->setCurrentIndex(info().number(spot));
    mytab->setCurrentIndex(MoveTab);

    mydisplay->updatePoke(spot);

    for (int i = 0; i< 4; i++) {
        myazones[info().number(spot)]->tattacks[i]->updateAttack(info().tempPoke(spot).move(i), info().tempPoke(spot), gen());
    }
}

void BattleWindow::targetChosen(int i)
{
    int n = info().number(info().currentSlot);

    info().choice[n].setTarget(i);
    info().done[n] = true;

    goToNextChoice();
}

void BattleWindow::clickClose()
{
    if (battleEnded) {
        forfeit();
        return;
    }

    if (question != NULL) {
        question->activateWindow();
        return;
    }

    question = new QMessageBox(QMessageBox::Question,tr("Losing your battle"), tr("Do you mean to forfeit?"), QMessageBox::Yes | QMessageBox::No,this);
    question->setAttribute(Qt::WA_DeleteOnClose, true);
    question->show();

    connect(question, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(questionButtonClicked(QAbstractButton*)));
    connect(question, SIGNAL(destroyed()), this, SLOT(nullQuestion()));
}

void BattleWindow::forfeit() {
    emit forfeit(battleId());
}

void BattleWindow::nullQuestion() {
    question = NULL;
}

void BattleWindow::questionButtonClicked(QAbstractButton * b)
{
    QMessageBox::ButtonRole role = question->buttonRole(b);
    if (role == QMessageBox::YesRole) {
        forfeit();
    }
    question->close();
}

void BattleWindow::switchToPokeZone()
{
    int n = info().number(info().currentSlot);
    if (sender() && info().mode == ChallengeInfo::Triples && n != 1) {
        BattleChoice &b = info().choice[n];
        b = BattleChoice(info().currentSlot, MoveToCenterChoice());
        info().done[n] = true;
        goToNextChoice();

        return;
    }
    // Go back to the attack zone if the window is on the switch zone
    if (mytab->currentIndex() == PokeTab) {
        mytab->setCurrentIndex(MoveTab);
    } else {
        mytab->setCurrentIndex(PokeTab);
    }
}

int BattleWindow::ownSlot() const {
    return conf().slot(conf().spot(idme()));
}

void BattleWindow::attackClicked(int zone)
{
    int slot = info().currentSlot;

    if (zone != -1) //struggle
        info().lastMove[info().number(slot)] = zone;
    if (info().possible) {
        BattleChoice &b = info().choice[info().number(slot)];
        b = BattleChoice(slot, AttackChoice());
        b.setAttackSlot(zone);
        b.setTarget(info().slot(info().opponent));

        if (!info().multiples()) {
            info().done[info().number(slot)] = true;
            goToNextChoice();
        } else {
            int move = zone == -1 ? int(Move::Struggle) : info().tempPoke(slot).move(zone);
            int target = MoveInfo::Target(move, gen());
            /* Triples still require to choose the target */
            if (target == Move::ChosenTarget || target == Move::PartnerOrUser || target == Move::Partner || target == Move::MeFirstTarget || target == Move::IndeterminateTarget
                    || info().numberOfSlots > 4) {
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
        if (zone == info().number(slot)) {
            switchTo(info().number(slot), slot, false);
        } else {
            BattleChoice &b = info().choice[info().number(slot)];
            b = BattleChoice(slot, SwitchChoice());
            b.setPokeSlot(zone);
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

            myswitch->setText(tr("&Switch Pokemon"));
            if (info().choices[n].attacksAllowed == false && info().choices[n].switchAllowed == true)
                mytab->setCurrentIndex(PokeTab);
            else {
                switchTo(info().number(slot), slot, false);
                if (info().mode == ChallengeInfo::Triples && i != 1) {
                    myswitch->setText(tr("&Shift to centre"));
                }
            }

            /* moves first */
            if (info().pokeAlive[slot])
            {
                if (info().choices[n].attacksAllowed == false) {
                    myattack->setEnabled(false);
                    for (int i = 0; i < 4; i ++) {
                        myazones[info().number(slot)]->attacks[i]->setEnabled(false);
                    }
                } else {
                    myattack->setEnabled(true);
                    for (int i = 0; i < 4; i ++) {
                        myazones[info().number(slot)]->attacks[i]->setEnabled(info().choices[n].attackAllowed[i]);
                    }

                    if (info().choices[n].struggle()) {
                        mystack->setCurrentWidget(szone);
                    } else {
                        mystack->setCurrentWidget(myazones[info().number(slot)]);
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
                    mypzone->pokes[i]->setEnabled(team().poke(i).num() != 0 && team().poke(i).lifePoints() > 0 && team().poke(i).status() != Pokemon::Koed);
                }

                if (info().multiples()) {
                    /* In doubles, whatever happens, you can't switch to your partner */
                    for (int i = 0; i < info().numberOfSlots/2; i++) {
                        mypzone->pokes[i]->setEnabled(false);
                    }

                    /* Also, you can't switch to a pokemon you've chosen before */
                    for (int i = 0; i < info().available.size(); i++) {
                        if (info().available[i] && info().done[i] && info().choice[i].switchChoice()) {
                            mypzone->pokes[info().choice[i].pokeSlot()]->setEnabled(false);
                        }
                    }
                }
            }

            return;
        }
    }

    myattack->setEnabled(false);
    myswitch->setEnabled(false);
    mysend->setEnabled(false);

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
    for (int i = 0; i < 3; i++)
        myazones[i]->setEnabled(false);
    if (info().multiples())
        tarZone->setEnabled(false);
}

void BattleWindow::enableAll()
{
    mypzone->setEnabled(true);
    for (int i = 0; i < 3; i++)
        myazones[i]->setEnabled(true);
    if (info().multiples())
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
            int mv = info().lastMove[info().number(slot)];
            int tar = MoveInfo::Target(mv,gen());
            if (info().choices[n].struggle() || tar == Move::ChosenTarget || tar == Move::MeFirstTarget
                    || mv == Move::Curse || tar == Move::PartnerOrUser) {
                return; //We have to wait for the guy to choose a target
            }
            info().done[n] = true;
            goToNextChoice();
        } else {
            //We go with the last move, struggle, or the first possible move
            if (info().choices[n].struggle()) {
                /* Struggle! */
                if (info().multiples()) {
                    attackClicked(-1);
                } else {
                    BattleChoice &b = info().choice[n];
                    b = BattleChoice(slot, AttackChoice());
                    b.setAttackSlot(-1);
                    b.setTarget(info().slot(info().opponent));
                    info().done[n] = true;
                    goToNextChoice();
                }
            } else {
                if (info().choices[n].attackAllowed[info().lastMove[info().number(slot)]]) {
                    attackClicked(info().lastMove[info().number(slot)]);
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

    if (!b.drawChoice())
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

void BattleWindow::offerTie()
{
    mysend->setDisabled(true);
    sendChoice(BattleChoice(info().slot(info().myself), DrawChoice()));
}

void BattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot, int truespot)
{
    int player = info().player(spot);
    switch (command)
    {
    case SendOut:
	{
        if (player != info().myself) {
            BaseBattleWindow::dealWithCommandInfo(in, command, spot, truespot);
            break;
        }

        bool silent;
        quint8 prevIndex;
        in >> silent;
        in >> prevIndex;

        info().sub[spot] = false;
        info().specialSprite[spot] = Pokemon::NoPoke;

        switchTo(prevIndex, spot, true);

        if (!in.atEnd())
            in >> info().currentShallow(spot);

        mydisplay->updatePoke(info().player(spot), info().slotNum(spot));
        mydisplay->updatePoke(info().player(spot), prevIndex);

        //Plays the battle cry when a pokemon is switched in
        if (musicPlayed())
        {
            playCry(info().currentShallow(spot).num().pokenum);
        }

        QString pokename = PokemonInfo::Name(info().currentShallow(spot).num());
        if (pokename != rnick(spot))
            printLine(tr("%1 sent out %2! (%3)").arg(name(info().player(spot)), rnick(spot), pokename), silent);
        else
            printLine(tr("%1 sent out %2!").arg(name(info().player(spot)), rnick(spot)), silent);

        printLine(tr("%1's previous position in the team: %2.").arg(nick(spot)).arg(prevIndex), true);
        printLine(tr("%1's life: %2/%3 HP.").arg(nick(spot)).arg(info().currentPoke(spot).lifePoints()).arg(info().currentPoke(spot).totalLifePoints()), true);
        printLine(tr("%1's status: %2.").arg(nick(spot), StatInfo::Status(info().currentPoke(spot).status())), true);
        printLine(tr("%1's level: %2.").arg(nick(spot)).arg(info().currentShallow(spot).level()), true);
        printLine(tr("%1's shininess: %2.").arg(nick(spot)).arg(info().currentShallow(spot).shiny()), true);
        printLine(tr("%1's gender: %2.").arg(nick(spot)).arg(GenderInfo::Name(info().currentShallow(spot).gender())), true);
	    break;
	}
    case ChangeHp:
    {
        if (player != info().myself) {
            BaseBattleWindow::dealWithCommandInfo(in, command, spot, truespot);
            break;
        }

        quint16 newHp;
        in >> newHp;

        printLine(tr("%1's new HP is %2/%3.").arg(nick(spot)).arg(newHp).arg(info().currentPoke(spot).totalLifePoints()), true);

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
        myazones[info().number(spot)]->tattacks[move]->updateAttack(info().tempPoke(spot).move(move), info().tempPoke(spot), gen());
        mypzone->pokes[info().number(spot)]->updateToolTip();

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
        /* Allows to ask for draw again */
        mysend->setEnabled(true);
        mysend->setChecked(false);

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
            mypzone->pokes[info().number(spot)]->setEnabled(false); //crash!!
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

        printLine(tr("Pokemon number %1 of %2 had its status changed to: %3.").arg(poke).arg(name(spot), StatInfo::Status(status)), true);

        mydisplay->changeStatus(spot,poke,status);

        if (player == info().myself) {
            info().myteam.poke(poke).changeStatus(status);
            mypzone->pokes[poke]->update();
        }

        info().pokemons[player][poke].changeStatus(status);
        if (info().isOut(player, poke))
            mydisplay->updatePoke(info().slot(player, poke));

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
        if (type == TempMove || type == DefMove) {
            quint8 slot;
            quint16 move;
            in >> slot >> move;
            info().tempPoke(spot).move(slot).num() = move;
            info().tempPoke(spot).move(slot).load(gen());
            myazones[info().number(spot)]->tattacks[slot]->updateAttack(info().tempPoke(spot).move(slot), info().tempPoke(spot), gen());

            if (type == DefMove) {
                info().myteam.poke(info().number(spot)).move(slot).num() = move;
                info().myteam.poke(info().number(spot)).move(slot).load(gen());
            }
        } else if (type == TempPP){
            quint8 slot;
            quint8 PP;
            in >> slot >> PP;
            info().tempPoke(spot).move(slot).PP() = PP;
            myazones[info().number(spot)]->tattacks[slot]->updateAttack(info().tempPoke(spot).move(slot), info().tempPoke(spot), gen());
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
                if (info().isOut(player, poke)) {
                    info().currentShallow(info().slot(player, spot)).num() = newform;
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
    case RearrangeTeam:
    {
        ShallowShownTeam t;
        in >> t;

        openRearrangeWindow(t);
        break;
    }
    case SpotShifts:
    {
        qint8 s1, s2;
        bool silent;

        in >> s1 >> s2 >> silent;

        if (!silent) {
            if (info().currentShallow(info().slot(spot, s2)).status() == Pokemon::Koed) {
                printLine(tr("%1 shifted spots to the middle!").arg(tu(nick(info().slot(spot, s1)))));
            } else {
                printLine(tr("%1 shifted spots with %2!").arg(tu(nick(info().slot(spot, s1))), nick(info().slot(spot, s2))));
            }
        }

        info().switchOnSide(spot, s1, s2);

        int pk1 = info().slot(spot, s1);
        int pk2 = info().slot(spot, s2);
        mydisplay->updatePoke(pk1);
        mydisplay->updatePoke(pk2);

        mydisplay->updatePoke(info().player(spot), s1);
        mydisplay->updatePoke(info().player(spot), s2);

        mypzone->pokes[s1]->changePokemon(info().myteam.poke(s1));
        mypzone->pokes[s2]->changePokemon(info().myteam.poke(s2));

        delay(500);
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
        mypzone->pokes[info().number(spot)]->update();
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
    mypzone->pokes[info().number(spot)]->update();

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
                myazones[0]->attacks[i]->setEnabled(false);
            }
        } else {
            myattack->setEnabled(true);
            for (int i = 0; i < 4; i ++) {
                myazones[0]->attacks[i]->setEnabled(info().choices[0].attackAllowed[i]);
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

void BattleWindow::openRearrangeWindow(const ShallowShownTeam &t)
{
    if (!hasLoggedWifiClause) {
        hasLoggedWifiClause = true;

        QStringList mynames, oppnames;

        for (int i = 0; i < 6; i++) {
            Pokemon::uniqueId id = info().myteam.poke(i).num();

            if (id != Pokemon::NoPoke) {
                mynames.push_back(PokemonInfo::Name(id));
            }
        }
        for (int i = 0; i < 6; i++) {
            Pokemon::uniqueId id = t.poke(i).num;

            if (id != Pokemon::NoPoke) {
                oppnames.push_back(PokemonInfo::Name(id));
            }
        }

        printLine(toBoldColor(tr("Your team: "), Qt::blue) + mynames.join(" / "));
        printLine(toBoldColor(tr("Opponent's team: "), Qt::blue) + oppnames.join(" / "));
        printLine("");
    }

    RearrangeWindow *r = new RearrangeWindow(info().myteam, t);
    r->setParent(this, Qt::Window | Qt::Dialog);
    r->move(x() + (width()-r->width())/2, y() + (height()-r->height())/2);
    r->show();
    r->move(x() + (width()-r->width())/2, y() + (height()-r->height())/2);

    connect(r, SIGNAL(forfeit()), SLOT(clickClose()));
    connect(r, SIGNAL(done()), SLOT(sendRearrangedTeam()));
    connect(r, SIGNAL(done()), r, SLOT(deleteLater()));
}

void BattleWindow::sendRearrangedTeam()
{
    RearrangeChoice r;

    for (int i = 0; i < 6; i++)
        r.pokeIndexes[i] = info().myteam.internalId(info().myteam.poke(i));

    BattleChoice c = BattleChoice(info().slot(info().myself), r);
    sendChoice(c);

    /* If the team was rearranged... */
    for (int i = 0; i < 6; i++) {
        mypzone->pokes[i]->changePokemon(info().myteam.poke(i));
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
        power = QString("%1").arg(HiddenPowerInfo::Power(gen, p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]));
    } else {
        power = MoveInfo::PowerS(b.num(), gen);
    }

    QString moveCategory;
    moveCategory = CategoryInfo::Name(MoveInfo::Category(b.num(), gen));

    QString ttext = tr("%1\n\nPower: %2\nAccuracy: %3\n\nDescription: %4\n\nCategory: %5\n\nEffect: %6").arg(MoveInfo::Name(b.num()), power,
                                                                                                             MoveInfo::AccS(b.num(), gen), MoveInfo::Description(b.num(), gen),
                                                                                                             moveCategory,
                                                                                                             MoveInfo::DetailedDescription(b.num()));

    int type = b.num() == Move::HiddenPower ?
                HiddenPowerInfo::Type(gen, p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]) : MoveInfo::Type(b.num(), gen);
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
        power = QString("%1").arg(HiddenPowerInfo::Power(gen, p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]));
    } else {
        power = MoveInfo::PowerS(b.num(), gen);
    }

    QString ttext = tr("%1\n\nPower: %2\nAccuracy: %3\n\nDescription: %4\n\nEffect: %5").arg(MoveInfo::Name(b.num()), power,
                                                                                             MoveInfo::AccS(b.num(), gen), MoveInfo::Description(b.num(), gen),
                                                                                             MoveInfo::DetailedDescription(b.num()));

    int type = b.num() == Move::HiddenPower ?
                HiddenPowerInfo::Type(gen, p.dvs()[0], p.dvs()[1],p.dvs()[2],p.dvs()[3],p.dvs()[4],p.dvs()[5]) : MoveInfo::Type(b.num(), gen);
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

void PokeButton::changePokemon(const PokeBattle &p)
{
    this->p = &p;

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
    QString tooltip;
    if (p.ability() != 0) {
        tooltip = tr("%1 lv %2\n\nItem:%3\nAbility:%4\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                .arg(PokemonInfo::Name(p.num()), QString::number(p.level()), ItemInfo::Name(p.item()),
                     AbilityInfo::Name(p.ability()), MoveInfo::Name(p.move(0).num()), MoveInfo::Name(p.move(1).num()),
                     MoveInfo::Name(p.move(2).num()), MoveInfo::Name(p.move(3).num())).arg(p.move(0).PP()).arg(p.move(1).PP())
                .arg(p.move(2).PP()).arg(p.move(3).PP());
    } else if (p.ability() == 0) {
        if (p.item() != 0) {
            tooltip = tr("%1 lv %2\nItem:%3\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                    .arg(PokemonInfo::Name(p.num()), QString::number(p.level()), ItemInfo::Name(p.item()),
                         MoveInfo::Name(p.move(0).num()), MoveInfo::Name(p.move(1).num()),
                         MoveInfo::Name(p.move(2).num()), MoveInfo::Name(p.move(3).num())).arg(p.move(0).PP()).arg(p.move(1).PP())
                    .arg(p.move(2).PP()).arg(p.move(3).PP());
        } else {
            tooltip = tr("%1 lv %2\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                    .arg(PokemonInfo::Name(p.num()), QString::number(p.level()),
                         MoveInfo::Name(p.move(0).num()), MoveInfo::Name(p.move(1).num()),
                         MoveInfo::Name(p.move(2).num()), MoveInfo::Name(p.move(3).num())).arg(p.move(0).PP()).arg(p.move(1).PP())
                    .arg(p.move(2).PP()).arg(p.move(3).PP());
        }
    }
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

    for (int i = 0; i < info().numberOfSlots/2; i++) {
        updatePoke(info().slot(info().myself, i));
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
        tooltip += "\n" + stats[i] + ": ";
        int stat = info().mystats[info().number(spot)].stats[i];
        if (stat == -1) {
            tooltip += "???";
        } else {
            tooltip += QString::number(stat);
        }
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

    for (int i = 0; i < info.numberOfSlots; i++) {
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
    int num = info.numberOfSlots;

    for (int i = 0; i < num; i++) {
        if (info.currentShallow(i).status() == Pokemon::Koed) {
            pokes[i]->setText("");
            pokes[i]->setIcon(QIcon());
        } else {
            pokes[i]->setText(info.currentShallow(i).nick());
            pokes[i]->setIcon(PokemonInfo::Icon(info.currentShallow(i).num()));
        }
        pokes[i]->setDisabled(true);
        pokes[i]->setChecked(false);
        pokes[i]->setStyleSheet("");
    }

    switch (Move::Target(MoveInfo::Target(move, gen))) {
    case Move::All:
        for (int i = 0; i < num; i++) {
            if (info.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::AllButSelf:
        for (int i = 0; i < num; i++) {
            if (i != slot && info.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::Opponents:
        for (int i = 0; i < num; i++) {
            if (info.player(i) == info.opponent && info.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::OpposingTeam:
        for (int i = 0; i < num; i++) {
            if (info.player(i) == info.opponent) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::TeamParty: case Move::TeamSide:
        for (int i = 0; i < num; i++) {
            if (info.player(i) == info.myself) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::IndeterminateTarget:
    case Move::ChosenTarget:
        for (int i = 0; i < num; i++) {
            if (i != slot &&
                    ((MoveInfo::Flags(move, gen) & Move::PulsingFlag) || info.areAdjacent(i, slot)))
                pokes[i]->setEnabled(true);
        }
        break;
    case Move::PartnerOrUser:
        for (int i = 0; i < num; i++) {
            if (info.player(i) == info.myself && info.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
            }
        }
        break;
        /* Me first only allows targetting opponents but not allies */
    case Move::MeFirstTarget:
        for (int i = 0; i < num; i++) {
            if (info.player(i) != info.myself && info.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
            }
        }
        break;
    case Move::Partner:
        for (int i = 0; i < num; i++) {
            if (info.player(i) == info.myself && i != slot  && info.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
            }
        }
        break;
    case Move::User:
    case Move::RandomTarget:
    case Move::Field:
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

/******************************************************************************/
/******************** REARRANGE WINDOW ****************************************/
/******************************************************************************/

RearrangeWindow::RearrangeWindow(TeamBattle &t, const ShallowShownTeam &op)
{
    setAttribute(Qt::WA_DeleteOnClose);

    myteam = &t;

    QVBoxLayout *v = new QVBoxLayout(this);

    v->addWidget(new QLabel(tr("You can rearrange your team by clicking on your pokemon before the battle.")));

    QHBoxLayout *h1 = new QHBoxLayout();
    for (int i = 0; i < 6; i++) {
        QToolButton *p = new QToolButton();
        p->setFixedSize(70,70);
        RearrangeLayout *l = new RearrangeLayout(p, t.poke(i).num(), t.poke(i).level(), t.poke(i).gender(), t.poke(i).item());
        p->setAutoRaise(true);
        if (t.poke(i).num() != Pokemon::NoPoke)
            p->setCheckable(true);
        connect(p, SIGNAL(toggled(bool)), SLOT(runExchanges()));

        h1->addWidget(p, 0, Qt::AlignCenter);
        buttons[i] = p;
        layouts[i] = l;
    }

    v->addLayout(h1);

    v->addWidget(new QLabel(tr("Team of your opponent:")));

    QHBoxLayout *h2 = new QHBoxLayout();
    for (int i = 0; i < 6; i++) {
        QLabel *l = new QLabel();
        l->setFixedSize(70,70);
        new RearrangeLayout(l, op.poke(i).num, op.poke(i).level, op.poke(i).gender, op.poke(i).item);

        h2->addWidget(l, 0, Qt::AlignCenter);
    }
    v->addLayout(h2);

    QPushButton *doneButton, *forfeitButton;
    QHBoxLayout *h3 = new QHBoxLayout();
    h3->addStretch();
    h3->addWidget(doneButton = new QPushButton(tr("Done")));
    h3->addWidget(forfeitButton = new QPushButton(tr("Forfeit")));
    h3->addStretch();
    v->addLayout(h3);

    connect(doneButton, SIGNAL(clicked()), SIGNAL(done()));
    connect(forfeitButton, SIGNAL(clicked()), SIGNAL(forfeit()));

    show();
}

void RearrangeWindow::runExchanges()
{
    int check1 = -1;
    for (int i = 0; i < 6; i++) {
        if (buttons[i]->isChecked()) {
            check1 = i;
            break;
        }
    }
    if (check1 == -1)
        return;
    for (int i = check1+1; i < 6; i++) {
        if (buttons[i]->isChecked()) {
            myteam->switchPokemon(check1, i);
            buttons[check1]->setChecked(false);
            buttons[i]->setChecked(false);

            PokeBattle &p1 = myteam->poke(check1);
            layouts[check1]->update(p1.num(), p1.level(), p1.gender(), p1.item());
            PokeBattle &p2 = myteam->poke(i);
            layouts[i]->update(p2.num(), p2.level(), p2.gender(), p2.item());
        }
    }
}

void RearrangeWindow::closeEvent(QCloseEvent *)
{
    emit done();
    close();
}

RearrangeLayout::RearrangeLayout(QWidget *parent, const Pokemon::uniqueId &pokenum, int level, int gender, bool item)
    : QVBoxLayout(parent)
{
    addWidget(pokemonPicture = new QLabel(), 0, Qt::AlignCenter);
    addWidget(levelLabel = new QLabel(), 0, Qt::AlignCenter);
    setSpacing(2);

    pokemonPicture->setObjectName("PokemonPicture");
    levelLabel->setObjectName("LevelLabel");

    update(pokenum, level, gender, item);
}

void RearrangeLayout::update(const Pokemon::uniqueId &pokenum, int level, int gender, bool item)
{
    pokemonPicture->setPixmap(getFullIcon(pokenum, item, gender));
    pokemonPicture->setFixedSize(pokemonPicture->pixmap()->size());

    levelLabel->setText(tr("Lv. %1").arg(level));
}

QPixmap RearrangeLayout::getFullIcon(Pokemon::uniqueId num, bool item, int gender)
{
    QPixmap poke = PokemonInfo::Icon(num);

    QPainter painter(&poke);
    if (item) {
        QPixmap helditem = ItemInfo::HeldItem();
        painter.drawPixmap(20, 22, helditem);
    }

    QPixmap genderIcon = Theme::GenderPicture(gender, Theme::IngameM);
    painter.drawPixmap(32-genderIcon.width(), 3, genderIcon);

    return poke;
}
