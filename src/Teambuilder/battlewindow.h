#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"

class AttackZone;
class PokeZone;
class GraphicsZone;

/* The battle window called by the client, online */

class BattleWindow : public QWidget
{
    Q_OBJECT

    PROPERTY(int, currentPoke);
public:
    BattleWindow(const QString &opponent, const TeamBattle &myteam);

    /* analyzes the command and calls the right function */
    void dealWithCommand(const QByteArray &);
    TeamBattle &team();
    const TeamBattle &team() const;

    enum BattleCommand
    {
	SendPoke,
	OfferChoice,
	BeginTurn
    };
    enum
    {
	ZoneOfPokes = 6,
	ZoneOfNothing = 7
    };

    void switchToNaught(bool self);
    void switchTo(int pokezone);

    /* Disable / enable buttons */
    void updatePossibilities();
    /* sends the choice */
    void sendChoice(const BattleChoice &b);

    void printLine(const QString &str);

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
    GraphicsZone *myview;
    QPushButton *myswitch, *myattack, *myforfeit, *mysend;
    TeamBattle myteam;

    /* What can I do? */
    bool possible;
    BattleChoices possibilities;
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

/* An attack zone is the zone where the attacks are displayed */
class AttackZone : public QWidget
{
    Q_OBJECT
public:
    AttackZone(const PokeBattle &poke);
    int moves[4];

    QPushButton *attacks[4];
signals:
    void clicked(int attack);

private:
    QSignalMapper *mymapper;
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
