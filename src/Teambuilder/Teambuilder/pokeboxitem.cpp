#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QDrag>
#include "pokeboxitem.h"
#include "pokebox.h"
#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/pokemoninfo.h>

PokeBoxItem::PokeBoxItem(PokeTeam *poke, PokeBox *box) : poke(NULL), m_Box(box)
{
    setCursor(Qt::OpenHandCursor);

    changePoke(poke);
    setFlags(QGraphicsItem::ItemIsMovable);
}

void PokeBoxItem::changePoke(PokeTeam *poke)
{
    delete this->poke, this->poke = poke;
    setPixmap(PokemonInfo::Icon(poke->num()));
}

void PokeBoxItem::setBox(PokeBox *newBox)
{
    m_Box = newBox;
}

PokeBoxItem::~PokeBoxItem()
{
    delete poke, poke = NULL;
}

void PokeBoxItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = int((event->pos()-startPos).manhattanLength());
        if(distance >= QApplication::startDragDistance())
        {
            startDrag();
        }
    }
}

void PokeBoxItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(QApplication::mouseButtons() == Qt::NoButton ? Qt::OpenHandCursor : Qt::ClosedHandCursor);

    QGraphicsPixmapItem::hoverMoveEvent(event);
}

void PokeBoxItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!isUnderMouse()) {
        event->ignore();
        return;
    }
    setCursor(Qt::ClosedHandCursor);
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}

void PokeBoxItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    setCursor(Qt::OpenHandCursor);
    QGraphicsPixmapItem::mouseReleaseEvent(event);
}

void PokeBoxItem::startDrag()
{
    QMimeData * data = new QMimeData();

    data->setData("Box", QByteArray::number(qlonglong(m_Box)));
    data->setData("Item", QByteArray::number(m_Box->getNumOf(this)));
    data->setImageData(pixmap());
    QDrag * drag = new QDrag(m_Box->parentWidget());
    drag->setMimeData(data);
    drag->setPixmap(pixmap());
    drag->setHotSpot(QPoint(pixmap().width()/2,pixmap().height()/2));
    drag->exec(Qt::MoveAction);
}
