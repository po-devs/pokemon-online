#include "teambuilder.h"
#include "box.h"
//#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"

/*************************************************/
/************** TB_PokemonDetail *****************/
/*************************************************/

TB_PokemonDetail::TB_PokemonDetail()
{
    setFixedSize(215,225);

    QGridLayout *gl = new QGridLayout(this);
    gl->setMargin(3);

    /* Nick layout */
    QGridLayout *nicklayout = new QGridLayout();
    nicklayout->setSpacing(0);
    nicklayout->setMargin(0);
    gl->addLayout(nicklayout, 0,0);

    QLabel *whitePokeball = new QLabel();
    whitePokeball->setPixmap(QPixmap("db/Teambuilder/Box/Whiteball.png"));
    nicklayout->addWidget(whitePokeball,0,0);
    nicklayout->addWidget(m_name = new QLabel(),0,1);
    nicklayout->addWidget(m_nick = new QLabel(),1,0,1,2);
    m_name->setObjectName("NormalText");
    m_nick->setObjectName("SmallText");

    /* Level/Gender layout */
    QGridLayout *lvlayout = new QGridLayout();
    lvlayout->setSpacing(0);
    lvlayout->setMargin(0);
    gl->addLayout(lvlayout, 0,1);

    lvlayout->addWidget(m_num = new QLabel(),0,0,1,2, Qt::AlignRight);
    lvlayout->addWidget(m_gender = new QLabel(),1,0,1,1, Qt::AlignLeft);
    lvlayout->addWidget(m_level = new QLabel(),1,1,1,1,Qt::AlignLeft);
    lvlayout->setColumnStretch(1,100);
    m_num->setObjectName("BigText");
    m_level->setObjectName("SmallText");

    /* Avatar box! */
    gl->addWidget(m_pic = new AvatarBox(),1,0);

    /* Type / Nature / Item */
    QVBoxLayout *tnlayout = new QVBoxLayout();
    gl->addLayout(tnlayout,1,1);

    tnlayout->addStretch(100);
    QHBoxLayout *types = new QHBoxLayout();
    types->addWidget(m_type1 = new QLabel(),0,Qt::AlignLeft);
    types->addWidget(m_type2 = new QLabel(),100,Qt::AlignLeft);
    tnlayout->addLayout(types);
    tnlayout->addWidget(m_nature = new QLabel());
    m_nature->setObjectName("SmallText");
    QHBoxLayout *items = new QHBoxLayout();
    QLabel *item;
    items->addWidget(item = new QLabel(tr("Item: ")));
    items->addWidget(m_item=new QLabel(), 100, Qt::AlignLeft);
    tnlayout->addLayout(items);
    item->setObjectName("SmallText");
    tnlayout->addStretch(100);

    //Moves
    QGridLayout *moves = new QGridLayout();
    QLabel *lw;
    moves->addWidget(lw = new QLabel(tr("Moves:")),0,0,1,1,Qt::AlignLeft);
    for (int i = 0; i < 4; i++) {
        moves->addWidget(m_moves[i] = new QLabel(), 1+i,1,1,1,Qt::AlignLeft|Qt::AlignTop);
        m_moves[i]->setObjectName("SmallText");
    }
    lw->setObjectName("NormalText");
    moves->setColumnStretch(1,100);
    gl->addLayout(moves,2,0,1,2);

    gl->setRowStretch(2,200);
}

void TB_PokemonDetail::changePoke(PokeTeam *poke, int num)
{
    this->num = num;
    this->poke = poke;
}

