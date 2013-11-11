#include "battlewindow.h"
#include "../BattleManager/advancedbattledata.h"
#include "../BattleManager/battleinput.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "basebattlewindow.h"
#include "logmanager.h"
#include "theme.h"
#include "spectatorwindow.h"
#include "../Shared/battlecommands.h"
#include <cstdlib>
#ifdef QT5
#include <QButtonGroup>
#endif

BattleInfo::BattleInfo(const TeamBattle &team, const PlayerInfo &me, const PlayerInfo &opp, int mode, int my, int op)
    : BaseBattleInfo(me, opp, mode, my, op)
{
    possible = false;
    sent = true;
    _myteam = team;
    phase = Regular;

    currentSlot = data->spot(myself);

    for (int i = 0; i < numberOfSlots/2; i++) {
        choices.push_back(BattleChoices());
        choice.push_back(BattleChoice());
        available.push_back(false);
        done.push_back(false);
    }

    memset(lastMove, 0, sizeof(lastMove));
}

QHash<quint16, quint16>& BattleInfo::myitems()
{
    return data->items(myself);
}

TeamProxy &BattleInfo::myteam()
{
    return data->team(myself);
}

const TeamProxy &BattleInfo::myteam() const
{
    return data->team(myself);
}

PokeProxy &BattleInfo::tempPoke(int spot)
{
    //return m_tempPoke[number(spot)];
    return data->tempPoke(spot);
}

const PokeProxy & BattleInfo::currentPoke(int spot) const
{
    return data->poke(spot);
}

PokeProxy & BattleInfo::currentPoke(int spot)
{
    return data->poke(spot);
}

