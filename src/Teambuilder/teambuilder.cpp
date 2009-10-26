#include "teambuilder.h"
#include "advanced.h"
#include <QMenu>
#include <QAction>

template <class T, class U>
QList<QPair<typename T::value_type, U> > map_container_with_value(T container, const U & value)
{
    QList<QPair<typename T::value_type, U> > ret;

    foreach(typename T::value_type val, container)
	ret << (QPair<typename T::value_type, U>(val, value));

    return ret;
}

QCompactTable::QCompactTable(int row, int column)
	: QTableWidget(row, column)
{
}

int QCompactTable::sizeHintForRow(int row) const
{
    (void) row;
    return 0;
}

TeamBuilder::TeamBuilder(QWidget *parent):QWidget(parent),menuBar(this)
{
    resize(600, 600);

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    PokemonInfo::init("db/");
    ItemInfo::init("db/");
    MoveInfo::init("db/");
    TypeInfo::init("db/");
    NatureInfo::init("db/");
    CategoryInfo::init("db/");
    AbilityInfo::init("db/");
    GenderInfo::init("db/");
    HiddenPowerInfo::init("db/");

    QGridLayout *layout = new QGridLayout(this);

    //creation des menus
    menuFichier = menuBar.addMenu("&Fichier");
    actionCharger = menuFichier->addAction(tr("Charger une Team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_C);
    actionSave = menuFichier->addAction(tr("Sauvegarder la Team"),this,SLOT(saveTeam()),Qt::CTRL+Qt::Key_S);
    actionQuitter = menuFichier->addAction("Quitter",qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);
    layout->addWidget(&menuBar,0,0,Qt::AlignTop);

    m_trainer = new QPushButton("&Trainer", this);
    m_trainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_trainer->setCheckable(true);
    m_trainer->setChecked(true);

    layout->addWidget(m_trainer, 1, 0, 2, 1);

    for (int i = 0; i < 6; i++)
    {
	m_pokemon[i] = new QPushButton(QString("Pokémon &%1").arg(i+1));
	m_pokemon[i]->setCheckable(true);
        if(i<3)
        {
            layout->addWidget(m_pokemon[i],1,(i%3)+1);
        }
        else
        {
            layout->addWidget(m_pokemon[i],2,(i%3)+1);
        }
    }

    m_body = new QStackedWidget(this);

    layout->addWidget(m_body,3,0,1,4);

    m_trainerBody = new QWidget();
    m_body->addWidget(m_trainerBody);

    for (int i = 0; i < 6; i++)
    {
	m_pbody[i] = new TB_PokemonBody(&team()->poke(i));
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

//slot private
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

void TeamBuilder::saveTeam()
{
    QString emplacement = QFileDialog::getSaveFileName(0,tr("Sauvegarde de la Team"),tr("../../bin/Team"));
    if(emplacement.isEmpty())
    {
        emplacement = qApp->applicationDirPath()+"/"+tr("Team");
    }
    if(emplacement == qApp->applicationDirPath()+"/Team")
    {
        QDir dossier(emplacement);
        if(!dossier.exists())
        {
            if(!dossier.mkdir(emplacement))
            {
                QMessageBox::warning(0,tr("Erreur Creation dossier Team"),tr("Impossible de creer le Dossier ")+dossier.path());
                return;
            }
        }
        emplacement+="/dresseur.tp";
    }
    QFile fichier(emplacement);
    if(!fichier.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(0, tr("Erreur Creation fichier Team"),tr("Impossible de creer le fichier ")+fichier.fileName());
        return;
    }
    QDataStream out(&fichier);
    out << this->team();
}

void TeamBuilder::loadTeam()
{
    QString emplacementTeam = QFileDialog::getOpenFileName(0,tr("Charger une Team"),tr("../../bin/Team"),tr("Team(*tp)"));
    QFile fichier(emplacementTeam);
    if(!fichier.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(0,tr("Erreur chargement Team"),tr("Impossible de charger la team ")+fichier.fileName());
        return;
    }
    QDataStream in(&fichier);

    in >> (*team());
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
    m_title->setBuddy(m_widget);

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

TB_PokemonBody::TB_PokemonBody(PokeTeam *_poke)
	: m_adv(NULL)
{
    m_poke = _poke;

    /* The layout of the whole body */
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);

    initPokemons();

    /* first column, in the upper part */
    QVBoxLayout *first_column = new QVBoxLayout();
    layout->addLayout(first_column,0,0);

    first_column->addWidget(new QEntitled("&Pokemon", pokechoice));
    first_column->addWidget(new QEntitled("&Nickname", new QLineEdit()));

    initItems();

    first_column->addWidget(new QEntitled("&Item", itemchoice));

    /* second column, in the upper part */
    QVBoxLayout *second_column = new QVBoxLayout();
    layout->addLayout(second_column,0,1);

    pokeImage = new QLabel();
    second_column->addWidget(pokeImage,0,Qt::AlignBottom|Qt::AlignHCenter);
    updateImage();

    QHBoxLayout *gender_level = new QHBoxLayout;
    second_column->addLayout(gender_level);

    genderIcon = new QLabel();
    gender_level->addWidget(genderIcon, 0, Qt::AlignCenter | Qt::AlignTop);
    updateGender();

    level = new QLabel();
    updateLevel();

    gender_level->addWidget(level, 0, Qt::AlignLeft | Qt::AlignTop);

    naturechoice = new QComboBox();
    for (int i = 0; i < NatureInfo::NumberOfNatures(); i++)
    {
	naturechoice->addItem(NatureInfo::Name(i));
    }
    connect(naturechoice, SIGNAL(activated(int)), SLOT(setNature(int)));

    second_column->addWidget(new QEntitled(tr("&Nature"), naturechoice));

    QPushButton *advanced = new QPushButton(tr("&Advanced"));
    advanced->setShortcut(Qt::Key_A);
    second_column->addWidget(advanced);
    connect(advanced, SIGNAL(pressed()), SLOT(goToAdvanced()));

    /* third and last column of the upper body */
    QVBoxLayout *third_column = new QVBoxLayout();
    layout->addLayout(third_column,0,2);

    evchoice = new TB_EVManager(poke());

    third_column->addLayout(evchoice);

    initMoves();

    QGridLayout *mlayout = new QGridLayout();
    layout->addLayout(mlayout, 1,0,1,3);
    mlayout->addWidget(new QEntitled("&Moves", movechoice), 0, 0, 1, 4);

    for (int i = 0; i < 4; i++)
    {
	mlayout->addWidget(m_moves[i],1,i);
    }
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

    pokechoice->resizeRowsToContents();

    connect(pokechoice, SIGNAL(cellActivated(int,int)), SLOT(setNum(int)));
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
    movechoice->horizontalHeader()->setStretchLastSection(true);
    movechoice->horizontalHeader()->setResizeMode(PP, QHeaderView::ResizeToContents);
    movechoice->horizontalHeader()->setResizeMode(Pow, QHeaderView::ResizeToContents);
    movechoice->horizontalHeader()->setResizeMode(Acc, QHeaderView::ResizeToContents);
    movechoice->setMinimumHeight(200);
    movechoice->setMidLineWidth(0);

    connect(movechoice, SIGNAL(cellActivated(int,int)), SLOT(moveEntered(int)));
    connect(this, SIGNAL(moveChosen(int)), SLOT(setMove(int)));

    QSignalMapper *mapper = new QSignalMapper(this);

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

	connect(m_moves[i], SIGNAL(customContextMenuRequested(QPoint)), m_moves[i], SLOT(selectAll()));

	mapper->setMapping(completer, i);
	mapper->setMapping(m_moves[i], i);
	connect(completer, SIGNAL(activated(QString)), mapper, SLOT(map()));
	connect(m_moves[i], SIGNAL(editingFinished()), mapper, SLOT(map()));
    }

    connect(mapper, SIGNAL(mapped(int)), SLOT(moveCellActivated(int)));
}

void TB_PokemonBody::moveCellActivated(int cell)
{
    int movenum = MoveInfo::Number(m_moves[cell]->text());
    setMove(movenum, cell);
}

void TB_PokemonBody::goToAdvanced()
{
    if (poke()->num() != 0)
    {
        if (advancedOpen())
        {
	    /* we show the user where the advanced window is */
	    advanced()->activateWindow();
	    advanced()->raise();
	    return;
	}

	m_adv = new TB_Advanced(poke());
	advanced()->show();
	advanced()->setAttribute(Qt::WA_DeleteOnClose, true);

	connect(this, SIGNAL(pokeChanged(int)), advanced(), SLOT(close()));
	connect(this, SIGNAL(destroyed()), advanced(), SLOT(close()));
	connect(advanced(), SIGNAL(destroyed()), SLOT(updateAdvanced()));
	connect(advanced(), SIGNAL(destroyed()), SLOT(setAdvancedOpenToFalse()));
    }
}

void TB_PokemonBody::updateLevel()
{
    level->setText(tr("Lv. %1").arg(poke()->level()));
}

void TB_PokemonBody::setAdvancedOpenToFalse()
{
    m_adv=NULL;
}

bool TB_PokemonBody::advancedOpen()
{
    return m_adv != NULL;
}

TB_Advanced * TB_PokemonBody::advanced()
{
    return m_adv;
}

void TB_PokemonBody::updateAdvanced()
{
    //in case DVs are changed
    evchoice->updateEVs();
    //in case shiny is changed;
    updateImage();
    //in case the gender is changed
    updateGender();
    //in case the level is changed
    updateLevel();
}

void TB_PokemonBody::updateGender()
{
    genderIcon->setPixmap(GenderInfo::Picture(poke()->gender()));
}

void TB_PokemonBody::initItems()
{
    itemchoice = new QComboBox();

    QStringList itemList = ItemInfo::Names();
    qSort(itemList);

    itemchoice->addItems(itemList);

    connect(itemchoice, SIGNAL(activated(QString)), SLOT(setItem(const QString &)));
}

PokeTeam * TB_PokemonBody::poke()
{
    return m_poke;
}

void TB_PokemonBody::setMove(int movenum, int moveslot)
{
    try {
	poke()->setMove(movenum, moveslot);
    }
    catch (QString &expr)
    {
	QMessageBox::critical(this, tr("Error"), expr);
	/* Restoring previous move */
	m_moves[moveslot]->setText(MoveInfo::Name(poke()->move(moveslot)));
    }
}

void TB_PokemonBody::setMove(int movenum)
{
    try {
	int slot = poke()->addMove(movenum);
	m_moves[slot]->setText(MoveInfo::Name(movenum));
    } catch (QString &expr)
    {
	QMessageBox::critical(this, tr("Error"), expr);
    }
}

void TB_PokemonBody::moveEntered(int row)
{
    emit moveChosen(MoveInfo::Number(movechoice->item(row, Name)->text()));
}

void TB_PokemonBody::setNum(int pokenum)
{
    if (pokenum == poke()->num())
	return;

    poke()->reset();
    poke()->setNum(pokenum);
    poke()->load();
    evchoice->updateEVs();

    /* changes the move list */
    configureMoves();
    /* updates the pic */
    updateImage();
    updateGender();

    emit pokeChanged(pokenum);
}

void TB_PokemonBody::updateImage()
{
    pokeImage->setPixmap(poke()->picture());
}

void TB_PokemonBody::configureMoves()
{
    QList<QPair<int, QString> > moves;
    int num = poke()->num();

    moves << map_container_with_value(PokemonInfo::LevelMoves(num), tr("Level")) << map_container_with_value(PokemonInfo::EggMoves(num), tr("Breeding"))
	    << map_container_with_value(PokemonInfo::TMMoves(num), tr("TM/HM")) << map_container_with_value(PokemonInfo::TutorMoves(num), tr("Tutor"))
	    << map_container_with_value(PokemonInfo::SpecialMoves(num), tr("Special"));

    movechoice->setRowCount(moves.size());
    movechoice->setSortingEnabled(false);

    for (int i = 0; i < moves.size(); i++)
    {
	QTableWidgetItem *witem;
	int movenum = moves[i].first;
	
	witem = new QTableWidgetItem(TypeInfo::Name(MoveInfo::Type(movenum)));
	witem->setForeground(QColor("white"));
	witem->setBackground(QColor(TypeInfo::Color(MoveInfo::Type(movenum))));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Type, witem);

	witem = new QTableWidgetItem(MoveInfo::Name(movenum));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Name, witem);

	witem = new QTableWidgetItem(moves[i].second);
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Learning, witem);

	witem = new QTableWidgetItem(QString::number(MoveInfo::PP(movenum)));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, PP, witem);

	witem = new QTableWidgetItem(MoveInfo::AccS(movenum));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Acc, witem);

	witem = new QTableWidgetItem(MoveInfo::PowerS(movenum));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Pow, witem);

	witem = new QTableWidgetItem(CategoryInfo::Name(MoveInfo::Category(movenum)));
	witem->setForeground(QColor(CategoryInfo::Color(MoveInfo::Category(movenum))));
	witem->setFlags(witem->flags() ^Qt::ItemIsEditable);
	movechoice->setItem(i, Category, witem);
    }

    for (int i = 0; i < 4; i++)
    {
	m_moves[i]->setText(MoveInfo::Name(poke()->move(i)));
    }

    movechoice->sortItems(Name);
    movechoice->setSortingEnabled(true);
    movechoice->resizeRowsToContents();
}