void TB_PokemonDetail::updatePoke()
{
    m_name->setText(PokemonInfo::Name(poke->num()));
    m_num->setText(QString("[%1]").arg(num+1));
    m_nick->setText(poke->nickname());
    m_pic->changePic(poke->picture());
    m_type1->setPixmap(TypeInfo::Picture(PokemonInfo::Type1(poke->num())));
    m_type2->setPixmap(TypeInfo::Picture(PokemonInfo::Type2(poke->num())));
    m_type2->setVisible(PokemonInfo::Type2(poke->num()) != Type::Curse);
    m_nature->setText(tr("Nature: %1").arg(NatureInfo::Name(poke->nature())));
    m_item->setPixmap(ItemInfo::Icon(poke->item()));
    m_gender->setPixmap(GenderInfo::Picture(poke->gender(), false));
    m_level->setText(tr("Lv. %1").arg(poke->level()));

    for (int i = 0; i < 4; i++) {
        m_moves[i]->setText(QString("(%1) %2").arg(i+1).arg(MoveInfo::Name(poke->move(i))));
    }
}

/****************************************************************/
/************** PokemonBoxButton ********************************/
/****************************************************************/

QPixmap *PokemonBoxButton::theicon = NULL;
QPixmap *PokemonBoxButton::theglowedicon = NULL;

PokemonBoxButton::PokemonBoxButton(int num)
{
    if (theicon == NULL)
        theicon = new QPixmap("db/Teambuilder/Box/WhiteBall.png");
    if (theglowedicon == NULL)
        theglowedicon = new QPixmap("db/Teambuilder/Box/WhiteBallGlow.png");

    setText(tr("Pokémon &%1").arg(num+1));
    setIcon(*theicon);
    setCheckable(true);
}

/****************************************************************/
/*************** TB_PokemonButtons ******************************/
/****************************************************************/

TB_PokemonButtons::TB_PokemonButtons()
{
    QVBoxLayout *vl = new QVBoxLayout(this);
    QButtonGroup *thegroup = new QButtonGroup(this);

    setFixedSize(145,180);
    vl->setSpacing(3);
    vl->setMargin(5);

    for(int i = 0; i < 6; i++) {
        PokemonBoxButton *pokebutton;
        vl->addWidget(pokebutton = new PokemonBoxButton(i));

        thegroup->addButton(pokebutton,i);

        if (i==0)
            pokebutton->setChecked(true);

        buttons[i] = pokebutton;
    }
    connect(thegroup, SIGNAL(buttonClicked(int)), SIGNAL(buttonChecked(int)));
}

/****************************************************************/
/******************** TB_PokemonItem ****************************/
/****************************************************************/

TB_PokemonItem::TB_PokemonItem(PokeTeam *item) : poke(NULL)
{
    changePoke(item);
}

void TB_PokemonItem::changePoke(PokeTeam *poke)
{
    delete this->poke, this->poke = poke;
    setPixmap(PokemonInfo::Icon(poke->num()));
}

TB_PokemonItem::~TB_PokemonItem()
{
    delete poke, poke=NULL;
}

/****************************************************************/
/******************** TB_PokemonBox *****************************/
/****************************************************************/

QPixmap * PokemonBox::selBg = NULL;

PokemonBox::PokemonBox(int num) : num(num), currentPoke(0)
{
    if (selBg == NULL)
        selBg = new QPixmap("db/Teambuilder/Box/smallBox.png");

    pokemons.resize(30);

    setScene(new QGraphicsScene(this));
    setSceneRect(0,0,width()-10,160);
}

void PokemonBox::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    if (currentPoke != -1) {
        QPointF selBGPos = calculatePos(currentPoke, selBg->size());
        QRectF intersection = rect.intersect(QRectF(selBGPos, selBg->size()));
        QRectF srcRect = QRectF(std::max(qreal(0), intersection.x()-selBGPos.x()), std::max(qreal(0), intersection.y()-selBGPos.y()),
                                selBg->size().width(), selBg->size().height());
        painter->drawPixmap(intersection, *selBg, srcRect);
    }
}

bool PokemonBox::isFull() const {
    return !pokemons.contains(NULL);
}

int PokemonBox::freeSpot() const {
    for (int i = 0; ; i++)
        if (pokemons[i] == NULL)
            return i;
    //Never reached
}

bool PokemonBox::isEmpty() const {
    for (int i = 0; i < pokemons.size(); i++) {
        if (pokemons[i] != NULL)
            return false;
    }
    return true;
}

