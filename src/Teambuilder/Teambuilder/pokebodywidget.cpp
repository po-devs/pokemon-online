#include "pokebodywidget.h"

#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QCompleter>
#include <QGridLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "pokeballed.h"
#include "../../Utilities/otherwidgets.h"
#include "pokemovesmodel.h"
#include "../pokechoice.h"
#include "evmanager.h"
#include "../theme.h"

PokeBodyWidget::PokeBodyWidget(QWidget *upparent, int gen, QAbstractItemModel *itemModel, QAbstractItemModel *pokeModel, QAbstractItemModel *natureModel)
{
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

    box21->addWidget(num = new QLabel(), 10, Qt::AlignTop);
    num->setObjectName("NormalText");
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

    QLabel *lw;
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
    evchoice = new TB_EVManager();
    ml->addWidget(evchoice,0,2);

    /*** Box 4 ***/
    QGridLayout *box4 = new QGridLayout();
    ml->addLayout(box4, 1,1,1,2);
    ml->setRowStretch(1, 100);
    ml->setColumnStretch(2, 100);
    box4->setSpacing(0);

    movechoice = new MoveList();
    box4->addWidget(movechoice, 0, 0, 1, 4);
    movechoice->sortByColumn(1);
    for (int i = 0; i < 4; i++)
    {
        box4->addWidget(m_moves[i] = new QLineEdit(),1,i);
    }

    /* Init & connect */

    initPokemons();
    initMoves();

    connect(naturechoice, SIGNAL(activated(int)), SIGNAL(natureChanged(int)));
    connect(naturechoice, SIGNAL(activated(int)),evchoice, SLOT(updateNatureButtons()));
    connect(m_nick, SIGNAL(textEdited(QString)), SIGNAL(nickChosen(QString)));
    connect(m_nick, SIGNAL(textChanged(QString)), SIGNAL(nickChosen(QString)));
    connect(evchoice, SIGNAL(natureChanged(int, int)),SLOT(setNature(int,int)));
    connect(evchoice, SIGNAL(EVChanged(int)), SIGNAL(EVChanged(int)));
    connect(itemchoice, SIGNAL(activated(QString)), SLOT(setItem(const QString &)));

    changeGen(gen);
}

void PokeBodyWidget::goToAdvanced()
{
    emit advanceMenuOpen(sender()->property("window").toBool());
}

void PokeBodyWidget::setWidgetNum(int num)
{
    this->num->setText(tr("Pokémon %1").arg(num+1));
    setProperty("num", num);
}

void PokeBodyWidget::updateEVs()
{
    evchoice->updateEVs();
}

void PokeBodyWidget::initPokemons()
{
    QCompleter *completer = new QCompleter(m_pokeedit);
    completer->setModel(pokechoice->model());
    completer->setCompletionColumn(1);
    completer->setCompletionRole(Qt::DisplayRole);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    m_pokeedit->setCompleter(completer);

    connect(completer, SIGNAL(activated(QString)), this, SLOT(pokemonTextTriggered()));
    connect(m_pokeedit, SIGNAL(returnPressed()), this, SLOT(pokemonTextTriggered()));
    connect(pokechoice, SIGNAL(pokemonActivated(Pokemon::uniqueId)), SIGNAL(pokemonChosen(Pokemon::uniqueId)));
}

