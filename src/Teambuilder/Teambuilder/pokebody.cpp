#include "pokebody.h"
#include "advanced.h"
#include "pokemovesmodel.h"
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "pokechoice.h"
#include "pokeballed.h"
#include "../../PokemonInfo/pokemoninfo.h"
#include "../../Utilities/otherwidgets.h"
#include "evmanager.h"
#include "theme.h"
#include "modelenum.h"

TB_PokemonBody::TB_PokemonBody(QWidget *upparent, PokeTeam *_poke, int num, int gen, QAbstractItemModel *itemModel,
                               QAbstractItemModel *pokeModel, QAbstractItemModel *natureModel)
{
    m_poke = _poke;
    m_num = num;
    this->gen = gen;

    movesModel = new PokeMovesModel(poke()->num(), gen, this);

    QGridLayout *ml = new QGridLayout(this);
    ml->setMargin(0);

    /*** Box 1 ***/
    QVBoxLayout *box1 = new QVBoxLayout();
    ml->addLayout(box1,0,0,2,1);

    box1->addWidget(new Pokeballed(m_pokeedit = new QLineEdit()));
    box1->addWidget(pokechoice = new TB_PokeChoice(pokeModel, true),100);

    box1->addWidget(new TitledWidget(tr("&Nickname"),m_nick = new QLineEdit()));
    m_nick->setValidator(new QNickValidator(m_nick));
    m_nick->setMaxLength(12);

    QWidget *itemw = new QWidget();
    QHBoxLayout *hitem = new QHBoxLayout(itemw);
    hitem->setMargin(0);
    hitem->addWidget(itemchoice = new QComboBox());
    hitem->addWidget(itemicon = new QLabel());
    itemicon->setFixedSize(24,24);
    box1->addWidget(itemlabel = new TitledWidget(tr("&Item"), itemw));

    itemchoice->setModel(itemModel);

    box1->addSpacerItem(new QSpacerItem(0,6));

    QPushButton  *importb;
    box1->addWidget(importb = new QPushButton(tr("&Import")));
    connect(importb, SIGNAL(clicked()), upparent, SLOT(importFromTxt()));

    QPushButton  *exportb;
    box1->addWidget(exportb = new QPushButton(tr("&Export")));
    connect(exportb, SIGNAL(clicked()), upparent, SLOT(exportToTxt()));

    /*** Box 2 ***/
    QHBoxLayout *box2 = new QHBoxLayout();
    ml->addLayout(box2,0,1);

    QVBoxLayout *box21 = new QVBoxLayout();
    QVBoxLayout *box22 = new QVBoxLayout();
    box2->addLayout(box21);
    box2->addLayout(box22);
    box21->setSpacing(3);
    box22->setSpacing(3);

    QLabel *lw;
    box21->addWidget(lw= new QLabel(tr("Pokémon %1").arg(num+1)), 10, Qt::AlignTop);
    lw->setObjectName("NormalText");
    box21->addWidget(pokeImage = new QToolButton(),0);
    pokeImage->setAutoRaise(true);
    QPushButton *advanced;
    box21->addWidget(advanced = new QPushButton(tr("&Advanced")), 10, Qt::AlignBottom);
    QMenu *m = new QMenu(advanced);
    QAction *a = m->addAction(tr("Side Window"), this, SLOT(goToAdvanced()));
    a->setProperty("window", 0);
    a = m->addAction(tr("New Window"), this, SLOT(goToAdvanced()));
    a->setProperty("window", 1);
    advanced->setMenu(m);

    pokeImage->setObjectName("PokemonPicture");

    box22->addWidget(pokename = new QLabel(),0,Qt::AlignTop);
    pokename->setObjectName("NormalText");
    QHBoxLayout *hlevel = new QHBoxLayout();
    hlevel->setMargin(0);
    box22->addLayout(hlevel);
    hlevel->addWidget(level = new QLabel());
    level->setObjectName("SmallText");
    hlevel->addWidget(genderIcon=new QLabel(),0,Qt::AlignRight);

    box22->addWidget(lw = new QLabel(tr("Type")));
    lw->setObjectName("NormalText");
    QHBoxLayout *htype = new QHBoxLayout();
    htype->setMargin(0);
    htype->addWidget(type1 = new QLabel());
    htype->addWidget(type2 = new QLabel());
    box22->addLayout(htype);

    box22->addWidget(nature = new QLabel(tr("N&ature")));
    box22->addWidget(naturechoice = new QComboBox());
    nature->setBuddy(naturechoice);
    nature->setObjectName("NormalText");
    naturechoice->setModel(natureModel);

    /*** Box 3 ***/
    evchoice = new TB_EVManager(poke());
    ml->addWidget(evchoice,0,2);


    /*** Box 4 ***/
    QGridLayout *box4 = new QGridLayout();
    ml->addLayout(box4, 1,1,1,2);
    ml->setRowStretch(1, 100);
    ml->setColumnStretch(2, 100);
    box4->setSpacing(0);

    movechoice = new MoveList(movesModel);
    box4->addWidget(movechoice, 0, 0, 1, 4);
    for (int i = 0; i < 4; i++)
    {
        box4->addWidget(m_moves[i] = new QLineEdit(),1,i);
    }

    /* Init & connect */

    initPokemons();
    initMoves();

    connect(evchoice, SIGNAL(EVChanged(int)), SIGNAL(EVChanged(int)));
    connect(naturechoice, SIGNAL(activated(int)), SLOT(setNature(int)));
    connect(naturechoice, SIGNAL(activated(int)), SIGNAL(natureChanged()));
    connect(m_nick, SIGNAL(textEdited(QString)), SLOT(setNick(QString)));
    connect(m_nick, SIGNAL(textChanged(QString)),this,SLOT(setNick(QString)));
    connect(evchoice, SIGNAL(natureChanged(int, int)),SLOT(editNature(int,int)));
    connect(itemchoice, SIGNAL(activated(QString)), SLOT(setItem(const QString &)));

    changeGeneration(poke()->gen());
}


