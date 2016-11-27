#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QtGui>
#include <PokemonInfo/battlestructs.h>
#include <Utilities/otherwidgets.h>
#include "basebattlewindow.h"
#ifdef QT5
#include <QMessageBox>
#include <QToolButton>
#endif

#include "battlewindowattackbutton.h"

class AttackZone;
class PokeZone;
class BattleDisplay;
class TargetSelection;
class StruggleZone;
class TeamProxy;
class PokeProxy;

class BattleInfo : public BaseBattleInfo
{
public:
    BattleInfo(const TeamBattle &myteam, const PlayerInfo &me, const PlayerInfo &opp, int mode, int myself, int oppo);

    /* Possible choices */
    bool possible;

    QList<BattleChoices> choices;
    QList<BattleChoice> choice;
    QList<bool> available;
    QList<bool> done;

    const PokeProxy &currentPoke(int spot) const;
    PokeProxy &currentPoke(int spot);
    PokeProxy &tempPoke(int spot);
    PokeProxy &ownTempPoke(int slot);

    int currentSpot;
    TeamBattle _myteam;
    TeamProxy &myteam();
    QHash<quint16, quint16>& myitems();
    const TeamProxy &myteam() const;

    bool sent;

    int phase;

    enum Phase {
        Regular,
        ItemPokeSelection,
        ItemAttackSelection,
        ItemFieldPokeSelection
    };


    QList<BattleStats> mystats;

    int lastMove[6];
};

/* The battle window called by the client, online */

class BattleWindow : public BaseBattleWindow, public BattleCommandManager<BattleWindow>
{
    Q_OBJECT

public:
    BattleWindow(int battleid, const PlayerInfo &me, const PlayerInfo &opponent, const TeamBattle &myteam, const BattleConfiguration &conf);

    BattleInfo &info() {
        return *(BattleInfo*)(&BaseBattleWindow::info());
    }

    const BattleInfo &info() const {
        return *(BattleInfo*)(&BaseBattleWindow::info());
    }

    enum {
        MoveTab= 0,
        PokeTab= 1,
        SpectatorTab=2,
        ItemTab = 3
    };

    enum {
        TargetTab = 4,
        StruggleTab = 5
    };

    TeamProxy &team();
    const TeamProxy &team() const;
    PokeProxy &poke(int slot);
    const PokeProxy &poke(int slot) const;

    void switchToNaught(int spot);
    void switchTo(int pokezone, int spot, bool forced);

    void onKo(int spot);
    void onSendOut(int spot, int previndex, ShallowBattlePoke* pokemon, bool silent);
    void onHpChange(int spot, int newHp);
    void onPokeballStatusChanged(int player, int poke, int status);
    void onShiftSpots(int player, int spot1, int spot2, bool silent);
    void onBattleEnd(int res, int winner);
    void onPPChange(int spot, int move, int PP);
    void onItemChange(int spot, int poke, int item);
    void onTempPPChange(int spot, int move, int PP);
    void onOfferChoice(int player, const BattleChoices &choice);
    void onMoveChange(int spot, int slot, int move, bool definite);
    void onDefiniteFormeChange(int spot, int, int);
    void onCosmeticFormeChange(int spot, int);
    void onRearrangeTeam(int player, const ShallowShownTeam& team);
    void onChoiceSelection(int player);
    void onChoiceCancellation(int player);
    void onReconnect(int player);
    void onDisconnect(int player);
    void onItemChangeCount(int player, int item, int count);
    void addSpectator(bool add, int id, const QString &);
    void updateTeam(const TeamBattle &b);

    /* Disable / enable buttons */
    void updateChoices();
    /* sends the choice */
    void sendChoice(const BattleChoice &b);

    int ownSlot() const;

    void disable();

