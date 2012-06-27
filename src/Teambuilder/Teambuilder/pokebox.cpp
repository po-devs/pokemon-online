#include <QDomElement>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include "pokebox.h"
#include "pokeboxitem.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "theme.h"

Q_DECLARE_METATYPE(PokeBox*)

PokeBox::PokeBox(int boxNum, const QString &file) : m_Num(boxNum), currentPokemon(0)
{
    m_Empty = Theme::Sprite("boxempty");
    m_Clicked = Theme::Sprite("boxclicked");
    m_Occupied = Theme::Sprite("boxoccupied");
    m_Hover = Theme::Sprite("boxhover");

    m_Pokemons.resize(30);

    setScene(new QGraphicsScene(this));
    setSceneRect(0, 0, width() - 10, 160);

    QFileInfo f(file);
    m_Name = QUrl::fromPercentEncoding(f.baseName().toUtf8());
}

void PokeBox::deleteBox()
{
    //TODO
}

void PokeBox::saveBox()
{
    QDomDocument doc;

    QDomElement box = doc.createElement("Box");
    box.setAttribute("Num", getNum());
    box.setAttribute("Version", "1");
    doc.appendChild(box);

    for(int i = 0; i < m_Pokemons.size(); i++) {
        if (m_Pokemons[i]) {
            QDomElement slot = doc.createElement("Slot");
            slot.setAttribute("Num", i);
            box.appendChild(slot);
            QDomElement pokemon = doc.createElement("Pokemon");
            slot.appendChild(m_Pokemons[i]->poke->toXml(pokemon));
        }
    }

    QFile out(QString(getBoxPath() + "/%1.box").arg(QString::fromUtf8(QUrl::toPercentEncoding(getBoxName()))));
    out.open(QIODevice::WriteOnly);
    QTextStream str(&out);
    doc.save(str,4);
}

void PokeBox::loadBox()
{
    QDomDocument doc;

    QFile in(QString(getBoxPath() + "/%1.box").arg(QString::fromUtf8(QUrl::toPercentEncoding(getBoxName()))));
    in.open(QIODevice::ReadOnly);

    if(!doc.setContent(&in))
    {
        return ;
    }

    QDomElement box = doc.firstChildElement("Box");
    if (box.isNull())
        return;
    QDomElement slot = box.firstChildElement("Slot");
    int version = box.attribute("Version", "0").toInt();

    try {
        while (!slot.isNull()) {
            if (slot.attribute("Num").toInt() < 0 || slot.attribute("Num").toInt() > m_Pokemons.size())
                break;
            int num = slot.attribute("Num").toInt();

            if (m_Pokemons[num] != NULL)
                break;

            QDomElement poke = slot.firstChildElement("Pokemon");

            if (poke.isNull())
                break;

            PokeTeam p;
            p.loadFromXml(poke, version);

            addPokemonToBox(p,num);

            slot = slot.nextSiblingElement("Slot");
        }
    } catch(const QString &ex) {
        qDebug() << "Error when loading box " << getBoxName() << ": " << ex;
    }
}

void PokeBox::addPokemonToBox(const PokeTeam &poke, int slot)
{
    if(isFull()) {
        throw tr("Could not add the Pokemon to the box, the box is full.");
    }

    int spot = slot == -1 ? (m_Pokemons[currentPokemon] == NULL ? currentPokemon : freeSpot()) : slot;
    m_Pokemons[spot] = new PokeBoxItem(new PokeTeam(poke), this);

    addGraphicsItem(spot);
    changeCurrentSpot(spot, true);
}

PokeTeam *PokeBox::getCurrent()
{
    if(m_Pokemons[currentPokemon] == NULL) {
        throw tr("There's no Pokemon there.");
    }

    return m_Pokemons[currentPokemon]->poke;
}

void PokeBox::deleteCurrent()
{
    if(m_Pokemons[currentPokemon] == NULL) {
        throw tr("There's no Pokemon there.");
    }

    scene()->removeItem(m_Pokemons[currentPokemon]);
    delete m_Pokemons[currentPokemon];
    m_Pokemons[currentPokemon] = NULL;
}

void PokeBox::changeCurrent(const PokeTeam &poke)
{
    if(m_Pokemons[currentPokemon] == NULL) {
        throw tr("There's no Pokemon there.");
    }

    m_Pokemons[currentPokemon]->changePoke(new PokeTeam(poke));
}

void PokeBox::changeCurrentSpot(int newSpot, bool f)
{
    if(newSpot == currentPokemon && !f) {
        return;
    }
    updateScene(QList<QRectF>() << QRectF(calculatePos(currentPokemon, m_Occupied.size()), m_Occupied.size())
                                << QRectF(calculatePos(newSpot, m_Occupied.size()), m_Occupied.size()));
    currentPokemon = newSpot;
}

QString PokeBox::getBoxName() const
{
    return this->m_Name;
}

void PokeBox::setName(const QString &name)
{
    this->m_Name = name;
}

void PokeBox::reName(const QString &newName)
{
    QDir m_Dir(getBoxPath());
    m_Dir.rename(getBoxName() + ".box", newName + ".box");
    setName(newName);
}

