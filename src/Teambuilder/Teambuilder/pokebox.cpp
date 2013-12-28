#include <QDomElement>
#include <QApplication>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>
#include <QMouseEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include "TeambuilderLibrary/theme.h"
#include "pokebox.h"
#include "pokeboxitem.h"
#include "../PokemonInfo/pokemonstructs.h"

PokeBox::PokeBox(int boxNum, const QString &file) : m_Num(boxNum), currentPokemon(0), isLoaded(false)
{
    m_Empty = Theme::Sprite("boxempty");
    m_Clicked = Theme::Sprite("boxclicked");
    m_Occupied = Theme::Sprite("boxoccupied");
    m_Hover = Theme::Sprite("boxhover");

    m_Pokemons.resize(30);

    setMouseTracking(true);
    setScene(new QGraphicsScene(this));
    setSceneRect(0, 0, width() - 10, 160);

    QFileInfo f(getBoxPath() + "/" + file);
    m_Name = QUrl::fromPercentEncoding(f.baseName().toUtf8());

    if (!f.exists()) {
        QFile in(getBoxPath() + "/" + file);
        in.open(QIODevice::WriteOnly);
        in.close();
    }
}

void PokeBox::loadIfNeeded()
{
    if (!isLoaded) {
        loadBox();
        isLoaded = true;
    }
}

void PokeBox::deleteBox()
{
    QDir d(getBoxPath());
    QString file = QString::fromUtf8(QUrl::toPercentEncoding(getBoxName()))+".box";
    d.remove(file);
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
    if (poke.num() == Pokemon::NoPoke) {
        throw tr("Can't store a Missingno");
    }

    int spot = slot == -1 ? (m_Pokemons[currentPokemon] == NULL ? currentPokemon : freeSpot()) : slot;
    m_Pokemons[spot] = new PokeBoxItem(new PokeTeam(poke), this);

    addGraphicsItem(spot);
    changeCurrentSpot(spot);
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

void PokeBox::updateSpots(int i, int j)
{
    updateScene(QList<QRectF>() << QRectF(calculatePos(i, m_Clicked.size()), m_Clicked.size())
                                << QRectF(calculatePos(j, m_Clicked.size()), m_Clicked.size()));
}

void PokeBox::changeCurrentSpot(int newSpot)
{
    if(newSpot == currentPokemon) {
        return;
    }

    updateSpots(currentPokemon, newSpot);

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
    m_Dir.rename(QString::fromUtf8(QUrl::toPercentEncoding(getBoxName())) + ".box",
                    QString::fromUtf8(QUrl::toPercentEncoding(newName)) + ".box");
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

int PokeBox::currentSlot() const
{
    return currentPokemon;
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
    m_Pos.setX((spot % 10) * 66 + (66-itemSize.width()) / 2);
    m_Pos.setY((spot / 10) * 52 + (66-itemSize.height()) / 2);

    return m_Pos;
}

int PokeBox::calculateSpot(const QPoint &graphicViewPos)
{
    QPointF m_Pos = mapToScene(graphicViewPos);
    int x, y;

    if (m_Pos.x() < 0 || m_Pos.y() < 0) {
        return -1;
    }

    x = int(m_Pos.x() / 66 );
    y = int(m_Pos.y() / 52 );

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

        if (calculateSpot(oldMousePos) == i) {
            //Not really the best but w/e
            if (QApplication::mouseButtons() != Qt::NoButton) {
                type = m_Clicked;
            } else {
                type = m_Hover;
            }
        } else if (i == currentPokemon) {
            type = m_Clicked;
        } else if (m_Pokemons[i]) {
            type = m_Occupied;
        } else {
            type = m_Empty;
        }

        QPointF self_back_pos = calculatePos(i, m_Clicked.size());
        QRectF selfRect = QRectF(self_back_pos, m_Clicked.size());
        if (!rect.intersects(selfRect)) {
            continue;
        }
        QRectF intersection = rect.intersected(selfRect);
        QRectF srcRect = QRectF(std::max(qreal(0), intersection.x() - self_back_pos.x()), std::max(qreal(0), intersection.y() - self_back_pos.y()),
                                std::min(qreal(type.width()),intersection.width()), std::min(qreal(type.height()),intersection.height()));
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
    updateSpots(calculateSpot(event->pos()), -1);

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

void PokeBox::mouseReleaseEvent(QMouseEvent *event)
{
    updateSpots(calculateSpot(event->pos()), -1);

    QGraphicsView::mouseReleaseEvent(event);
}

void PokeBox::mouseMoveEvent(QMouseEvent *event)
{
    QPoint swap = oldMousePos;
    oldMousePos = event->pos();
    updateSpots(calculateSpot(swap), calculateSpot(oldMousePos));

    QGraphicsView::mouseMoveEvent(event);
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

    bool b1 = data->data("Box").isNull();
    bool b2 = data->data("Item").isNull();

    if(!b1 && !b2) {
        event->accept();

        PokeBox *box = (PokeBox*)(intptr_t)data->data("Box").toLongLong();
        int item = data->data("Item").toInt();

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

    } else if (!data->data("TeamSlot").isNull()) {
        int slot = data->data("TeamSlot").toInt();
        event->accept();

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
