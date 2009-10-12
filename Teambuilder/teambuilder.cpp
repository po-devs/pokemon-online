#include "teambuilder.h"

QCompactTable::QCompactTable(int row, int column)
	: QTableWidget(row, column)
{
}

int QCompactTable::sizeHintForRow(int row) const
{
    (void) row;
    return 0;
}

TeamBuilder::TeamBuilder(QWidget *parent)
    : QWidget(parent)
{
    resize(600, 600);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    PokemonInfo::init("db/");
    ItemInfo::init("db/");
    MoveInfo::init("db/");
    TypeInfo::init("db/");

    QGridLayout *layout = new QGridLayout(this);

    m_trainer = new QPushButton("&Trainer", this);
    m_trainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_trainer->setCheckable(true);
    m_trainer->setChecked(true);

    layout->addWidget(m_trainer, 0, 0, 2, 1);

    for (int i = 0; i < 6; i++)
    {
	m_pokemon[i] = new QPushButton(QString("Pokémon &%1").arg(i+1));
	m_pokemon[i]->setCheckable(true);
	layout->addWidget(m_pokemon[i], int(i>=3), (i%3)+1);
    }

    m_body = new QStackedWidget(this);

    layout->addWidget(m_body,2,0,1,4);

    m_trainerBody = new QWidget();
    m_body->addWidget(m_trainerBody);

    for (int i = 0; i < 6; i++)
    {
	m_pbody[i] = new TB_PokemonBody();
	m_pbody[i]->setPokeTeam(&team()->poke(i));
	m_body->addWidget(m_pbody[i]);
    }

    connectAll();
}

void TeamBuilder::connectAll()
{
    /* Connecting trainer/poke buttons and their associated zone */
    QSignalMapper *mapper = new QSignalMapper(this);

    for (int i = 0; i < 7; i++)
    {
	mapper->setMapping(at(i), i);
	connect(at(i), SIGNAL(pressed()), mapper, SLOT(map()));
    }

    connect(mapper, SIGNAL(mapped(int)), SLOT(changeBody(int)));
}

int TeamBuilder::currentZone() const
{
    return m_body->currentIndex();
}

QPushButton* TeamBuilder::at(int i)
{
    if (i == 0)
	return m_trainer;
    else
	return m_pokemon[i-1];
}

Team* TeamBuilder::team()
{
    return &m_team;
}

void TeamBuilder::changeBody(int i)
{
    if (i != currentZone())
    {
	/* Uncheck the previously checked button */
	at(currentZone())->setChecked(false);
	/* change the body to the one requested */
	m_body->setCurrentIndex(i);
    }
}

TeamBuilder::~TeamBuilder()
{
}

QEntitled::QEntitled(const QString &title, QWidget *widget)
{
    m_layout = new QVBoxLayout(this);

    /* The space is there for correct alignment of the title */
    m_title = new QLabel( title);
    m_layout->addWidget(m_title, 0, Qt::AlignBottom);
    if (widget)
	m_widget = widget;
    else
	m_widget = new QWidget();
    m_layout->addWidget(m_widget, 0, Qt::AlignTop);

    /* Makes the title/items stick together */
    m_layout->setSpacing(0);
}

void QEntitled::setWidget(QWidget *widget)
{
    m_layout->removeWidget(m_widget);
    m_widget = widget;
    m_layout->addWidget(m_widget, 0, Qt::AlignTop);
}

void QEntitled::setTitle(const QString &title)
{
    m_title->setText(title);
}

TB_PokemonBody::TB_PokemonBody()
{
    /* The layout of the whole body */
    QGridLayout *layout = new QGridLayout(this);

    initPokemons();

    /* first column, in the upper part */
    QVBoxLayout *first_column = new QVBoxLayout();
    layout->addLayout(first_column,0,0);

    first_column->addWidget(new QEntitled("Pokemon", pokechoice));
    first_column->addWidget(new QEntitled("Nickname", new QLineEdit()));

    initItems();

    first_column->addWidget(new QEntitled("Item", itemchoice));

    /* second column, in the upper part */
    QVBoxLayout *second_column = new QVBoxLayout();
    layout->addLayout(second_column,0,1);

    pokeimage = new QLabel();

    second_column->addWidget(pokeimage,0,Qt::AlignBottom|Qt::AlignHCenter);

    QHBoxLayout *gender_level = new QHBoxLayout;
    second_column->addLayout(gender_level);

    QLabel *gender_icon = new QLabel();
    gender_icon->setPixmap(QPixmap("Male.png"));
    QLabel *level = new QLabel(tr("Lv. 100"));

    gender_level->addWidget(gender_icon, 0, Qt::AlignLeft | Qt::AlignTop);
    gender_level->addWidget(level, 0, Qt::AlignRight | Qt::AlignTop);

    QString nature[4] = {"Hardy","Lonely","Brave","Adamant"};

    QComboBox *naturechoice = new QComboBox();
    for (int i = 0; i < 4; i++)
    {
	naturechoice->addItem(nature[i]);
    }

    second_column->addWidget(new QEntitled(tr("Nature"), naturechoice));
    second_column->addWidget(new QPushButton(tr("Advanced")));

    /* third and last column of the upper body */
    QVBoxLayout *third_column = new QVBoxLayout();
    layout->addLayout(third_column,0,2);

    QString hadsss[] = {"Hp:", "Att:", "Def:", "Speed:", "SpAtt:", "SpDef:"};
    TB_EVBar *evbar = new TB_EVBar();

    for (int i = 0; i < 6; i++)
    {
	evbar->add_bar(hadsss[i]);
    }
    third_column->addLayout(evbar);
    third_column->addWidget(new QSlider(Qt::Horizontal));

    initMoves();

    QGridLayout *mlayout = new QGridLayout();
    layout->addLayout(mlayout, 1,0,1,3);
    mlayout->addWidget(new QEntitled("Moves", movechoice), 0, 0, 1, 4);

    for (int i = 0; i < 4; i++)
    {
	mlayout->addWidget(m_moves[i],1,i);
    }

    connect(pokechoice, SIGNAL(cellDoubleClicked(int,int)), SLOT(setNum(int)));
}

