#include "teambuilder.h"
#include "advanced.h"
#include "mainwindow.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/dockinterface.h"

template <class T, class U>
QList<QPair<typename T::value_type, U> > map_container_with_value(T container, const U & value)
{
    QList<QPair<typename T::value_type, U> > ret;

    foreach(typename T::value_type val, container)
	ret << (QPair<typename T::value_type, U>(val, value));

    return ret;
}

QNickValidator::QNickValidator(QWidget *parent)
	: QValidator(parent)
{}

bool QNickValidator::isBegEndChar(QChar ch) const
{
    return ch.isLetterOrNumber() || ch.isPunct();
}

void QNickValidator::fixup(QString &input) const
{
    /* The only real case when you need to fix a string that's intermediate
       is to remove the trailing space at the end. */
    if (input.length() > 0 && input[input.length()-1] == ' ') {
	input.resize(input.length()-1);
    }
}

QValidator::State QNickValidator::validate(QString &input, int& pos) const
{
    (void) pos;

    if (input.length() == 0)
	return QValidator::Intermediate;

    if (!isBegEndChar(input[0])) {
	return QValidator::Invalid;
    }

    bool spaced = false;
    bool punct = false;

    for (int i = 0; i < input.length(); i++) {
	if (input[i] == '%')
	    return QValidator::Invalid;
	if (input[i].isPunct()) {
	    if (punct == true) {
		//Error: two punctuations are not separated by a letter/number
		return QValidator::Invalid;
	    }
	    punct = true;
	    spaced = false;
	} else if (input[i] == ' ') {
	    if (spaced == true) {
		//Error: two spaces are following
		return QValidator::Invalid;
	    }
	    spaced = true;
	} else if (input[i].isLetterOrNumber()) {
	    //we allow another punct & space
	    punct = false;
	    spaced = false;
	}
    }

    //let's check if there is at least a letter/number & no whitespace at the end
    if (input.length() == 1 && input[0].isPunct()) {
	return QValidator::Intermediate;
    }
    if (!isBegEndChar(input[input.length()-1])) {
	return QValidator::Intermediate;
    }

    return QValidator::Acceptable;
}


TeamBuilder::TeamBuilder(TrainerTeam *pub_team) : m_team(pub_team),m_dockAdvanced(0)
{
    setFixedSize(600, 650);
    setWindowTitle(tr("Teambuilder"));

    //setStyleSheet("background: qradialgradient(cx:0, cy:0, radius: 1, fx:0.5, fy:0.5, stop:0 white, stop:1 blue)");

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
        if(i<3)
        {
            layout->addWidget(m_pokemon[i],0,(i%3)+1);
        }
        else
        {
            layout->addWidget(m_pokemon[i],1,(i%3)+1);
        }
    }

    m_body = new QStackedWidget(this);

    layout->addWidget(m_body,2,0,1,4);

    m_trainerBody = new TB_TrainerBody(this);
    m_body->addWidget(m_trainerBody);

    for (int i = 0; i < 6; i++)
    {
	m_pbody[i] = new TB_PokemonBody(&team()->poke(i));
        m_pbody[i]->setObjectName(tr("Poke%1").arg(i));
        connect(m_pbody[i],SIGNAL(pokeChanged(int)),this,SLOT(setIconForPokeButton()));
        connect(m_pbody[i],SIGNAL(nicknameChanged(QString)),this,SLOT(setNicknameIntoButton(QString)));
        connect(m_pbody[i],SIGNAL(advanced(int)),this,SLOT(advancedClicked(int)));
        connect(m_pbody[i],SIGNAL(pokeChanged(int)),this,SLOT(indexNumPokemonChangedForAdvanced(int)));
	m_body->addWidget(m_pbody[i]);
    }

    connectAll();

    updateTeam();
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
    return & m_team->team();
}

Team* TeamBuilder::getTeam()const
{
    return & m_team->team();
}

TrainerTeam * TeamBuilder::trainerTeam()
{
    return m_team;
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
        //partie a changer
        if(i>0)
        {
	    if(dockAdvanced() != 0)
            {
		dockAdvanced()->setCurrentPokemon(i-1);
            }
        }
        else
        {
	    //m_dockAdvanced->close();
        }
    }
}