BattleWindow::BattleWindow(int battleId, const PlayerInfo &me, const PlayerInfo &opponent, const TeamBattle &team, const BattleConfiguration &_conf)
{
    canLeaveBattle = false;
    question = NULL;
    this->battleId() = battleId;
    this->started() = false;
    ownid() = me.id;

    conf() = _conf;

    myInfo = new BattleInfo(team, me, opponent, conf().mode, conf().spot(me.id), conf().spot(opponent.id));
    info()._myteam.name = me.name;

    if (conf().ids[0] == ownid()) {
        conf().receivingMode[0] = BattleConfiguration::Player;
        conf().teams[0] = &info()._myteam;
        conf().receivingMode[1] = BattleConfiguration::Spectator;
    } else {
        conf().teams[1] = &info()._myteam;
        conf().receivingMode[1] = BattleConfiguration::Player;
        conf().receivingMode[0] = BattleConfiguration::Spectator;
    }
    conf().avatar[info().myself] = me.avatar;
    conf().avatar[info().opponent] = opponent.avatar;
    conf().name[info().opponent] = opponent.name;

    info().gen = conf().gen;

    BaseBattleWindow::init();

    QSettings s;
    saveLogs->setChecked(s.value("Battle/SaveLogs").toBool());
    log->override = saveLogs->isChecked() ? Log::OverrideYes : Log::OverrideNo;
    replay->override = saveLogs->isChecked() ? Log::OverrideYes : Log::OverrideNo;

    setWindowTitle(tr("Battling against %1").arg(name(info().opponent)));

    myclose->setText(tr("&Forfeit"));
    mylayout->addWidget(mytab = new QTabWidget(), 2, 0, 1, 3);
    mylayout->addWidget(mycancel = new QPushButton(tr("&Cancel")), 3,0);
    mylayout->addWidget(myattack = new QPushButton(tr("&Attack")), 3, 1);
    mylayout->addWidget(myswitch = new QPushButton(tr("&Switch Pokemon")), 3, 2);
    mytab->setObjectName("Modified");

    mytab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mytab->addTab(mystack = new QStackedWidget(), tr("&Moves"));
    mytab->addTab(mypzone = new PokeZone(data().team(info().myself)), tr("&Pokemon"));
    mytab->addTab(myspecs = new QListWidget(), tr("Spectators"));
    if (!info().myitems().empty()) {
        mytab->addTab(myitems = new QListWidget(), tr("Items"));
        myitems->setIconSize(QSize(24,24));
        listItems();
        connect(myitems, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(itemActivated(QListWidgetItem*)));
    } else {
        myitems = NULL;
    }

    myspecs->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    for (int i = 0; i < 4; i++) {
        myazones[i] = new AttackZone(poke(i), gen());
        mystack->addWidget(myazones[i]);
        mybgroups.append(new QButtonGroup());
        for (int j = 0; j < 4; j ++) {
            myazones[i]->attacks[j]->setCheckable(true);
            mybgroups.value(i)->addButton(myazones[i]->attacks[j],j);
        }
        connect(myazones[i], SIGNAL(clicked(int)), SLOT(attackClicked(int)));
    }

    if (data().multiples()) {
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

    switchTo(0,data().spot(info().myself,0), false);

    show();

    disableAll();
    flashIfNeeded(); //So people notice when there's a new battle
}

void BattleWindow::itemActivated(QListWidgetItem *it)
{
    int item = it->data(Qt::UserRole).toInt();
    int target = ItemInfo::Target(item, gen());

    int n = data().slotNum(info().currentSlot);
    ItemChoice ic;
    ic.item = item;
    info().choice[n] = BattleChoice(info().currentSlot, ic);

    if (target == Item::Team || target == Item::Opponent) {
        info().done[n] = true;
        goToNextChoice();
        return;
    }

    if (target == Item::TeamPokemon || target == Item::Attack) {
        info().phase = BattleInfo::ItemPokeSelection;
        switchToPokeZone();
    }
}

void BattleWindow::listItems()
{
    if (!myitems) {
        return;
    }

    myitems->clear();

    auto items = info().myitems();
    QVector<quint16> keys = items.keys().toVector();
    qSort(keys);

    for (int i = 0; i < keys.size(); i++) {
        if (ItemInfo::IsBattleItem(keys[i], gen())) {
            QListWidgetItem *it = new QListWidgetItem(ItemInfo::Icon(keys[i]), tr("%1 (x%2)").arg(ItemInfo::Name(keys[i])).arg(items[keys[i]]));
            it->setData(Qt::UserRole, keys[i]);
            myitems->addItem(it);
        }
    }
}

void BattleWindow::changeAttackText(int i)
{
    if (i == MoveTab) {
        myattack->setText(tr("&Attack"));
    } else if (i == ItemTab)  {
        myattack->setText(tr("&Use Item"));
    } else {
        myattack->setText(tr("&Go Back"));
    }
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
    int snum = data().slotNum(spot);

    if (snum != pokezone || forced) {
        auto poke1 = team().poke(snum);
        auto poke2 = team().poke(pokezone);
        mypzone->pokes[snum]->changePokemon(*poke1);
        mypzone->pokes[pokezone]->changePokemon(*poke2);
    }

    mystack->setCurrentIndex(snum);
    mytab->setCurrentIndex(MoveTab);

    updateAttacks(myazones[snum], &info().tempPoke(spot));
}

void BattleWindow::updateAttacks(AttackZone *zone, PokeProxy *p)
{
    for (int i = 0; i< 4; i++) {
        zone->tattacks[i]->updateAttack(p->move(i)->exposedData(), *p, gen());
    }
}

void BattleWindow::targetChosen(int i)
{
    int n = data().slotNum(info().currentSlot);

    info().choice[n].setTarget(i);
    info().done[n] = true;

    goToNextChoice();
}

void BattleWindow::clickClose()
{
    if (battleEnded || canLeaveBattle) {
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

    if (question->x() < 0 || question->y() < 0) {
        question->move(std::max(question->x(), 0), std::max(question->y(), 0));
    }

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
    int n = data().slotNum(info().currentSlot);
    if (sender() == myswitch && info().mode == ChallengeInfo::Triples && n != 1) {
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

        /* We are selecting a pokemon in our team in order to use an item on them.
          Since we can use items on Koed pokemon (cf. Max Revive), we set the disabled button differently */
        if (info().phase == BattleInfo::ItemPokeSelection) {
            for (int i = 0; i < 6; i++) {
                mypzone->pokes[i]->setDisabled(poke(i).num() == Pokemon::NoPoke);
            }
        }
    }
}

int BattleWindow::ownSlot() const {
    return conf().slot(conf().spot(idme()));
}

void BattleWindow::attackClicked(int zone)
{
    int slot = info().currentSlot;
    int n = data().slotNum(slot);

    if (info().phase == BattleInfo::ItemAttackSelection) {
        info().choice[n].choice.item.attack = zone;
        info().done[n] = true;
        goToNextChoice();
    } else {
        if (zone != -1) //struggle
            info().lastMove[n] = zone;
        if (info().possible) {
            BattleChoice &b = info().choice[n];
            b = BattleChoice(slot, AttackChoice());
            b.setAttackSlot(zone);
            b.setTarget(data().spot(info().opponent));
            if (myazones[n]->megaevo->isChecked()) {
                b.setMegaEvo(true);
            }

            if (!data().multiples()) {
                info().done[n] = true;
                goToNextChoice();
            } else {
                int move = zone == -1 ? int(Move::Struggle) : info().tempPoke(slot).move(zone)->num();
                int target = MoveInfo::Target(move, gen());
                /* Triples still require to choose the target */
                if (target == Move::ChosenTarget || target == Move::PartnerOrUser || target == Move::Partner || target == Move::MeFirstTarget || target == Move::IndeterminateTarget
                        || data().numberOfSlots() > 4) {
                    tarZone->updateData(info(), move);
                    mystack->setCurrentIndex(TargetTab);
                } else {
                    info().done[n] = true;
                    goToNextChoice();
                }
            }
        }
    }
}

void BattleWindow::switchClicked(int zone)
{
    int slot = info().currentSlot;
    int snum = data().slotNum(slot);

    if (!info().possible)
    {
        switchToPokeZone();
    } else {
        if (info().phase == BattleInfo::ItemPokeSelection) {
            info().choice[snum].choice.item.target = data().spot(info().myself,zone);
            int tar = ItemInfo::Target(info().choice[snum].item(), gen());
            if (tar == Item::TeamPokemon) {
                info().done[snum] = true;
                goToNextChoice();
            } else {
                info().phase = BattleInfo::ItemAttackSelection;
                mystack->setCurrentWidget(myazones[3]);
                PokeProxy &p = zone < info().numberOfSlots/2 ? info().tempPoke(data().spot(info().myself, zone)) : poke(zone);
                updateAttacks(myazones[3], &p);
                for (int i = 0; i < 4; i++) {
                    myazones[3]->attacks[i]->setDisabled(poke(zone).move(i)->num() == Move::NoMove);
                }
                switchToPokeZone(); // go to attack zone :)
            }
        } else {
            if (!info().choices[snum].switchAllowed)
                return;
            if (zone == snum) {
                switchTo(snum, slot, false);
            } else {
                BattleChoice &b = info().choice[snum];
                b = BattleChoice(slot, SwitchChoice());
                b.setPokeSlot(zone);
                info().done[snum] = true;
                goToNextChoice();
            }
        }
    }
}

void BattleWindow::goToNextChoice()
{
    for (int i =0; i < info().available.size(); i++)  {
        int slot = data().spot(info().myself, i);
        int n = i;

        if (info().available[n] && !info().done[n]) {
            enableAll();

            info().currentSlot = slot;
            info().phase = BattleInfo::Regular;

            myswitch->setText(tr("&Switch Pokemon"));
            if (info().choices[n].attacksAllowed == false && info().choices[n].switchAllowed == true)
                mytab->setCurrentIndex(PokeTab);
            else {
                switchTo(data().slotNum(slot), slot, false);
                if (info().mode == ChallengeInfo::Triples && i != 1) {
                    myswitch->setText(tr("&Shift to centre"));
                }
            }

            /* moves first */
            if (!data().isKoed(slot))
            {
                int snum = data().slotNum(slot);
                myazones[snum]->megaevo->setVisible(info().choices[n].mega);
                myazones[snum]->megaevo->setChecked(false);
                if (info().choices[n].attacksAllowed == false) {
                    myattack->setEnabled(false);
                    for (int i = 0; i < 4; i ++) {
                        myazones[snum]->attacks[i]->setEnabled(false);
                    }
                    if (myitems) {
                        myitems->setEnabled(false);
                    }
                } else {
                    myattack->setEnabled(true);
                    for (int i = 0; i < 4; i ++) {
                        myazones[snum]->attacks[i]->setEnabled(info().choices[n].attackAllowed[i]);
                    }

                    if (info().choices[n].struggle()) {
                        mystack->setCurrentWidget(szone);
                    } else {
                        mystack->setCurrentWidget(myazones[snum]);
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
                    mypzone->pokes[i]->setEnabled(poke(i).num() != 0 && poke(i).life() > 0 && poke(i).status() != Pokemon::Koed);
                }

                if (data().multiples()) {
                    /* In doubles, whatever happens, you can't switch to your partner */
                    for (int i = 0; i < data().numberOfSlots()/2; i++) {
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

void BattleWindow::disable()
{
    onBattleEnd(0,0); // TODO: add meaninfull numbers
    mysend->setEnabled(false);
    myattack->setEnabled(false);
    myswitch->setEnabled(false);
    mycancel->setEnabled(false);
    disableAll();
    disconnect(myclose);
    connect(myclose, SIGNAL(clicked()), this, SLOT(deleteLater()));
    BaseBattleWindow::disable();
}

void BattleWindow::disableAll()
{
    mypzone->setEnabled(false);
    for (int i = 0; i < 3; i++)
        myazones[i]->setEnabled(false);
    if (data().multiples())
        tarZone->setEnabled(false);
}

void BattleWindow::enableAll()
{
    mypzone->setEnabled(true);
    for (int i = 0; i < 3; i++)
        myazones[i]->setEnabled(true);
    if (data().multiples())
        tarZone->setEnabled(true);
    if (myitems) {
        myitems->setEnabled(true);
    }
}

void BattleWindow::attackButton()
{
    if (mytab->currentIndex() == PokeTab) {
        switchToPokeZone();
        return;
    }

    int slot = info().currentSlot;
    int n = data().slotNum(slot);

    if (info().possible) {
        if (mytab->currentIndex() == ItemTab) {
            if (myitems && myitems->currentItem()) {
                itemActivated(myitems->currentItem());
            } else {
                return;
            }
        } else {
            if (mystack->currentIndex() == TargetTab) {
                /* Doubles, move selection */
                int mv = info().lastMove[data().slotNum(slot)];
                int tar = MoveInfo::Target(mv,gen());
                if (info().choices[n].struggle() || tar == Move::ChosenTarget || tar == Move::MeFirstTarget
                        || mv == Move::Curse || tar == Move::PartnerOrUser) {
                    return; //We have to wait for the guy to choose a target
                }
                info().done[n] = true;
                goToNextChoice();
            } else {
                if (info().phase == BattleInfo::ItemAttackSelection) {
                    if (!myazones[3]->attacks[info().lastMove[info().choice[n].itemTarget()]]->isEnabled()) {
                        return;
                    }
                    attackClicked(info().lastMove[info().choice[n].itemTarget()]);
                } else {
                    //We go with the last move, struggle, or the first possible move
                    if (info().choices[n].struggle()) {
                        /* Struggle! */
                        if (data().multiples()) {
                            attackClicked(-1);
                        } else {
                            BattleChoice &b = info().choice[n];
                            b = BattleChoice(slot, AttackChoice());
                            b.setAttackSlot(-1);
                            b.setTarget(data().spot(info().opponent));
                            info().done[n] = true;
                            goToNextChoice();
                        }
                    } else {
                        if (info().choices[n].attackAllowed[info().lastMove[data().slotNum(slot)]]) {
                            attackClicked(info().lastMove[data().slotNum(slot)]);
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
    }
}

void BattleWindow::sendChoice(const BattleChoice &b)
{
    /* Stores choice made in replay data */
    QByteArray ar;
    DataStream out(&ar, QIODevice::WriteOnly);
    out << quint8(BattleCommands::ChoiceMade) << quint8(info().myself) << b;

    getInput()->receiveData(ar);

    /* Send choice made to the server */
    emit battleCommand(battleId(), b);

    /* Disable the choice selection if we didn't ask just for a draw */
    if (!b.drawChoice())
        info().possible = false;
}

void BattleWindow::onDisconnection()
{
    BaseBattleWindow::onDisconnection();
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
    sendChoice(BattleChoice(data().spot(info().myself), DrawChoice()));
}

void BattleWindow::onSendOut(int spot, int prevIndex, ShallowBattlePoke *p, bool s)
{
    if (data().player(spot) == info().myself) {
        switchTo(prevIndex, spot, true);
    }

    BaseBattleWindow::onSendOut(spot, prevIndex, p, s);
}

void BattleWindow::onHpChange(int spot, int)
{
    if (data().player(spot) == info().myself) {
        mypzone->pokes[data().slotNum(spot)]->update();
    }
}

void BattleWindow::onPPChange(int spot, int move, int)
{
    if (data().isOut(spot)) {
        myazones[data().slotNum(spot)]->tattacks[move]->updateAttack(info().tempPoke(spot).move(move)->exposedData(), info().tempPoke(spot), gen());
    }
    mypzone->pokes[data().slotNum(spot)]->updateToolTip();
}

void BattleWindow::onTempPPChange(int spot, int move, int)
{
    myazones[data().slotNum(spot)]->tattacks[move]->updateAttack(info().tempPoke(spot).move(move)->exposedData(), info().tempPoke(spot), gen());
}

void BattleWindow::onDisconnect(int)
{
    canLeaveBattle = true;
    myclose->setText(tr("&Close"));
}

void BattleWindow::onReconnect(int)
{
    canLeaveBattle = false;
    if (!battleEnded) {
        myclose->setText(tr("&Forfeit"));
    }
}

void BattleWindow::onOfferChoice(int, const BattleChoices &c)
{
    if (info().sent) {
        info().sent = false;
        for (int i = 0; i < info().available.size(); i++) {
            info().available[i] = false;
            info().done[i] = false;
        }
    }

    info().choices[c.numSlot/2] = c;
    info().available[c.numSlot/2] = true;
    /* Allows to ask for draw again */
    mysend->setEnabled(true);
    mysend->setChecked(false);

    if (!started()) {
        started() = true;

        delay(700);
    }
}

void BattleWindow::onMoveChange(int spot, int slot, int, bool definite)
{
    myazones[data().slotNum(spot)]->tattacks[slot]->updateAttack(info().tempPoke(spot).move(slot)->exposedData(), info().tempPoke(spot), gen());
    if (definite) {
        mypzone->pokes[data().slotNum(spot)]->updateToolTip();
    }
}

void BattleWindow::onChoiceSelection(int player)
{
    if (player != info().myself) {
        return;
    }
    info().possible = true;
    info().sent = true;

    goToNextChoice();
}

void BattleWindow::onKo(int spot)
{
    if (data().player(spot) == info().myself) {
        mypzone->pokes[data().slotNum(spot)]->setEnabled(false);
    }
    BaseBattleWindow::onKo(spot);
}

void BattleWindow::onPokeballStatusChanged(int player, int poke, int)
{
    if (player == info().myself) {
        mypzone->pokes[poke]->update();
    }
}

void BattleWindow::onBattleEnd(int res, int winner)
{
    myclose->setText(tr("&Close"));
    BaseBattleWindow::onBattleEnd(res, winner);
}

void BattleWindow::onChoiceCancellation(int) {
    cancel();
}

void BattleWindow::onRearrangeTeam(int, const ShallowShownTeam &team)
{
    openRearrangeWindow(team);
}

void BattleWindow::onShiftSpots(int player, int s1, int s2, bool)
{
    if (player == info().myself) {
        mypzone->pokes[s1]->changePokemon(poke(s1));
        mypzone->pokes[s2]->changePokemon(poke(s2));
    }
}

void BattleWindow::onItemChangeCount(int, int, int)
{
    listItems();
}

void BattleWindow::addSpectator(bool add, int id, const QString &name)
{
    BaseBattleWindow::addSpectator(add,id, name);
    if (add) {
        myspecs->addItem(new QIdListWidgetItem(id, name));
    } else {
        for (int i =0 ; i < myspecs->count(); i++) {
            if ( ((QIdListWidgetItem*)(myspecs->item(i)))->id() == id) {
                delete myspecs->takeItem(i);
                return;
            }
        }
    }
}

void BattleWindow::switchToNaught(int spot)
{
    if (data().player(spot) == info().myself) {
        switchToPokeZone();
    }
}

void BattleWindow::updateChoices()
{
    if (info().choices[0].attacksAllowed == false && info().choices[0].switchAllowed == true)
        mytab->setCurrentIndex(PokeTab);

    /* moves first */
    if (!data().isKoed(data().spot(info().myself, 0)))
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
            mypzone->pokes[i]->setEnabled(poke(i).num() != 0 && poke(i).life() > 0);
        }
    }
    
    if (!info().possible) {
        myattack->setEnabled(false);
        myswitch->setEnabled(false);
    }
}

void BattleWindow::openRearrangeWindow(const ShallowShownTeam &t)
{
    RearrangeWindow *r = new RearrangeWindow(info()._myteam, t);
    r->setParent(this, Qt::Window | Qt::Dialog);
    r->move(x() + (width()-r->width())/2, y() + (height()-r->height())/2);
    r->show();
    r->move(x() + (width()-r->width())/2, y() + (height()-r->height())/2);

    if (r->x() < 0 || r->y() < 0) {
        r->move(std::max(r->x(), 0), std::max(r->y(), 0));
    }

    connect(r, SIGNAL(forfeit()), SLOT(clickClose()));
    connect(r, SIGNAL(done()), SLOT(sendRearrangedTeam()));
    connect(r, SIGNAL(done()), r, SLOT(deleteLater()));
}

void BattleWindow::sendRearrangedTeam()
{
    RearrangeChoice r;

    for (int i = 0; i < 6; i++)
        r.pokeIndexes[i] = info()._myteam.internalId(info()._myteam.poke(i));

    BattleChoice c = BattleChoice(data().spot(info().myself), r);
    sendChoice(c);

    /* If the team was rearranged... */
    reloadTeam(ownid()==conf().ids[0] ? 0 : 1);
    for (int i = 0; i < 6; i++) {
        mypzone->pokes[i]->changePokemon(poke(i));
    }
}

void BattleWindow::updateTeam(const TeamBattle &b)
{
    QString name = info().myteam().name();
    info()._myteam = b;
    info()._myteam.name = name;

    reloadTeam(ownid()==conf().ids[0] ? 0 : 1);
    for (int i = 0; i < 6; i++) {
        mypzone->pokes[i]->changePokemon(poke(i));
    }
}

TeamProxy &BattleWindow::team()
{
    return info().myteam();
}

const TeamProxy &BattleWindow::team() const
{
    return info().myteam();
}

PokeProxy &BattleWindow::poke(int slot)
{
    return *info().myteam().poke(slot);
}

const PokeProxy &BattleWindow::poke(int slot) const
{
    return *info().myteam().poke(slot);
}

AttackZone::AttackZone(const PokeProxy &poke, Pokemon::gen gen)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    QSettings s;

    /* Old buttons look so much better on New Battle window! */
    bool old = s.value("Battle/OldAttackButtons").toBool() || !s.value("Battle/OldWindow", true).toBool();

    if (!old) {
        l->setSpacing(2);
        l->setMargin(5);
    }

    for (int i = 0; i < 4; i++)
    {
        if (old)
            attacks[i] = new OldAttackButton(poke.move(i)->exposedData(), poke, gen);
        else
            attacks[i] = new ImageAttackButton(poke.move(i)->exposedData(), poke, gen);

        tattacks[i] = dynamic_cast<AbstractAttackButton*>(attacks[i]);

        l->addWidget(attacks[i], i >= 2, i % 2);

        mymapper->setMapping(attacks[i], i);
        connect(dynamic_cast<QAbstractButton*>(attacks[i]), SIGNAL(clicked()), mymapper, SLOT(map()));
    }
    megaevo = new QPushButton(this);
    megaevo->setText(tr("Mega evolution"));
    megaevo->setCheckable(true);
    megaevo->setObjectName("MegaEvo");
    l->addWidget(megaevo, 2, 0, 1, 2);
    megaevo->setVisible(false);

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(clicked(int)));
}

OldAttackButton::OldAttackButton(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen)/* : QImageButton("db/BattleWindow/Buttons/0D.png", "db/BattleWindow/Buttons/0H.png")*/
{
    QVBoxLayout *l = new QVBoxLayout(this);

    l->addWidget(name = new QLabel(), 0, Qt::AlignCenter);
    l->addWidget(pp = new QLabel(), 0, Qt::AlignRight | Qt::AlignVCenter);
    name->setObjectName("AttackName");
    pp->setObjectName("AttackPP");
    //setMinimumWidth(200);

    updateAttack(b,p,gen);
}

void OldAttackButton::updateAttack(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen)
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
    setAccessibleName(MoveInfo::Name(b.num()));
}

ImageAttackButton::ImageAttackButton(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen)
    : QImageButton(Theme::path("BattleWindow/Buttons/0D.png"), Theme::path("BattleWindow/Buttons/0H.png"))
{
    QVBoxLayout *l = new QVBoxLayout(this);

    l->addWidget(name = new QLabel(), 0, Qt::AlignCenter);
    l->addWidget(pp = new QLabel(), 0, Qt::AlignRight | Qt::AlignVCenter);
    name->setObjectName("AttackName");
    pp->setObjectName("AttackPP");

    updateAttack(b,p,gen);
}

void ImageAttackButton::updateAttack(const BattleMove &b, const PokeProxy &p, Pokemon::gen gen)
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
    setAccessibleName(MoveInfo::Name(b.num()));
}

PokeZone::PokeZone(const TeamProxy &team)
{
    QGridLayout *l = new QGridLayout(this);
    mymapper = new QSignalMapper(this);

    for (int i = 0; i < 6; i++)
    {
        l->addWidget(pokes[i] = new BattlePokeButton(*team.poke(i)), i >= 3, i % 3);

        mymapper->setMapping(pokes[i], i);
        connect(pokes[i], SIGNAL(clicked()), mymapper, SLOT(map()));
    }

    connect(mymapper, SIGNAL(mapped(int)), SIGNAL(switchTo(int)));
}


BattlePokeButton::BattlePokeButton(const PokeProxy &p)
    : p(&p)
{
    setIconSize(QSize(32,32));
    setIcon(PokemonInfo::Icon(p.num()));
    update();

    updateToolTip();
}

void BattlePokeButton::changePokemon(const PokeProxy &p)
{
    this->p = &p;

    setIcon(PokemonInfo::Icon(p.num()));
    update();

    updateToolTip();
}

void BattlePokeButton::update()
{
    setText(p->nickname() + "\n" + QString::number(p->life()) + "/" + QString::number(p->totalLife()));
    int status = p->status();
    if (status == Pokemon::Koed || status == Pokemon::Fine) {
        setStyleSheet("");
    } else {
        setStyleSheet("background: " + Theme::StatusColor(status).name() + ";");
    }
    
    updateToolTip();
    setAccessibleName(PokemonInfo::Name(p->num()));
}

void BattlePokeButton::updateToolTip()
{
    const PokeProxy &p = *(this->p);
    QString tooltip;
    if (p.ability() != 0) {
        tooltip = tr("%1 lv %2\n\nItem:%3\nAbility:%4\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                .arg(PokemonInfo::Name(p.num()), QString::number(p.level()), ItemInfo::Name(p.item()),
                     AbilityInfo::Name(p.ability()), MoveInfo::Name(p.move(0)->num()), MoveInfo::Name(p.move(1)->num()),
                     MoveInfo::Name(p.move(2)->num()), MoveInfo::Name(p.move(3)->num())).arg(p.move(0)->PP()).arg(p.move(1)->PP())
                .arg(p.move(2)->PP()).arg(p.move(3)->PP());
    } else if (p.ability() == 0) {
        if (p.item() != 0) {
            tooltip = tr("%1 lv %2\nItem:%3\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                    .arg(PokemonInfo::Name(p.num()), QString::number(p.level()), ItemInfo::Name(p.item()),
                         MoveInfo::Name(p.move(0)->num()), MoveInfo::Name(p.move(1)->num()),
                         MoveInfo::Name(p.move(2)->num()), MoveInfo::Name(p.move(3)->num())).arg(p.move(0)->PP()).arg(p.move(1)->PP())
                    .arg(p.move(2)->PP()).arg(p.move(3)->PP());
        } else {
            tooltip = tr("%1 lv %2\n\nMoves:\n--%5 - %9 PP\n--%6 - %10 PP\n--%7 - %11 PP\n--%8 - %12 PP")
                    .arg(PokemonInfo::Name(p.num()), QString::number(p.level()),
                         MoveInfo::Name(p.move(0)->num()), MoveInfo::Name(p.move(1)->num()),
                         MoveInfo::Name(p.move(2)->num()), MoveInfo::Name(p.move(3)->num())).arg(p.move(0)->PP()).arg(p.move(1)->PP())
                    .arg(p.move(2)->PP()).arg(p.move(3)->PP());
        }
    }
    setToolTip(tooltip);
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
        bool opp = info.data->player(i) == info.opponent;

        gl->addWidget(pokes[i] = new QPushButton(), !opp, info.data->slotNum(i));
        pokes[i]->setCheckable(true);
        pokes[i]->setObjectName("PokemonTargetButton");
        pokes[i]->setIconSize(QSize(32,32));

        bg->addButton(pokes[i], i);
    }

    connect(bg, SIGNAL(buttonClicked(int)), SIGNAL(targetSelected(int)));
}

void TargetSelection::updateData(const BattleInfo &info, int move)
{
    int slot = info.currentSlot;
    int num = info.numberOfSlots;
    auto gen = info.gen;
    advbattledata_proxy &data = *info.data;

    for (int i = 0; i < num; i++) {
        if (info.data->poke(i).status() == Pokemon::Koed) {
            pokes[i]->setText("");
            pokes[i]->setIcon(QIcon());
        } else {
            pokes[i]->setText(data.poke(i).nickname());
            pokes[i]->setIcon(PokemonInfo::Icon(data.poke(i).num()));
        }
        pokes[i]->setDisabled(true);
        pokes[i]->setChecked(false);
        pokes[i]->setStyleSheet("");
    }

    switch (Move::Target(MoveInfo::Target(move, gen))) {
    case Move::All:
        for (int i = 0; i < num; i++) {
            if (data.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::AllButSelf:
        for (int i = 0; i < num; i++) {
            if (i != slot && data.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::Opponents:
        for (int i = 0; i < num; i++) {
            if (data.player(i) == info.opponent && data.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::OpposingTeam:
        for (int i = 0; i < num; i++) {
            if (data.player(i) == info.opponent) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::TeamParty: case Move::TeamSide:
        for (int i = 0; i < num; i++) {
            if (data.player(i) == info.myself) {
                pokes[i]->setEnabled(true);
                pokes[i]->setStyleSheet("background: #07a7c9; color: white;");
            }
        }
        break;
    case Move::IndeterminateTarget:
    case Move::ChosenTarget:
        for (int i = 0; i < num; i++) {
            if (i != slot &&
                    ((MoveInfo::Flags(move, gen) & Move::PulsingFlag) || data.areAdjacent(i, slot)))
                pokes[i]->setEnabled(true);
        }
        break;
    case Move::PartnerOrUser:
        for (int i = 0; i < num; i++) {
            if (data.player(i) == info.myself && data.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
            }
        }
        break;
        /* Me first only allows targetting opponents but not allies */
    case Move::MeFirstTarget:
        for (int i = 0; i < num; i++) {
            if (data.player(i) != info.myself && data.areAdjacent(i, slot)) {
                pokes[i]->setEnabled(true);
            }
        }
        break;
    case Move::Partner:
        for (int i = 0; i < num; i++) {
            if (data.player(i) == info.myself && i != slot  && data.areAdjacent(i, slot)) {
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