void TB_PokemonBody::changeGeneration(int gen)
{
    this->gen = gen;

    poke()->setGen(gen);
    poke()->loadQuietly();

    updateNum();
    updateItem();

    if (gen == 1) {
        itemchoice->hide();
        genderIcon->hide();
        itemlabel->hide();
    } else {
        itemchoice->show();
        genderIcon->show();
        itemlabel->show();
    }

    if (gen <= 2) {
        naturechoice->hide();
        nature->hide();
    } else {
        naturechoice->show();
        nature->show();
    }

    evchoice->changeGen(gen);
}

void TB_PokemonBody::initPokemons(TB_PokemonBody *)
{
    QCompleter *completer = new QCompleter(m_pokeedit);
    completer->setModel(pokechoice->model());
    completer->setCompletionColumn(1);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    m_pokeedit->setCompleter(completer);

    connect(completer, SIGNAL(activated(QString)), this, SLOT(setPokeByNick()));
    connect(m_pokeedit, SIGNAL(returnPressed()), this, SLOT(setPokeByNick()));
    connect(pokechoice, SIGNAL(pokemonActivated(Pokemon::uniqueId)), SLOT(setNum(Pokemon::uniqueId)));
}

void TB_PokemonBody::initMoves()
{
    connect(movechoice, SIGNAL(activated(QModelIndex)), SLOT(moveEntered(QModelIndex)));
    connect(this, SIGNAL(moveChosen(int)), SLOT(setMove(int)));

    QSignalMapper *mapper = new QSignalMapper(this);

    /* the four move choice items */
    for (int i = 0; i < 4; i++)
    {
        QCompleter *completer = new QCompleter(m_moves[i]);
        completer->setModel(movechoice->model());
        completer->setCompletionColumn(PokeMovesModel::Name);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        m_moves[i]->setCompleter(completer);

        connect(m_moves[i], SIGNAL(customContextMenuRequested(QPoint)), m_moves[i], SLOT(selectAll()));

        mapper->setMapping(completer, i);
        mapper->setMapping(m_moves[i], i);
        connect(completer, SIGNAL(activated(QString)), mapper, SLOT(map()));
        connect(m_moves[i], SIGNAL(returnPressed()), mapper, SLOT(map()));
        connect(m_moves[i], SIGNAL(editingFinished()), mapper, SLOT(map()));
    }

    connect(mapper, SIGNAL(mapped(int)), SLOT(moveCellActivated(int)));
}

