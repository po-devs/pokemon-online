#ifndef BATTLEWINDOW_H
#define BATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"

class AttackZone;
class PokeZone;

class BattleWindow : public QWidget
{
    Q_OBJECT
public:
    BattleWindow(const QString &opponent);

    /* analyzes the command and calls the right function */
    void dealWithCommand(const QByteArray &);
    TeamBattle &team();
    const TeamBattle &team() const;

    int currentPoke() const;
public slots:
    void switchTo(int pokezone);
    void switchToPokeZone();
signals:
    void battleCommand(const QByteArray &);
private:
    QStackedWidget *mystack;
    AttackZone *myazones[6];
    PokeZone *mypzone;
    QTextEdit *mychat;
    QLineEdit *myline;
    QGraphicsView *myview;
    QPushButton *myswitch, *myattack, *myforfeit, *mysend;
    TeamBattle myteam;

    int mycurrentpoke;
};

class AttackZone : public QWidget
{
    Q_OBJECT
public:
    AttackZone();
    int moves[4];
private:
    QPushButton *attacks[4];
};

class PokeZone : public QWidget
{
    Q_OBJECT
public:
    PokeZone();
signals:
    void switchTo(int poke);
private:
    QPushButton *pokes[6];
    QSignalMapper *mymapper;
};

#endif // BATTLEWINDOW_H