void TB_PokemonBody::setItem(const QString &item)
{
    poke()->setItem(ItemInfo::Number(item));
}

void TB_PokemonBody::setNature(int nature)
{
    poke()->setNature(nature);
    /* As the nature has an influence over the stats, we update them */
    evchoice->updateEVs();
}

TB_EVManager::TB_EVManager(PokeTeam *_poke)
{
    m_poke = _poke;

    QString labels[6] = {"HP:", "Att:", "Def:", "Speed:", "Spatt:", "SpDef:"};

    for (int i = 0; i < 6; i++)
    {
	addWidget(new QLabel(labels[i]), i, 0, Qt::AlignLeft);
	addWidget(m_stats[i] = new QLabel(), i, 1, Qt::AlignLeft);
	addWidget(m_sliders[i] = new QSlider(Qt::Horizontal), i, 2);
	addWidget(m_evs[i] = new QLabel(), i, 3, Qt::AlignLeft);

	slider(i)->setTracking(true);
	slider(i)->setRange(0,255);
	slider(i)->setMinimumWidth(150);
	m_evs[i]->setMinimumWidth(24);
	connect(slider(i),SIGNAL(valueChanged(int)),SLOT(EVChanged(int)));
    }

    addWidget(m_mainSlider = new QSlider(Qt::Horizontal), 6, 0, 1, 4);
    m_mainSlider->setEnabled(false);
    m_mainSlider->setRange(0,510);

    /*Setting the vals */
    updateEVs();
}

