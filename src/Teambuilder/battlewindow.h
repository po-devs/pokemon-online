#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"

class AttackZone;
class PokeZone;
class BattleDisplay;

enum {
    Myself,
    Opponent
};

struct BattleInfo
{
    /* name [0] = mine, name[1] = other */
    QString name[2];

    /* Possible choices */
    bool possible;
    BattleChoices choices;

    /* My team */
    TeamBattle myteam;
    const PokeBattle &currentPoke() const;
    PokeBattle &currentPoke();
    /* Opponent pokemon */
    ShallowBattlePoke opponent;
    bool opponentAlive;

    /* Current poke for ourself */
    int currentIndex;
};

/* The battle window called by the client, online */

class BattleWindow : public QWidget
{
    Q_OBJECT

    PROPERTY(BattleInfo, info);
public:
    BattleWindow(const QString &me, const QString &opponent, const TeamBattle &myteam);

    /* analyzes the command and calls the right function */
    void dealWithCommand(const QByteArray &);
    TeamBattle &team();
    const TeamBattle &team() const;

    enum BattleCommand
    {
	SendOut,
	SendBack,
	UseAttack,
	OfferChoice,
	BeginTurn,
	ChangePP
    };
    enum
    {
	ZoneOfPokes = 6,
	ZoneOfNothing = 7
    };

    void switchToNaught(bool self);
    void switchTo(int pokezone);

    /* Disable / enable buttons */
    void updateChoices();
    /* sends the choice */
    void sendChoice(const BattleChoice &b);

    void printLine(const QString &str);
    QString name(bool self) const;
    QString nick(bool self) const;

public slots:
    void receiveInfo(QByteArray);
    void switchClicked(int zone);
    void attackClicked(int zone);
    void attackButton();

    void switchToPokeZone();
signals:
    void battleCommand(const BattleChoice &);
    void forfeit();
private:
    QStackedWidget *mystack;
    AttackZone *myazones[6];
    PokeZone *mypzone;
    QTextEdit *mychat;
    QLineEdit *myline;
    BattleDisplay *mydisplay;
    QPushButton *myswitch, *myattack, *myforfeit, *mysend;

    /* What can I do? */

};

/* The graphics zone, where both pokes are displayed */
class GraphicsZone : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsZone();
    /* displays that poke */
    template <class T>
    void switchTo(const T &poke, bool self=true);
    /* Display blank */
    void switchToNaught(bool self);

    /* Loads a pixmap if not loaded otherwise go see graphics */
    QPixmap loadPixmap(quint16 num, bool shiny, bool back, quint8 gender);
    /* We are using a qmap to store the graphics already loaded. So the key of the pixmap
	is a combination of 2 bools, 1 quin8; and one quint16 */
    qint32 key(quint16 num, bool shiny, bool back, quint8 gender) const;
    QMap<qint32, QPixmap> graphics;
    /* Current pixmaps displayed */
    QGraphicsPixmapItem *mine, *foe;
    QGraphicsScene scene;
};

class BattleDisplay : public QWidget
{
    Q_OBJECT
public:
    const BattleInfo & info;

    BattleDisplay(const BattleInfo &i);

    void updatePoke(bool self);

protected:
    const PokeBattle &mypoke() const {return info.currentPoke(); }
    const ShallowBattlePoke &foe() const {return info.opponent; }

    GraphicsZone *zone;
    QLabel *nick[2];
    QProgressBar *bars[2];
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

/* When you want to switch pokemons, that's what you see */
class PokeZone : public QWidget
{
    Q_OBJECT
public:
    PokeZone(const TeamBattle &team);

    QPushButton *pokes[6];
signals:
    void switchTo(int poke);

private:
    QSignalMapper *mymapper;
};


/* Yeepee, at last templates */
template <class T>
void GraphicsZone::switchTo(const T &poke, bool self)
{
    if (self)
	mine->setPixmap(loadPixmap(poke.num(), poke.shiny(), true, poke.gender()));
    else
	foe->setPixmap(loadPixmap(poke.num(), poke.shiny(), false, poke.gender()));
}

#endif // BATTLEWINDOW_H