void PokemonBox::addPokemon(const PokeTeam &poke) throw(QString)
{
    if (isFull())
        throw tr("The box is full!");

    int spot = pokemons[currentPoke] == NULL ? currentPoke : freeSpot();
    pokemons[spot] = new TB_PokemonItem(new PokeTeam(poke));

    addGraphicsItem(spot);
    changeCurrentSpot(spot);
}

PokeTeam* PokemonBox::getCurrent() throw (QString)
{
    if (pokemons[currentPoke] == NULL)
        throw tr("There is no pokemon there!");

    return pokemons[currentPoke]->poke;
}

void PokemonBox::deleteCurrent() throw (QString)
{
    if (pokemons[currentPoke] == NULL)
        throw tr("There is no pokemon there!");

    scene()->removeItem(pokemons[currentPoke]);
    delete pokemons[currentPoke], pokemons[currentPoke] = NULL;
}

void PokemonBox::changeCurrent(const PokeTeam &poke) throw (QString)
{
    if (pokemons[currentPoke] == NULL)
        throw tr("There is no pokemon there!");

    pokemons[currentPoke]->changePoke(new PokeTeam(poke));
}

void PokemonBox::updateCurrentPoke()
{
    if (isEmpty())
        changeCurrentSpot(0);
    else {
        for (int i =0; i < pokemons.size(); i++) {
            if (currentPoke-i >= 0 && pokemons[currentPoke-i]!=NULL) {
                changeCurrentSpot(currentPoke-i);
                return;
            }
            if (currentPoke+i < pokemons.size() && pokemons[currentPoke+i]!=NULL) {
                changeCurrentSpot(currentPoke+i);
                return;
            }
        }
    }
}

TB_PokemonItem* PokemonBox::currentItem()
{
    if (currentPoke == -1)
        return NULL;
    else
        return pokemons[currentPoke];
}

void PokemonBox::addGraphicsItem(int spot)
{
    QPointF pos = calculatePos(spot);
    pokemons[spot]->setPos(pos);

    scene()->addItem(pokemons[spot]);
}

void PokemonBox::changeCurrentSpot(int newspot)
{
    if (newspot == currentPoke)
        return;

    /* You could test for -1s to optimize perfs */
    updateScene(QList<QRectF>() << QRectF(calculatePos(currentPoke, selBg->size()), selBg->size())
                                    << QRectF(calculatePos(newspot, selBg->size()), selBg->size()));

    currentPoke = newspot;
}

QPointF PokemonBox::calculatePos(int spot, const QSize &itemSize)
{
    QPointF pos;
    pos.setX((spot%10)*64 + 24 - itemSize.width()/2);
    pos.setY((spot/10)*50 + 24 - itemSize.height()/2);

    return pos;
}

void PokemonBox::mousePressEvent(QMouseEvent *event)
{
    int spot = calculateSpot(event->pos());

    if (spot != -1)
        changeCurrentSpot(spot);
}

int PokemonBox::calculateSpot(const QPoint &graphViewPos)
{
    QPointF pos = mapToScene(graphViewPos);

    int x,y;

    x = (pos.x()-24)/64 + 0.5;
    y = (pos.y()-24)/50 + 0.5;

    if (x < 10 && y < 3 && x >= 0 && y >= 0) {
        return y*10+x;
    } else {
        return -1;
    }
}

/****************************************************************/
/******************** TB_PokemonBoxes ***************************/
/****************************************************************/

