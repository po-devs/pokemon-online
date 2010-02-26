#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"
#include "basebattlewindow.h"

class AttackZone;
class PokeZone;
class BattleDisplay;
class QScrollDownTextEdit;

class BattleInfo : public BaseBattleInfo
{
public:
    BattleInfo(const TeamBattle &myteam, const QString &me, const QString &opp);

    /* Possible choices */
    bool possible;
    BattleChoices choices;

    /* My team */
    TeamBattle myteam;
    const PokeBattle &currentPoke() const;
    PokeBattle &currentPoke();

    BattleStats mystats;

    /* Current poke for ourself */
    int currentIndex;
    int lastIndex;
    int validIndex() const {
        return currentIndex == -1 ? lastIndex : currentIndex;
    }

    PROPERTY(PokeBattle, tempPoke);
};

/* The battle window called by the client, online */

class Client;

class BattleWindow : public BaseBattleWindow
{
    Q_OBJECT

    PROPERTY(BattleConfiguration, conf);
    PROPERTY(int, idme);
    PROPERTY(int, idopp);
public:
    BattleWindow(const QString &me, const QString &opponent, int idme, int idopp, const TeamBattle &myteam, const BattleConfiguration &conf);

    BattleInfo &info() {
        return *(BattleInfo*)(&BaseBattleWindow::info());
    }

    const BattleInfo &info() const {
        return *(BattleInfo*)(&BaseBattleWindow::info());
    }

    TeamBattle &team();
    const TeamBattle &team() const;

    enum {
        ZoneOfPokes = 6
    };

    void switchToNaught(int spot);
    void switchTo(int pokezone);

    /* Disable / enable buttons */
    void updateChoices();
    /* sends the choice */
    void sendChoice(const BattleChoice &b);

    QString nick(int spot) const;

public slots:
    void switchClicked(int zone);
    void attackClicked(int zone);
    void sendMessage();
    void attackButton();
    void clickClose();
    void emitCancel();
    void switchToPokeZone();
signals:
    void battleCommand(const BattleChoice &);
    void battleMessage(const QString &str);
    void forfeit();
protected:
    void closeEvent(QCloseEvent *);
    virtual void dealWithCommandInfo(QDataStream &s, int command, int spot, int truespot);
private:
    QStackedWidget *mystack;
    AttackZone *myazones[6];
    PokeZone *mypzone;
    QPushButton *myswitch, *myattack, *mycancel;
};

class BattleDisplay : public BaseBattleDisplay
{
    Q_OBJECT
public:
    BattleDisplay(BattleInfo &i);

    void updatePoke(int spot);
    void updateToolTip(int spot);

    BattleInfo &info() const {
        return *(BattleInfo *)(&BaseBattleDisplay::info());
    }

protected:
    const PokeBattle &mypoke() const {return info().currentPoke(); }
    const ShallowBattlePoke &foe() const {return info().pokes[Opponent]; }
};


class AttackButton;

/* An attack zone is the zone where the attacks are displayed */
class AttackZone : public QWidget
{
    Q_OBJECT
public:
    AttackZone(const PokeBattle &poke);

    AttackButton *attacks[4];
signals:
    void clicked(int attack);

private:
    QSignalMapper *mymapper;
};

class AttackButton: public QPushButton
{
    Q_OBJECT
public:
    AttackButton(const BattleMove& b);
    void updateAttack(const BattleMove& b);

    QLabel *name;
    QLabel *pp;
};

class PokeButton;

/* When you want to switch pokemons, that's what you see */
class PokeZone : public QWidget
{
    Q_OBJECT
public:
    PokeZone(const TeamBattle &team);

    PokeButton *pokes[6];
signals:
    void switchTo(int poke);

private:
    QSignalMapper *mymapper;
};

class PokeButton : public QPushButton
{
    Q_OBJECT
public:
    PokeButton(const PokeBattle &p);
    void update();
private:

    const PokeBattle *p;
};


#endif // BATTLEWINDOW_H
