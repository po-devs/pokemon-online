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
    types->addWidget(m_type1 = new QLabel());
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

    connect(this, SIGNAL(toggled(bool)), SLOT(p_toggled(bool)));
}

void PokemonBoxButton::p_toggled(bool b)
{
    setIcon(b?*theglowedicon:*theicon);
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
    }
    connect(thegroup, SIGNAL(buttonClicked(int)), SIGNAL(buttonChecked(int)));
}

/****************************************************************/
/******************** TB_PokemonItem ****************************/
/****************************************************************/

TB_PokemonItem::TB_PokemonItem(PokeTeam *item) : poke(item)
{
    setPixmap(PokemonInfo::Icon(poke->num()));
}

TB_PokemonItem::~TB_PokemonItem()
{
    delete poke, poke=NULL;
}

/****************************************************************/
/******************** TB_PokemonBox *****************************/
/****************************************************************/

PokemonBox::PokemonBox(int num) : num(num)
{
    pokemons.resize(24);
    setScene(new QGraphicsScene(this));
}

bool PokemonBox::isFull() const {
    return freeSpot() == pokemons.size();
}

int PokemonBox::freeSpot() const {
    for (int i = 0; ; i++)
        if (pokemons[i] == NULL)
            return i;
    //Never reached
}

void PokemonBox::addPokemon(const PokeTeam &poke) throw(QString)
{

}

/****************************************************************/
/******************** TB_PokemonBoxes ***************************/
/****************************************************************/

TB_PokemonBoxes::TB_PokemonBoxes(TeamBuilder *parent) : QWidget(parent)
{
    m_team = parent->team();
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
        m_boxes->addTab(new QGraphicsView(), *PokemonBoxButton::theicon, tr("BOX &%1").arg(QChar('A'+i)));
    }

    m_details->changePoke(&m_team->poke(0),0);

    connect(m_buttons, SIGNAL(buttonChecked(int)), SLOT(changePokemon(int)));
}

void TB_PokemonBoxes::changePokemon(int newpoke)
{
    m_details->changePoke(&m_team->poke(newpoke), newpoke);
    m_details->updatePoke();
}

void TB_PokemonBoxes::updateBox()
{
    m_details->updatePoke();
}
