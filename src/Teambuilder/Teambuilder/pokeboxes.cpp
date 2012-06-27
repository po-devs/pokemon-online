#include "pokeboxes.h"
#include "ui_pokeboxes.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "Teambuilder/teamholder.h"
#include "theme.h"

PokeBoxes::PokeBoxes(QWidget *parent, TeamHolder *nteam) :
    TeamBuilderWidget(parent), m_team(nteam),
    ui(new Ui::PokeBoxes)
{
    ui->setupUi(this);

    ui->pokemonButtons->setTeam(team().team());
    changePoke(&team().team().poke(0));
    updatePoke();

    connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SLOT(changeTeamPoke(int)));
}

PokeBoxes::~PokeBoxes()
{
    delete ui;
}

void PokeBoxes::changePoke(PokeTeam *poke)
{
    this->m_poke = poke;
}

void PokeBoxes::updatePoke()
{
    ui->nickNameLabel->setText(poke().nickname());
    ui->speciesLabel->setText(PokemonInfo::Name(poke().num()));
    ui->pokemonSprite->setPixmap(poke().picture());
    ui->pokemonSprite->setFixedSize(poke().picture().size());
    ui->type1Label->setPixmap(Theme::TypePicture(PokemonInfo::Type1(poke().num(), poke().gen())));
    if(PokemonInfo::Type2(poke().num(), poke().gen()) != Type::Curse) {
        ui->type2Label->setVisible(true);
        ui->type2Label->setPixmap(Theme::TypePicture(PokemonInfo::Type2(poke().num(), poke().gen())));
    } else {
        ui->type2Label->setVisible(false);
    }
    ui->natureLabel->setText(QString("%1").arg(NatureInfo::Name(poke().nature())));
    ui->itemSprite->setPixmap(ItemInfo::Icon(poke().item()));
    ui->genderLabel->setPixmap(Theme::GenderPicture(poke().gender()));
    ui->levelLabel->setText(tr("Lv. %1").arg(poke().level()));
    QString movesInfo;
    for(int movesCount = 0; movesCount < 4; movesCount++) {
        if(movesCount == 4) {
            movesInfo.append(QString("%1").arg(MoveInfo::Name(poke().move(movesCount))));
        } else {
            movesInfo.append(QString("%1\n").arg(MoveInfo::Name(poke().move(movesCount))));
        }
    }
    ui->movesLabel->setText(movesInfo);
}

void PokeBoxes::changeTeamPoke(int index)
{
    changePoke(&team().team().poke(index));
    updatePoke();
}

PokemonItem::PokemonItem(PokeTeam *poke, PokeBox *box) : poke(NULL), m_Box(box)
{
    changePoke(poke);
    setFlags(QGraphicsItem::ItemIsMovable);
}

void PokemonItem::changePoke(PokeTeam *poke)
{
    delete this->poke;
    this->poke = poke;
    setPixmap(PokemonInfo::Icon(poke->num()));
}

void PokemonItem::setBox(PokeBox *newBox)
{
    m_Box = newBox;
}

PokemonItem::~PokemonItem()
{
    delete poke;
    poke = NULL;
}

void PokemonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
}

void PokemonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
}

void PokemonItem::startDrag()
{
}

PokeBox::PokeBox(QFrame *parent, int boxNum, const QString &file)
{
    setParent(parent);
    setScene(new QGraphicsScene(this));
    setSceneRect(0, 0, width() - 10, 160);
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
    //TODO
}

void PokeBox::addPokemonToBox(const PokeTeam &poke, int slot)
{
    if(isFull()) {
        QMessageBox::critical(0, QString(tr("Box") + " #" + this->getNum() + " - " + this->getBoxName()), QObject::tr("Could not add the Pokemon to the box, the box is full."), QMessageBox::Ok);
        return;
    }
    int spot = slot == -1 ? (m_Pokemons[currentPokemon] == NULL ? currentPokemon : freeSpot()) : slot;
    m_Pokemons[spot] = new PokemonItem(new PokeTeam(poke), this);

    addGraphicsItem(spot);
    changeCurrentSpot(spot);
}

PokeTeam *PokeBox::getCurrent()
{
    if(m_Pokemons[currentPokemon] == NULL) {
        QMessageBox::critical(0, QString(tr("Box") + " #" + this->getNum() + " - " + this->getBoxName()), QObject::tr("There's no Pokemon there."), QMessageBox::Ok);
        return NULL;
    }
    return m_Pokemons[currentPokemon]->poke;
}

void PokeBox::deleteCurrent()
{
    if(m_Pokemons[currentPokemon] == NULL) {
        QMessageBox::critical(0, QString(tr("Box") + " #" + this->getNum() + " - " + this->getBoxName()), QObject::tr("There's no Pokemon there."), QMessageBox::Ok);
        return;
    }
    scene()->removeItem(m_Pokemons[currentPokemon]);
    delete m_Pokemons[currentPokemon];
    m_Pokemons[currentPokemon] = NULL;
}

void PokeBox::changeCurrent(const PokeTeam &poke)
{
    if(m_Pokemons[currentPokemon] == NULL) {
        QMessageBox::critical(0, QString(tr("Box") + " #" + this->getNum() + " - " + this->getBoxName()), QObject::tr("There's no Pokemon there."), QMessageBox::Ok);
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
    return this->Number;
}

void PokeBox::setNum(int number)
{
    this->Number = number;
}

int PokeBox::getNumOf(const PokemonItem *item) const
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

PokemonItem *PokeBox::currentItem()
{
    if(currentPokemon == -1) {
        return NULL;
    } else {
        return m_Pokemons[currentPokemon];
    }
}

void PokeBox::mousePressEvent(QMouseEvent *event)
{
}

void PokeBox::dragEnterEvent(QDragEnterEvent *event)
{
}

void PokeBox::dropEvent(QDropEvent *event)
{
}

void PokeBox::dragMoveEvent(QDragMoveEvent *event)
{
}
