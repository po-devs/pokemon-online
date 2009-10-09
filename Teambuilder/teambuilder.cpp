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

    PkInfo = new PokemonInfo("db/");

    QGridLayout *layout = new QGridLayout(this);

    m_trainer = new QPushButton("&Trainer", this);
    m_trainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    layout->addWidget(m_trainer, 0, 0, 2, 1);

    for (int i = 0; i < 6; i++)
    {
	m_pokemon[i] = new QPushButton(QString("Pokémon &%1").arg(i+1));
	layout->addWidget(m_pokemon[i], int(i>=3), (i%3)+1);
    }

    m_body = new QStackedWidget(this);

    layout->addWidget(m_body,2,0,1,4);

    m_trainerBody = new QWidget();
    //m_body->addWidget(m_trainerBody);


    for (int i = 0; i < 6; i++)
    {
	m_pbody[i] = new TB_PokemonBody();
	m_body->addWidget(m_pbody[i]);
    }
}

TeamBuilder::~TeamBuilder()
{
    delete PkInfo;
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

    load_pokemons();

    /* first column, in the upper part */
    QVBoxLayout *first_column = new QVBoxLayout();
    layout->addLayout(first_column,0,0);

    first_column->addWidget(new QEntitled("Pokemon", pokechoice));
    first_column->addWidget(new QEntitled("Nickname", new QLineEdit()));

    QString items[9] = {"Big Root" ,"Blue Scarf","Bright Powder","Choice Band","Choice Scarf" ,"Choice Specs" ,"Destiny Knot","Expert Belt","Focus Band"};

    QComboBox *itemchoice = new QComboBox();
    for (int i = 0; i < 9; i++)
    {
	itemchoice->addItem(items[i]);
    }

    first_column->addWidget(new QEntitled("Item", itemchoice));

    /* second column, in the upper part */
    QVBoxLayout *second_column = new QVBoxLayout();
    layout->addLayout(second_column,0,1);

    QLabel *poke_image = new QLabel();
    poke_image->setPixmap(QPixmap("DPf.png"));

    second_column->addWidget(poke_image,0,Qt::AlignBottom|Qt::AlignHCenter);

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

    /* now the grand move list */
    QCompactTable *movechoice = new QCompactTable(5,7);
    movechoice->setSelectionBehavior(QAbstractItemView::SelectRows);
    movechoice->setSelectionMode(QAbstractItemView::SingleSelection);
    movechoice->setShowGrid(false);
    movechoice->verticalHeader()->hide();
    QStringList move_headers;
    move_headers << "Type" << "Name" << "Learning" << "PP" << "Pow" << "Acc" << "Category";
    movechoice->setHorizontalHeaderLabels(move_headers);
    movechoice->resizeRowsToContents();

    layout->addWidget(new QEntitled("Moves", movechoice), 1, 0, 1, 3);
    
    movechoice->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
}

void TB_PokemonBody::load_pokemons()
{
    pokechoice = new QCompactTable(PkInfo->NumberOfPokemons(),2);
    pokechoice->setSelectionBehavior(QAbstractItemView::SelectRows);
    pokechoice->setSelectionMode(QAbstractItemView::SingleSelection);
    pokechoice->setShowGrid(false);
    pokechoice->verticalHeader()->hide();
    pokechoice->horizontalHeader()->hide();
    pokechoice->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    pokechoice->resizeRowsToContents();

    /* Adding the poke names */
    for (int i = 0; i <= PkInfo->NumberOfPokemons(); i++)
    {
	QTableWidgetItem *item = new QTableWidgetItem(QString::number(i));
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	pokechoice->setItem(i, 0, item);

	item = new QTableWidgetItem(PkInfo->Name(i));
	item->setFlags(item->flags() ^ Qt::ItemIsEditable);
	pokechoice->setItem(i, 1, item);
    }
}

void TB_EVBar::add_bar(const QString &desc, int num, quint8 evs)
{
    int rowcount = rowCount();

    addWidget(new QLabel(desc), rowcount, 0, Qt::AlignLeft);
    addWidget(new QLabel(QString::number(num)), rowcount, 1, Qt::AlignLeft);
    addWidget(new QSlider(Qt::Horizontal), rowcount, 2);
    addWidget(new QLabel(QString::number(evs)), rowcount, 3, Qt::AlignLeft);
}