TB_PokemonBoxes::TB_PokemonBoxes(TeamBuilder *parent) : QWidget(parent)
{
    m_team = parent->team();
    currentPoke = 0;
    QVBoxLayout * ml = new QVBoxLayout(this);
    QHBoxLayout *firstline = new QHBoxLayout();
    ml->addLayout(firstline);

    firstline->addWidget(m_details = new TB_PokemonDetail());
    firstline->addSpacing(20);
    firstline->addWidget(new TitledWidget(tr("Change Order"), m_buttons = new TB_PokemonButtons()),0,Qt::AlignVCenter);

    QVBoxLayout *thirdColumn = new QVBoxLayout();
    firstline->addLayout(thirdColumn);
    QPushButton *bstore, *bwithdraw, *bswitch, *bdelete;

    thirdColumn->addStretch(100);
    thirdColumn->addWidget(bstore = new QPushButton(tr("&Store")));
    thirdColumn->addWidget(bwithdraw = new QPushButton(tr("&Withdraw")));
    thirdColumn->addWidget(bswitch = new QPushButton(tr("&Switch")));
    thirdColumn->addWidget(bdelete = new QPushButton(tr("&Delete")));
    thirdColumn->addStretch(100);

    firstline->addStretch(100);

    QLabel *brock = new QLabel(this);
    brock->setPixmap(QPixmap("db/Teambuilder/Box/Brock.png"));
    brock->setFixedSize(brock->pixmap()->size());
    brock->move(QPoint(635,2));

    QLabel *mudkip = new QLabel(this);
    mudkip->setPixmap(QPixmap("db/Teambuilder/Box/Mudkip.png"));
    mudkip->setFixedSize(mudkip->pixmap()->size());
    mudkip->move(QPoint(572,188));

    bstore->setFixedWidth(132);


    QHBoxLayout *secondline = new QHBoxLayout();
    ml->addLayout(secondline);

    secondline->addWidget(parent->createButtonMenu(), 0, Qt::AlignBottom);
    secondline->addWidget(m_boxes = new QTabWidget(), 100);

    for (int i = 0; i < 6; i++) {
        m_boxes->addTab(boxes[i] = new PokemonBox(i), *PokemonBoxButton::theicon, tr("BOX &%1").arg(QChar('A'+i)));
    }

    m_details->changePoke(&m_team->poke(0),0);

    connect(m_buttons, SIGNAL(buttonChecked(int)), SLOT(changeCurrentTeamPokemon(int)));
    connect(bstore, SIGNAL(clicked()), SLOT(store()));
    connect(bwithdraw, SIGNAL(clicked()), SLOT(withdraw()));
    connect(bdelete, SIGNAL(clicked()), SLOT(deleteP()));
    connect(bswitch, SIGNAL(clicked()), SLOT(switchP()));
}

void TB_PokemonBoxes::changeCurrentTeamPokemon(int newpoke)
{
    currentPoke = newpoke;
    m_details->changePoke(&m_team->poke(newpoke), newpoke);
    m_details->updatePoke();
}

void TB_PokemonBoxes::store()
{
    try {
        currentBox()->addPokemon(m_team->poke(currentPoke));
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Full Box"), ex);
    }
}

void TB_PokemonBoxes::withdraw()
{
    try {
        *currentPokeTeam() = *currentBox()->getCurrent();
        updateSpot(currentPoke);
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Empty Box"), ex);
    }
}

PokeTeam *TB_PokemonBoxes::currentPokeTeam()
{
    return &m_team->poke(currentPoke);
}

void TB_PokemonBoxes::updateBox()
{
    for (int i =0; i< 6; i++) {
        updateSpot(i);
    }
}

void TB_PokemonBoxes::deleteP()
{
    try {
        currentBox()->deleteCurrent();
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box Empty"), ex);
    }
}

void TB_PokemonBoxes::switchP()
{
    try {
        PokeTeam *p = new PokeTeam(*currentBox()->getCurrent());
        currentBox()->changeCurrent(*currentPokeTeam());
        *currentPokeTeam() = *p;

        delete p;

        updateSpot(currentPoke);
    } catch(const QString &ex) {
        QMessageBox::information(this, tr("Box Empty"), ex);
    }
}

void TB_PokemonBoxes::updateSpot(int i)
{
    if (currentPoke == i)
        m_details->updatePoke();

    m_buttons->buttons[i]->setIcon(PokemonInfo::Icon(m_team->poke(i).num()));
}