PokeTeam * TB_EVManager::poke()
{
    return m_poke;
}

QSlider * TB_EVManager::slider(int stat)
{
    return m_sliders[stat];
}

const QSlider * TB_EVManager::slider(int stat) const
{
    return m_sliders[stat];
}

QLabel * TB_EVManager::evLabel(int stat)
{
    return m_evs[stat];
}

QLabel * TB_EVManager::statLabel(int stat)
{
    return m_stats[stat];
}

/* the reverse of slider(int) */
int TB_EVManager::stat(QObject *sender) const
{
    for (int i = 0; i < 6; i++)
	if (sender == slider(i))
	    return i;
    throw QString("Fatal Error in TB_EVManager, alert the developers");
}

void TB_EVManager::updateEVs()
{
    for (int i = 0; i < 6; i++)
	updateEV(i);

    updateMain();
}

void TB_EVManager::EVChanged(int newvalue)
{
    int mstat = stat(sender());
    poke()->setEV(mstat, newvalue);

    updateEV(mstat);
    updateMain();
}

void TB_EVManager::updateEV(int stat)
{
    slider(stat)->setValue(poke()->EV(stat));

    /* first the color : red if the stat is hindered by the nature, black if normal, blue if the stat is enhanced */
    QColor colors[3] = {Qt::darkBlue, Qt::black, Qt::red};
    QColor mycol = colors[poke()->natureBoost(stat)+1];
    QPalette pal = evLabel(stat)->palette();
    pal.setColor(QPalette::WindowText, mycol);

    evLabel(stat)->setPalette(pal);
    evLabel(stat)->setText(QString::number(poke()->EV(stat)));

    statLabel(stat)->setPalette(pal);
    statLabel(stat)->setText(QString::number(poke()->stat(stat)));
}

void TB_EVManager::updateMain()
{
    m_mainSlider->setValue(poke()->EVSum());
}