void TB_PokemonBody::initPokemons()
{
    pokechoice = new QCompactTable(PokemonInfo::NumberOfPokemons(),2);
    pokechoice->setSelectionBehavior(QAbstractItemView::SelectRows);
    pokechoice->setSelectionMode(QAbstractItemView::SingleSelection);
    pokechoice->setShowGrid(false);
    pokechoice->verticalHeader()->hide();
    pokechoice->horizontalHeader()->hide();
    pokechoice->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    pokechoice->resizeRowsToContents();

    /* Adding the poke names */
    for (int i = 0; i < PokemonInfo::NumberOfPokemons(); i++)
    {
	QTableWidgetItem *item = new QTableWidgetItem(QString::number(i));
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	pokechoice->setItem(i, 0, item);

	item = new QTableWidgetItem(PokemonInfo::Name(i));
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	pokechoice->setItem(i, 1, item);
    }
}

void TB_PokemonBody::initMoves()
{
    /* now the grand move list */
    movechoice = new QCompactTable(0,7);
    movechoice->setSelectionBehavior(QAbstractItemView::SelectRows);
    movechoice->setSelectionMode(QAbstractItemView::SingleSelection);
    movechoice->setShowGrid(false);
    movechoice->verticalHeader()->hide();
    QStringList move_headers;
    move_headers << "Type" << "Name" << "Learning" << "PP" << "Pow" << "Acc" << "Category";
    movechoice->setHorizontalHeaderLabels(move_headers);
    movechoice->resizeRowsToContents();
    movechoice->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    /* the four move choice items */
    for (int i = 0; i < 4; i++)
    {
	m_moves[i] = new QLineEdit();
	QCompleter *completer = new QCompleter(m_moves[i]);
	completer->setModel(movechoice->model());
	completer->setCompletionColumn(Name);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCompletionMode(QCompleter::InlineCompletion);
	m_moves[i]->setCompleter(completer);
    }
}

void TB_PokemonBody::initItems()
{
    itemchoice = new QComboBox();

    QStringList itemList = ItemInfo::Names();
    qSort(itemList);

    itemchoice->addItems(itemList);
}

PokeTeam * TB_PokemonBody::poke()
{
    return m_poke;
}

void TB_PokemonBody::setPokeTeam(PokeTeam *new_poke)
{
    m_poke = new_poke;
}

void TB_PokemonBody::setNum(int pokenum)
{
    if (pokenum == poke()->num())
	return;

    poke()->setNum(pokenum);
    poke()->load();

    /* changes the move list */
    configureMoves();
    /* updates the pic */
    updateImage();
}

void TB_PokemonBody::updateImage()
{
    pokeimage->setPixmap(poke()->picture());
}

void TB_PokemonBody::configureMoves()
{
    QList<int> moves = poke()->moves();

    movechoice->setRowCount(moves.size());

    for (int i = 0; i < moves.size(); i++)
    {
	QTableWidgetItem *witem;
	
	witem = new QTableWidgetItem(TypeInfo::Name(MoveInfo::Type(moves[i])));
	witem->setForeground(QColor("white"));
	witem->setBackground(QColor(TypeInfo::Color(MoveInfo::Type(moves[i]))));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Type, witem);

	witem = new QTableWidgetItem(MoveInfo::Name(moves[i]));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Name, witem);

	witem = new QTableWidgetItem(QString::number(MoveInfo::PP(moves[i])));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, PP, witem);

	witem = new QTableWidgetItem(MoveInfo::AccS(moves[i]));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Acc, witem);

	witem = new QTableWidgetItem(MoveInfo::PowerS(moves[i]));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Pow, witem);
    }

    movechoice->sortItems(Name);
    movechoice->resizeRowsToContents();
}


void TB_EVBar::add_bar(const QString &desc, int num, quint8 evs)
{
    int rowcount = rowCount();

    addWidget(new QLabel(desc), rowcount, 0, Qt::AlignLeft);
    addWidget(new QLabel(QString::number(num)), rowcount, 1, Qt::AlignLeft);
    addWidget(new QSlider(Qt::Horizontal), rowcount, 2);
    addWidget(new QLabel(QString::number(evs)), rowcount, 3, Qt::AlignLeft);
}