void TeamBuilder::setIconForPokeButton()
{
    QString name = sender()->objectName();
    if(!name.contains("Poke"))
    {
        return;
    }
    TB_PokemonBody * body = qobject_cast<TB_PokemonBody *>(sender());
    int index = name.remove("Poke",Qt::CaseSensitive).toInt();
    m_pokemon[index]->setIcon(body->poke()->icon());
}

void TeamBuilder::setNicknameIntoButton(QString nickname)
{
    QString nameBody = sender()->objectName();
    if(!nameBody.contains("Poke"))
    {
        return;
    }
    int index = nameBody.remove("Poke",Qt::CaseSensitive).toInt();
    QString textButton = m_pokemon[index]->text();
    TB_PokemonBody * body = qobject_cast<TB_PokemonBody *>(sender());
    if(body->poke()->num() != 0 /*&& textButton == QString("Pokémon &%1").arg(index+1)*/)
    {
        QString newText = (nickname + "\n(&%1)").arg(index+1);
        m_pokemon[index]->setText(newText);
    }
    else
    {
        m_pokemon[index]->setText(QString("Pokémon &%1").arg(index+1));
    }
}

void TeamBuilder::advancedClicked(int index)
{
    if(dockAdvanced() == 0)
    {
	createDockAdvanced();
	dockAdvanced()->setCurrentPokemon(index);
    }
}

void TeamBuilder::advancedDestroyed()
{
    m_dockAdvanced = 0;
}

void TeamBuilder::saveTeam()
{
    QSettings settings;
    QString newLocation;

    if (saveTTeamDialog(*trainerTeam(), settings.value("team_location").toString(), &newLocation)) {
        settings.setValue("team_location", newLocation);
    }
}

void TeamBuilder::loadTeam()
{
    QSettings settings;
    QString newLocation;

    if (loadTTeamDialog(*trainerTeam(), settings.value("team_location").toString(), &newLocation)) {
        settings.setValue("team_location", newLocation);
        updateTeam();
    }
}

void TeamBuilder::clickOnDone()
{
    emit done();
}

void TeamBuilder::updateTeam()
{
    for (int i = 0; i < 6; i++) {
        updatePokemon(i);
    }
    updateTrainer();
}

void TeamBuilder::updatePokemon(int index)
{
    pokebody(index)->updateNum();
}

void TeamBuilder::updateTrainer()
{
    trainerbody()->updateTrainer();
}

