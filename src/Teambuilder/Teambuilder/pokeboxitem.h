#ifndef POKEBOXITEM_H
#define POKEBOXITEM_H

#include <QGraphicsPixmapItem>

class PokeBox;
class PokeTeam;

class PokeBoxItem : public QGraphicsPixmapItem
{
public:
    PokeBoxItem(PokeTeam *poke, PokeBox *box);
    ~PokeBoxItem();

    void changePoke(PokeTeam *poke);
    void setBox(PokeBox *newBox);

    PokeTeam *poke;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void startDrag();

    QPointF startPos;

private:
    PokeBox *m_Box;
};

#endif // POKEBOXITEM_H
