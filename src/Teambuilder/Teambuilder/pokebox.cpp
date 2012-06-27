#include "pokebox.h"
#include "pokeboxitem.h"
#include <QDomElement>
#include <QFileInfo>
#include <QUrl>
#include <QMessageBox>
#include "../PokemonInfo/pokemonstructs.h"

PokeBox::PokeBox(int boxNum, const QString &file)
{
    m_Pokemons.resize(30);
    m_Num = boxNum;

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
    //TODO
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
}

void PokeBox::addPokemonToBox(const PokeTeam &poke, int slot)
{
    if(isFull()) {
        QMessageBox::critical(0, QString(tr("Box #%1 - %2").arg(getNum()).arg(getBoxName())), QObject::tr("Could not add the Pokemon to the box, the box is full."), QMessageBox::Ok);
        return;
    }
    int spot = slot == -1 ? (m_Pokemons[currentPokemon] == NULL ? currentPokemon : freeSpot()) : slot;
    m_Pokemons[spot] = new PokeBoxItem(new PokeTeam(poke), this);

    addGraphicsItem(spot);
    changeCurrentSpot(spot);
}

PokeTeam *PokeBox::getCurrent()
{
    if(m_Pokemons[currentPokemon] == NULL) {
        QMessageBox::critical(0, QString(tr("Box #%1 - %2").arg(getNum()).arg(getBoxName())), QObject::tr("There's no Pokemon there."), QMessageBox::Ok);
        return NULL;
    }
    return m_Pokemons[currentPokemon]->poke;
}

void PokeBox::deleteCurrent()
{
    if(m_Pokemons[currentPokemon] == NULL) {
        QMessageBox::critical(0, QString(tr("Box #%1 - %2").arg(getNum()).arg(getBoxName())), QObject::tr("There's no Pokemon there."), QMessageBox::Ok);
        return;
    }
    scene()->removeItem(m_Pokemons[currentPokemon]);
    delete m_Pokemons[currentPokemon];
    m_Pokemons[currentPokemon] = NULL;
}

void PokeBox::changeCurrent(const PokeTeam &poke)
{
    if(m_Pokemons[currentPokemon] == NULL) {
        QMessageBox::critical(0, QString(tr("Box #%1 - %2").arg(getNum()).arg(getBoxName())), QObject::tr("There's no Pokemon there."), QMessageBox::Ok);
        return;
    }
    m_Pokemons[currentPokemon]->changePoke(new PokeTeam(poke));
}

void PokeBox::changeCurrentSpot(int newSpot)
{
    if(newSpot == currentPokemon) {
        return;
    }
    updateScene(QList<QRectF>() << QRectF(calculatePos(currentPokemon, m_Background.size()), m_Background.size())
                                << QRectF(calculatePos(newSpot, m_Background.size()), m_Background.size()));
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

    if(currentPokemon != -1) {
        QPointF self_back_pos = calculatePos(currentPokemon, m_Background.size());
        QRectF intersection = rect.intersect(QRectF(self_back_pos, m_Background.size()));
        QRectF srcRect = QRectF(std::max(qreal(0), intersection.x() - self_back_pos.x()), std::max(qreal(0), intersection.y() - self_back_pos.y()),
                                m_Background.width(), m_Background.height());
        painter->drawPixmap(intersection, m_Background, srcRect);
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
    (void) event;
}

void PokeBox::dragEnterEvent(QDragEnterEvent *event)
{
    (void) event;
}

void PokeBox::dropEvent(QDropEvent *event)
{
    (void) event;
}

void PokeBox::dragMoveEvent(QDragMoveEvent *event)
{
    (void) event;
}