QMenuBar * TeamBuilder::createMenuBar(MainWindow *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu("&File");
    menuFichier->addAction(tr("&Save Team"),this,SLOT(saveTeam()),Qt::CTRL+Qt::Key_S);
    menuFichier->addAction(tr("&Load Team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("&Quit"),w,SLOT(close()),Qt::CTRL+Qt::Key_Q);

    return menuBar;
}

//creation du dockAdvanced
void TeamBuilder::createDockAdvanced()
{
    m_dockAdvanced = new DockAdvanced(this);

    connect(m_dockAdvanced, SIGNAL(destroyed()), SLOT(advancedDestroyed()));
    
    emit this->showDockAdvanced(Qt::RightDockWidgetArea,m_dockAdvanced,Qt::Vertical);
}

void TeamBuilder::indexNumPokemonChangedForAdvanced(int pokeNum)
{
    if(dockAdvanced()!=0)
    {
	int index = QString(sender()->objectName()).remove("Poke").toInt();
        m_dockAdvanced->setPokemonNum(index,pokeNum);
    }
}

DockAdvanced * TeamBuilder::dockAdvanced() const
{
    return m_dockAdvanced;
}

TB_PokemonBody * TeamBuilder::pokebody(int index)
{
    return m_pbody[index];
}

TB_TrainerBody * TeamBuilder::trainerbody()
{
    return m_trainerBody;
}

TeamBuilder::~TeamBuilder()
{
}

TB_TrainerBody::TB_TrainerBody(TeamBuilder *teambuilder) : m_team(teambuilder->trainerTeam())
{
    //main layout
    QVBoxLayout *mlayout = new QVBoxLayout(this);

    QEntitled * trainernick = new QEntitled(tr("T&rainer"), m_nick = new QLineEdit());
    m_nick->setMaximumWidth(150);
    m_nick->setMaxLength(15);
    /* A non-whitespace word caracter followed by any number of white characters and not ended by a space, or just nothing */
    m_nick->setValidator(new QNickValidator(this));
    mlayout->addWidget(trainernick);

    QEntitled * minfo = new QEntitled(tr("Player &Info"), m_trainerInfo=new QTextEdit());
    mlayout->addWidget(minfo);

    QEntitled * mwin = new QEntitled(tr("&Winning Message"), m_winMessage=new QTextEdit());
    mlayout->addWidget(mwin);

//    QEntitled * mdraw = new QEntitled(tr("&Draw Message"), m_drawMessage=new QTextEdit());
//    mlayout->addWidget(mdraw);

    QEntitled * mlose = new QEntitled(tr("&Losing Message"), m_loseMessage=new QTextEdit());
    mlayout->addWidget(mlose);

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    mlayout->addLayout(buttonsLayout);

    QPushButton *saveb, *loadb, *doneb;

    buttonsLayout->addWidget(saveb = new QPushButton(tr("&Save")));
    buttonsLayout->addWidget(loadb = new QPushButton(tr("&Load")));
    buttonsLayout->addWidget(doneb = new QPushButton(tr("&Done")));

    connect(saveb, SIGNAL(clicked()), teambuilder, SLOT(saveTeam()));
    connect(loadb, SIGNAL(clicked()), teambuilder, SLOT(loadTeam()));
    connect(doneb, SIGNAL(clicked()), teambuilder, SLOT(clickOnDone()));

    m_winMessage->setTabChangesFocus(true);
    m_loseMessage->setTabChangesFocus(true);
    m_trainerInfo->setTabChangesFocus(true);

    connect (m_nick, SIGNAL(textEdited(QString)), SLOT(setTrainerNick(QString)));
    connect (m_winMessage, SIGNAL(textChanged()), SLOT(changeTrainerWin()));
    connect (m_loseMessage, SIGNAL(textChanged()), SLOT(changeTrainerLose()));
    connect (m_trainerInfo, SIGNAL(textChanged()), SLOT(changeTrainerInfo()));
}

TrainerTeam * TB_TrainerBody::trainerTeam()
{
    return m_team;
}

void TB_TrainerBody::updateTrainer()
{
    m_trainerInfo->setText(trainerTeam()->trainerInfo());
    m_nick->setText(trainerTeam()->trainerNick());
    m_winMessage->setText(trainerTeam()->trainerWin());
    m_loseMessage->setText(trainerTeam()->trainerLose());
}

void TB_TrainerBody::changeTrainerInfo()
{
    trainerTeam()->setTrainerInfo(m_trainerInfo->toPlainText());
}

void TB_TrainerBody::setTrainerNick(const QString &newnick)
{
    trainerTeam()->setTrainerNick(newnick);
}

 void TB_TrainerBody::changeTrainerWin()
{
     trainerTeam()->setTrainerWin(m_winMessage->toPlainText());
}

void TB_TrainerBody::changeTrainerLose()
{
    trainerTeam()->setTrainerLose(m_loseMessage->toPlainText());
}

TB_PokemonBody::TB_PokemonBody(PokeTeam *_poke)
{
    m_poke = _poke;

    /* The layout of the whole body */
    QGridLayout *layout = new QGridLayout(this);

    initPokemons();

    /* first column, in the upper part */
    QVBoxLayout *first_column = new QVBoxLayout();
    layout->addLayout(first_column,0,0);

    first_column->addWidget(new QEntitled("&Pokemon", pokechoice));
    first_column->addWidget(new QEntitled("&Nickname", m_nick = new QLineEdit()));
    m_nick->setValidator(new QNickValidator(this));
    connect(m_nick, SIGNAL(textEdited(QString)), SLOT(setNick(QString)));
    connect(m_nick, SIGNAL(textChanged(QString)),this,SLOT(setNick(QString)));

    initItems();

    first_column->addWidget(new QEntitled("&Item", itemchoice));

    /* second column, in the upper part */
    QVBoxLayout *second_column = new QVBoxLayout();
    layout->addLayout(second_column,0,1);

    //hack to restrain the size of it all
    QWidget *restrainer = new QWidget();
    second_column->addWidget(restrainer);

    restrainer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QGridLayout *restrainer_layout = new QGridLayout(restrainer);

    pokeImage = new QLabel();
    restrainer_layout->addWidget(pokeImage,0,0,1,2,Qt::AlignBottom|Qt::AlignHCenter);

    genderIcon = new QLabel();

    restrainer_layout->addWidget(genderIcon, 1,0, Qt::AlignCenter | Qt::AlignTop);

    level = new QLabel();

    restrainer_layout->addWidget(level, 1,1, Qt::AlignLeft | Qt::AlignTop);

    naturechoice = new QComboBox();
    for (int i = 0; i < NatureInfo::NumberOfNatures(); i++)
    {
	naturechoice->addItem(NatureInfo::Name(i));
    }
    connect(naturechoice, SIGNAL(activated(int)), SLOT(setNature(int)));

    second_column->addWidget(new QEntitled(tr("&Nature"), naturechoice));

    QPushButton *advanced = new QPushButton(tr("&Advanced"));
    second_column->addWidget(advanced);
    connect(advanced, SIGNAL(pressed()), SLOT(goToAdvanced()));

    /* third and last column of the upper body */
    evchoice = new TB_EVManager(poke());

    layout->addLayout(evchoice,0,2);

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
    movechoice->setMinimumHeight(230);
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
    /*if (poke()->num() != 0)
    {
	if (advancedOpen()) {
            // we show the user where the advanced window is
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
    }*/
    int index = QString(this->objectName()).remove("Poke").toInt();
    emit advanced(index);
}

void TB_PokemonBody::setNum(int pokenum)
{
    if (pokenum == poke()->num())
        return;

    poke()->reset();
    poke()->setNum(pokenum);
    poke()->load();

    updateNum();
}

void TB_PokemonBody::updateLevel()
{
    level->setText(tr("Lv. %1").arg(poke()->level()));
}

void TB_PokemonBody::updateNum()
{
    configureMoves();
    updateMoves();
    updateLevel();
    updateImage();
    updateEVs();
    updateGender();
    updateNature();
    updateItem();
    updateNickname();

    emit pokeChanged(poke()->num());
}

 void TB_PokemonBody::updateNickname()
{
    m_nick->setText(poke()->nickname());
}

void TB_PokemonBody::updateItem()
{
    itemchoice->setCurrentIndex(ItemInfo::SortedNumber(ItemInfo::Name(poke()->item())));
}

void TB_PokemonBody::updateNature()
{
    naturechoice->setCurrentIndex(poke()->nature());
}

void TB_PokemonBody::updateEVs()
{
    evchoice->updateEVs();
}

void TB_PokemonBody::updateImage()
{
    pokeImage->setPixmap(poke()->picture());
}

void TB_PokemonBody::updateGender()
{
    genderIcon->setPixmap(GenderInfo::Picture(poke()->gender()));
}

void TB_PokemonBody::updateMoves()
{
    for (int i = 0; i < 4; i++)
    {
        m_moves[i]->setText(MoveInfo::Name(poke()->move(i)));
    }
}

void TB_PokemonBody::initItems()
{
    itemchoice = new QComboBox();

    QStringList itemList = ItemInfo::SortedNames();

    itemchoice->addItems(itemList);

    connect(itemchoice, SIGNAL(activated(QString)), SLOT(setItem(const QString &)));
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

    movechoice->sortItems(Name);
    movechoice->setSortingEnabled(true);
    movechoice->resizeRowsToContents();
}

void TB_PokemonBody::setItem(const QString &item)
{
    poke()->item() = ItemInfo::Number(item);
}

void TB_PokemonBody::setNature(int nature)
{
    poke()->nature() = nature;
    /* As the nature has an influence over the stats, we update them */
    evchoice->updateEVs();
}

TB_EVManager::TB_EVManager(PokeTeam *_poke)
{
    m_poke = _poke;

    QString labels[6] = {"HP:", "Att:", "Def:", "Speed:", "SpAtt:", "SpDef:"};

    for (int i = 0; i < 6; i++)
    {
	addWidget(new QLabel(labels[i]), i, 0, Qt::AlignLeft);
	addWidget(m_stats[i] = new QLabel(), i, 1, Qt::AlignLeft);
	addWidget(m_sliders[i] = new QSlider(Qt::Horizontal), i, 2);
	addWidget(m_evs[i] = new QLabel(), i, 3, Qt::AlignLeft);

	slider(i)->setTracking(true);
	slider(i)->setRange(0,255);
	slider(i)->setMinimumWidth(150);
        slider(i)->setSingleStep(4);
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