void PokeBodyWidget::initMoves()
{
    connect(movechoice, SIGNAL(activated(QModelIndex)), SLOT(moveEntered(QModelIndex)));

    QSignalMapper *mapper = new QSignalMapper(this);

    /* the four move choice items */
    for (int i = 0; i < 4; i++)
    {
        QCompleter *completer = new QCompleter(m_moves[i]);
        completer->setModel(movechoice->model());
        completer->setCompletionColumn(PokeMovesModel::Name);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setCompletionRole(Qt::DisplayRole);
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

void PokeBodyWidget::pokemonTextTriggered()
{
    Pokemon::uniqueId number = PokemonInfo::Number(m_pokeedit->text());

    if (number != 0) {
        emit pokemonChosen(number);
    }
}

void PokeBodyWidget::setMovesModel(QAbstractItemModel *model)
{
    movechoice->setModel(model);
    for (int i = 0; i < 4; i++) {
        m_moves[i]->completer()->setModel(model);
    }
}

void PokeBodyWidget::setItem(const QString &item)
{
    int num = ItemInfo::Number(item);
    itemicon->setPixmap(ItemInfo::Icon(num));

    emit itemChanged(num);
}

void PokeBodyWidget::setItem(int itemnum)
{
    QString item = ItemInfo::Name(itemnum);

    /* Searches for the index of the item in the combobox */
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

        itemicon->setPixmap(ItemInfo::Icon(itemnum));
}

void PokeBodyWidget::setNature(int plus, int minus)
{
    int nature = NatureInfo::NatureOf(plus, minus);
    emit natureChanged(nature);
    setNature(nature);
}

void PokeBodyWidget::setNature(int nature)
{
    naturechoice->setCurrentIndex(nature);
    evchoice->updateNatureButtons();
    evchoice->updateEVs();
}

void PokeBodyWidget::moveEntered(const QModelIndex &index)
{
    emit moveChosen(index.data(CustomModel::MovenumRole).toInt());
}

void PokeBodyWidget::moveCellActivated(int index)
{
    emit moveChosen(index, MoveInfo::Number(m_moves[index]->text()));
}

void PokeBodyWidget::changeGen(int gen)
{
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

PokeBodyWidget::MoveList::MoveList(QAbstractItemModel *model)
{
    if (!model) {
        model = new PokeMovesModel(Pokemon::NoPoke, GEN_MAX, this);
    }

    verticalHeader()->setDefaultSectionSize(22);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(false);
    verticalHeader()->hide();

    setIconSize(Theme::TypePicture(Type::Normal).size());
    filterModel = new QSortFilterProxyModel(this);
    filterModel->setSourceModel(model);
    QTableView::setModel(filterModel);
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

void PokeBodyWidget::MoveList::setModel(QAbstractItemModel *model)
{
    filterModel->setSourceModel(model);
}

void PokeBodyWidget::loadPokemon(PokeTeam &poke)
{
    changeGen(poke.gen().num);

    evchoice->setPokemon(&poke);
    setNature(poke.nature());
    setItem(poke.item());
    setPicture(poke.picture());
    setNum(poke.num());
    setTypes(poke.type1(), poke.type2());
    setLevel(poke.level());
    setGender(poke.gender());

    for (int i = 0; i < 4; i++) {
        setMove(i, poke.move(i));
    }

    m_nick->setText(poke.nickname().isEmpty() ? PokemonInfo::Name(poke.num()) : poke.nickname());
}

void PokeBodyWidget::setPicture(const QPixmap &picture)
{
    pokeImage->setIconSize(picture.size());
    pokeImage->setIcon(picture);
}

void PokeBodyWidget::setNum(const Pokemon::uniqueId &num)
{
    pokename->setText(PokemonInfo::Name(num));
    m_pokeedit->setText(PokemonInfo::Name(num));

    pokechoice->scrollTo(pokechoice->model()->index(num.pokenum, 1));
}

void PokeBodyWidget::setTypes(int _type1, int _type2)
{
    type1->setPixmap(Theme::TypePicture(_type1));
    type2->setPixmap(Theme::TypePicture(_type2));

    type2->setVisible(_type2 != Type::Curse);
}

void PokeBodyWidget::setLevel(int level)
{
    this->level->setText(tr("Lv. %1").arg(level));
}

void PokeBodyWidget::setGender(int gender)
{
    this->genderIcon->setPixmap(Theme::GenderPicture(gender));
}

void PokeBodyWidget::setMove(int index, int num)
{
    m_moves[index]->setText(MoveInfo::Name(num));
}
