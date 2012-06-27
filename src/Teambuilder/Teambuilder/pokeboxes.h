#ifndef POKEBOXES_H
#define POKEBOXES_H

#include <QtGui>
#include "Teambuilder/teambuilderwidget.h"

class PokeTeam;
class TeamHolder;
class PokemonItem;
class PokeBox;

namespace Ui {
class PokeBoxes;
}

class PokeBoxes :  public TeamBuilderWidget
{
    Q_OBJECT

public:
    explicit PokeBoxes(QWidget *parent = 0, TeamHolder *nteam = NULL);
    ~PokeBoxes();
    void changePoke(PokeTeam *poke);
    void updatePoke();

    void updateTeam();
public slots:
    void changeTeamPoke(int index);

private:
    PokeTeam *m_poke;
    TeamHolder *m_team;
    Ui::PokeBoxes *ui;
    const PokeTeam &poke() const {return *m_poke;}
    const TeamHolder &team() const {return *m_team;}
    PokeTeam &poke() {return *m_poke;}
    TeamHolder &team() {return *m_team;}
};

class PokemonItem : public QGraphicsPixmapItem
{
public:
    PokemonItem(PokeTeam *poke, PokeBox *box);
    ~PokemonItem();

    void changePoke(PokeTeam *poke);
    void setBox(PokeBox *newBox);

    PokeTeam *poke;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void startDrag();

    QPointF startPos;

private:
    PokeBox *m_Box;
};

class PokeBox : public QGraphicsView
{
    Q_OBJECT

public:
    PokeBox(QFrame *parent, int boxNum, const QString &file);

    void deleteBox();
    void saveBox();
    void loadBox();
    void addPokemonToBox(const PokeTeam &poke, int slot = -1);

    PokeTeam *getCurrent();
    void deleteCurrent();
    void changeCurrent(const PokeTeam &poke);
    void changeCurrentSpot(int newSpot);

    QString getBoxName() const;
    void setName(const QString &name);
    void reName(const QString &newName);
    bool isFull() const;
    bool isEmpty() const;
    int freeSpot() const;

    int getNum() const;
    void setNum(int number);

    int getNumOf(const PokemonItem *item) const;

    static QString getBoxPath();

signals:
    void switchWithTeam(int boxNumber, int boxSlot, int teamSlot);
    void show(PokeTeam *team);

protected:
    void addGraphicsItem(int spot);
    QPointF calculatePos(int spot, const QSize& itemSize = QSize(32,32));
    int calculateSpot(const QPoint &graphicViewPos);
    void drawBackground(QPainter *painter, const QRectF &rect);
    PokemonItem* currentItem();
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

private:
    QVector<PokemonItem*> m_Pokemons;
    QString m_Name;
    QPixmap m_Background;
    int Number;
    int currentPokemon;
};

#endif // POKEBOXES_H