bool PokeBox::isFull() const
{
    return !m_Pokemons.contains(NULL);
}

bool PokeBox::isEmpty() const
{
    for(int pokeCount = 0; pokeCount < m_Pokemons.size(); pokeCount++) {
        if(m_Pokemons[pokeCount] != NULL) {
            return false;
        }
    }
    return true;
}

int PokeBox::freeSpot() const {
    for(int pokeCount = 0; ; pokeCount++) {
        if(m_Pokemons[pokeCount] == NULL) {
            return pokeCount;
        }
    }
}

int PokeBox::getNum() const
{
    return this->m_Num;
}

void PokeBox::setNum(int number)
{
    this->m_Num = number;
}

int PokeBox::getNumOf(const PokeBoxItem *item) const
{
    for(int pokeCount = 0; pokeCount < m_Pokemons.size(); pokeCount++) {
        if(item == m_Pokemons[pokeCount]) {
            return pokeCount;
        }
    }
    return -1;
}

QString PokeBox::getBoxPath()
{
    return appDataPath("Boxes", true);
}

void PokeBox::addGraphicsItem(int spot)
{
    QPointF m_Pos = calculatePos(spot);
    m_Pokemons[spot]->setPos(m_Pos);
    scene()->addItem(m_Pokemons[spot]);
    m_Pokemons[spot]->setBox(this);
}

QPointF PokeBox::calculatePos(int spot, const QSize &itemSize)
{
    QPointF m_Pos;
    m_Pos.setX((spot % 10) * 64 + 24 - itemSize.width() / 2);
    m_Pos.setY((spot / 10) * 50 + 24 - itemSize.height() / 2);

    return m_Pos;
}

int PokeBox::calculateSpot(const QPoint &graphicViewPos)
{
    QPointF m_Pos = mapToScene(graphicViewPos);
    int x, y;

    x = int((m_Pos.x() - 24) / 64 + 0.5);
    y = int((m_Pos.y() - 24) / 50 + 0.5);

    if (x < 10 && y < 3 && x >= 0 && y >= 0) {
        return y * 10 + x;
    } else {
        return -1;
    }
}

void PokeBox::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    for (int i = 0; i < m_Pokemons.size(); i++) {
        QPixmap type;

        if (i == currentPokemon) {
            type = m_Hover;
        } else if (m_Pokemons[i]) {
            type = m_Occupied;
        } else {
            type = m_Empty;
        }

        QPointF self_back_pos = calculatePos(i, type.size());
        QRectF intersection = rect.intersect(QRectF(self_back_pos, type.size()));
        QRectF srcRect = QRectF(std::max(qreal(0), intersection.x() - self_back_pos.x()), std::max(qreal(0), intersection.y() - self_back_pos.y()),
                                type.width(), type.height());
        painter->drawPixmap(intersection, type, srcRect);
    }
}

PokeBoxItem *PokeBox::currentItem()
{
    if(currentPokemon == -1) {
        return NULL;
    } else {
        return m_Pokemons[currentPokemon];
    }
}

void PokeBox::mousePressEvent(QMouseEvent *event)
{
    /* To let other items than the first clicked have the mouse */
    if (scene()->mouseGrabberItem())
        scene()->mouseGrabberItem()->ungrabMouse();

    int spot = calculateSpot(event->pos());

    if (spot != -1) {
        changeCurrentSpot(spot);

        /* To grab the mouse even if the mouse is a few
           pixels amiss */
        if (m_Pokemons[spot] != NULL) {
            m_Pokemons[spot]->grabMouse();
            emit show(m_Pokemons[spot]->poke);
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void PokeBox::dragEnterEvent(QDragEnterEvent *event)
{
    event->setDropAction(Qt::MoveAction);
    event->accept();
}

void PokeBox::dropEvent(QDropEvent *event)
{
    int spot;

    if ( (spot = calculateSpot(event->pos())) == -1 ) {
        return;
    }

    const QMimeData *data = event->mimeData();

    if(!data->property("Box").isNull() && !data->property("Item").isNull()) {
        event->accept();

        PokeBox *box = data->property("Box").value<PokeBox*>();
        int item = data->property("Item").toInt();

        if (box == this && item == spot)
            return;

        changeCurrentSpot(spot);

        std::swap(m_Pokemons[spot], box->m_Pokemons[item]);

        addGraphicsItem(spot);

        if (box->m_Pokemons[item])
            box->addGraphicsItem(item);

        saveBox();
        if (box != this)
            box->saveBox();

    } else if (!data->property("TeamSlot").isNull()) {
        event->accept();

        int slot = data->property("TeamSlot").toInt();
        emit switchWithTeam(getNum(), currentPokemon, slot);
    }
}

void PokeBox::dragMoveEvent(QDragMoveEvent *event)
{
    int spot = calculateSpot(event->pos());

    if (spot != -1) {
        event->setDropAction(Qt::MoveAction);
        event->accept();

        changeCurrentSpot(spot);
    }
}