TB_PokemonBody::MoveList::MoveList(QAbstractItemModel *model)
{
    verticalHeader()->setDefaultSectionSize(22);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(false);
    verticalHeader()->hide();

    setIconSize(Theme::TypePicture(Type::Normal).size());
    QSortFilterProxyModel *filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(model);
    setModel(filterModel);
    //resizeRowsToContents();
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setResizeMode(PokeMovesModel::PP, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(PokeMovesModel::Pow, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(PokeMovesModel::Acc, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(PokeMovesModel::Name, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(PokeMovesModel::Name, 125);
    //horizontalHeader()->setResizeMode(PokeMovesModel::Type, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(PokeMovesModel::Type, 54);
    setSortingEnabled(true);
    connect(horizontalHeader(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(sortByColumn(int)));
}

void TB_PokemonBody::connectWithAdvanced(TB_Advanced *ptr)
{
    connect(ptr, SIGNAL(abilityChanged()), this, SLOT(updateAbility()));
    connect(ptr, SIGNAL(levelChanged()), this, SLOT(updateLevel()));
    connect(ptr, SIGNAL(imageChanged()), this, SLOT(updateImage()));
    connect(ptr, SIGNAL(genderChanged()), this, SLOT(updateGender()));
    connect(ptr, SIGNAL(genderChanged()), this, SLOT(updateImage()));
    connect(ptr, SIGNAL(statChanged()), this, SLOT(updateEVs()));
    connect(ptr, SIGNAL(pokeFormeChanged(Pokemon::uniqueId)), this, SLOT(changeForme(Pokemon::uniqueId)), Qt::QueuedConnection);
    connect(this, SIGNAL(EVChanged(int)), ptr, SLOT(updateStat(int)));
    connect(this, SIGNAL(natureChanged()), ptr, SLOT(updateStats()));
    connect(this, SIGNAL(pokeImageChanged()), ptr, SLOT(updatePokeImage()));
    connect(ptr, SIGNAL(levelChanged()), this, SIGNAL(levelChanged()));
}

void TB_PokemonBody::moveCellActivated(int cell)
{
    int movenum = MoveInfo::Number(m_moves[cell]->text());
    setMove(movenum, cell);
}

void TB_PokemonBody::goToAdvanced()
{
    emit advanced(num(), sender()->property("window").toInt());
}

void TB_PokemonBody::setNum(Pokemon::uniqueId pokenum)
{
    setNum(pokenum, true);
}

void TB_PokemonBody::setNum(Pokemon::uniqueId pokenum, bool resetEverything)
{
    if (pokenum == poke()->num())
        return;

    if (resetEverything) {
        poke()->reset();
    }

    poke()->setNum(pokenum);
    poke()->load();
    poke()->runCheck();

    updateNum();
}

void TB_PokemonBody::setPokeByNick()
{
    Pokemon::uniqueId number = PokemonInfo::Number(m_pokeedit->text());

    if (number != 0) {
        setNum(number);
    }
}

void TB_PokemonBody::updateLevel()
{
    level->setText(tr("Lv. %1").arg(poke()->level()));
}

void TB_PokemonBody::updateNum()
{
    movesModel->setPokemon(poke()->num(), gen);

    updateMoves();
    updateLevel();
    updateImage();
    updateEVs();
    updateGender();
    updateNature();
    updateItem();
    updateNickname();
    updatePokeChoice();
    updateTypes();
    pokename->setText(PokemonInfo::Name(poke()->num()));

    emit pokeChanged(poke()->num());
}

void TB_PokemonBody::updatePokeChoice()
{
    Pokemon::uniqueId original = PokemonInfo::OriginalForme(poke()->num());
    m_pokeedit->setText(PokemonInfo::Name(original));
    pokechoice->setCurrentIndex(pokechoice->currentIndex().sibling(original.pokenum, 1));
    pokechoice->scrollTo(pokechoice->currentIndex(), QAbstractItemView::PositionAtCenter);
}

void TB_PokemonBody::updateNickname()
{
    m_nick->setText(poke()->nickname());
}

void TB_PokemonBody::updateTypes()
{
    int ttype1 = PokemonInfo::Type1(poke()->num(), poke()->gen());
    int ttype2 = PokemonInfo::Type2(poke()->num(), poke()->gen());

    type1->setPixmap(Theme::TypePicture(ttype1));
    type2->setPixmap(Theme::TypePicture(ttype2));

    if (ttype2 == Pokemon::Curse)  {
        type2->hide();
    } else {
        type2->show();
    }
}

void TB_PokemonBody::updateItem()
{
    QString item = ItemInfo::Name(poke()->item());
    int low = 0;
    int high = itemchoice->count();

    while (low != high) {
        if (low + 1 == high) {
            if (itemchoice->itemText(low) == item)
                itemchoice->setCurrentIndex(low);
            else if (itemchoice->itemText(high) == item)
                itemchoice->setCurrentIndex(high);
            break;
        }
        int mid = low + (high-low)/2;
        int x = itemchoice->itemText(mid).compare(item);
        if (x == 0) {
            itemchoice->setCurrentIndex(mid);
            break;
        } else if (x > 0){
            high = mid;
        } else {
            low = mid;
        }
    }

    itemicon->setPixmap(ItemInfo::Icon(poke()->item()));
}

void TB_PokemonBody::updateNature()
{
    naturechoice->setCurrentIndex(poke()->nature());
    evchoice->updateNatureButtons();
}

void TB_PokemonBody::updateEVs()
{
    evchoice->updateEVs();
}

void TB_PokemonBody::changeForme(Pokemon::uniqueId pokenum)
{
    setNum(pokenum, false);
}

void TB_PokemonBody::updateImage()
{
    QPixmap picture = poke()->picture();
    pokeImage->setIconSize(picture.size());
    pokeImage->setIcon(picture);
}

void TB_PokemonBody::updateGender()
{
    /* Gender may ruin move combinations, for example with dream world moves */
    poke()->runCheck();
    updateMoves();
    genderIcon->setPixmap(Theme::GenderPicture(poke()->gender(), Theme::TeamBuilderM));
}

void TB_PokemonBody::updateAbility()
{
    /* Abilities may ruin move combinations, for example dream world abilities (implemented)
       or 4th gen abilities with 3rd gen moves on first stage evos (non implemented) */
    poke()->runCheck();
    updateMoves();
}

void TB_PokemonBody::updateMoves()
{
    for (int i = 0; i < 4; i++)
    {
        m_moves[i]->setText(MoveInfo::Name(poke()->move(i)));
    }
}

PokeTeam * TB_PokemonBody::poke()
{
    return m_poke;
}

void TB_PokemonBody::setNick(const QString &nick)
{
    poke()->nickname() = nick;
    emit nicknameChanged(nick);
}

void TB_PokemonBody::setMove(int movenum, int moveslot)
{
    try {
        poke()->setMove(movenum, moveslot,true);
    }
    catch (QString &expr)
    {
    QMessageBox::critical(this, tr("Error"), expr);
    /* Restoring previous move */
    m_moves[moveslot]->setText(MoveInfo::Name(poke()->move(moveslot)));

        return;
    }
}

void TB_PokemonBody::setMove(int movenum)
{
    try {
        int slot = poke()->addMove(movenum,true);
    m_moves[slot]->setText(MoveInfo::Name(movenum));

        if (movenum == Move::Return) {
            poke()->happiness() = 255;
        } else if (movenum == Move::Frustration) {
            poke()->happiness() = 0;
        }
    } catch (QString &expr)
    {
    QMessageBox::critical(this, tr("Error"), expr);
    }
}

void TB_PokemonBody::moveEntered(const QModelIndex &index)
{
    emit moveChosen(index.data(CustomModel::MovenumRole).toInt());
}

void TB_PokemonBody::setItem(const QString &item)
{
    int it = ItemInfo::Number(item);

    if (PokemonInfo::OriginalForme(poke()->num()) == Pokemon::Arceus) {
        int type = 0;
        if (ItemInfo::isPlate(it)) {
            type = ItemInfo::PlateType(it);
        }

        if (type != poke()->num().subnum) {
            changeForme(Pokemon::uniqueId(poke()->num().pokenum, type));
        }
    }
    if (it == Item::GriseousOrb && poke()->num().pokenum != Pokemon::Giratina && gen <= 4)
        poke()->item() = 0;
    else {
        poke()->item() = ItemInfo::Number(item);
        if (poke()->item() == Item::GriseousOrb) {
            if (poke()->num() == Pokemon::Giratina)
                changeForme(Pokemon::Giratina_O);
        } else if (poke()->num() == Pokemon::Giratina_O) {
            changeForme(Pokemon::Giratina);
        }
    }

    updateItem();
    emit itemChanged(it);
}

void TB_PokemonBody::setNature(int nature)
{
    poke()->nature() = nature;
    /* As the nature has an influence over the stats, we update them */
    evchoice->updateEVs();
    evchoice->updateNatureButtons();
}

void TB_PokemonBody::editNature(int up, int down)
{
    setNature(NatureInfo::NatureOf(up,down));
    emit natureChanged();
    updateNature();
}