    Q_INVOKABLE void forfeit();
public slots:
    void switchClicked(int zone);
    void attackClicked(int zone);
    void zmoveClicked(bool checked);
    void itemActivated(QListWidgetItem*);
    void onDisconnection();
    void sendMessage();
    void offerTie();
    void attackButton();
    void clickClose();
    void emitCancel();
    void switchToPokeZone();
    void sendRearrangedTeam();
signals:
    void forfeit(int battleid);
    void offerTie(int battleid);
protected:
    void closeEvent(QCloseEvent *event);
protected slots:
    void changeAttackText(int i);
    void targetChosen(int i);
    void nullQuestion();
    void questionButtonClicked(QAbstractButton *);
protected:
    int idme() const {
        return info().pInfo[info().myself].id;
    }

    int idopp() const {
        return info().pInfo[info().opponent].id;
    }

    void goToNextChoice();
    void cancel();

    void disableAll();
    void enableAll();

    void openRearrangeWindow(const ShallowShownTeam &t);
    void listItems();

    void updateAttacks(int spot);
    void updateAttacks(AttackZone *zone, const PokeProxy *p);
    void updateAttack(int spot, int moveSlot);
    int currentChoiceIndex() const;

    QStackedWidget *mystack;
    QTabWidget *mytab;
    QListWidget *myspecs, *myitems;
    AttackZone *myazones[4]; /* Zones 0 to 2 are for the pokemon on the field, zone 4 is for special use when you want to put
                               anything in the zone */
    StruggleZone *szone;
    TargetSelection *tarZone;
    QList<QButtonGroup*> mybgroups;
    PokeZone *mypzone;
    QPushButton *myswitch, *myattack, *mycancel;
    QMessageBox *question;

    bool canLeaveBattle;
};

class AbstractAttackButton;

/* An attack zone is the zone where the attacks are displayed */
class AttackZone : public QWidget
{
    Q_OBJECT
public:
    AttackZone(const PokeProxy &poke, Pokemon::gen gen);

    AbstractAttackButton *tattacks[4];
    QAbstractButton *attacks[4];
    QPushButton *megaevo;
    QPushButton *zmove;
signals:
    void clicked(int attack);
    void zmoveClicked(bool checked);

private:
    QSignalMapper *mymapper;
};

class BattlePokeButton;
class TeamProxy;

/* When you want to switch pokemons, that's what you see */
class PokeZone : public QWidget
{
    Q_OBJECT
public:
    PokeZone(const TeamProxy &team);

    BattlePokeButton *pokes[6];
signals:
    void switchTo(int poke);

private:
    QSignalMapper *mymapper;
};

class PokeProxy;

class BattlePokeButton : public QPushButton
{
    Q_OBJECT
public:
    BattlePokeButton(const PokeProxy &p);
    void changePokemon(const PokeProxy &p);
    void update();
    void updateIcon();
    void updateToolTip();
private:

    const PokeProxy *p;
};

class TargetSelection : public QWidget
{
    Q_OBJECT
public:
    TargetSelection(const BattleInfo &info);

    void updateData(const BattleInfo &info, int move);
signals:
    void targetSelected(int target);
private:
    QPushButton * pokes[6];
};

class StruggleZone : public QWidget
{
    Q_OBJECT
public:
    StruggleZone();

signals:
    void attackClicked();
};

class RearrangeLayout;

class RearrangeWindow : public QWidget
{
    Q_OBJECT
public:
    RearrangeWindow(TeamBattle &t, const ShallowShownTeam &op);

signals:
    void forfeit();
    void done();
public slots:
    void runExchanges();
protected:
    void closeEvent(QCloseEvent *);
private:
    TeamBattle *myteam;

    QToolButton *buttons[6];
    RearrangeLayout *layouts[6];
};

class RearrangeLayout : public QVBoxLayout
{
public:
    RearrangeLayout(QWidget *parent, const Pokemon::uniqueId &pokenum, int level, int gender, bool item);

    void update(const Pokemon::uniqueId &pokenum, int level, int gender, bool item);

    static QPixmap getFullIcon(Pokemon::uniqueId num, bool item, int gender);
private:
    QLabel *levelLabel;
    QLabel *pokemonPicture;
};

#endif // BATTLEWINDOW_H
