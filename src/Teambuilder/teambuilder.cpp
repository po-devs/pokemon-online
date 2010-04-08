#include "teambuilder.h"
#include "box.h"
#include "advanced.h"
#include "mainwindow.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/pokemonstructs.h"
#include "../Utilities/dockinterface.h"

template <class T, class U>
        QSet<QPair<typename T::value_type, U> > map_container_with_value(T container, const U & value)
{
    QSet<QPair<typename T::value_type, U> > ret;

    foreach(typename T::value_type val, container)
	ret << (QPair<typename T::value_type, U>(val, value));

    return ret;
}

TeamImporter::TeamImporter()
{
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    QGridLayout *l = new QGridLayout(this);
    l->addWidget(new QLabel(tr("Paste your exported team from Netbattle Supremacy / Shoddy Battle")),0,0,1,2);
    l->addWidget(mycontent = new QPlainTextEdit(),1,0,1,2);
    mycontent->resize(mycontent->width(), 250);

    QPushButton *cancel, *_done;

    cancel = new QPushButton(tr("&Cancel"));
    _done = new QPushButton(tr("&Done"));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(_done, SIGNAL(clicked()), this, SLOT(close()));
    connect(_done, SIGNAL(clicked()), this, SLOT(done()));

    l->addWidget(cancel, 2,0);
    l->addWidget(_done,2,1);
}

void TeamImporter::done()
{
    emit done(mycontent->toPlainText());
}

/**********************************/
/**** AVATAR BOX ******************/
/**********************************/
QPixmap *AvatarBox::background = NULL;

AvatarBox::AvatarBox(const QPixmap &pic)
{
    if (background == NULL)
        background = new QPixmap("db/Teambuilder/avatarBG.png");

    setPixmap(*background);
    setFixedSize(background->size());

    underLying = new QLabel(this);
    underLying->setPixmap(pic);
    underLying->move(1,1);
}

void AvatarBox::changePic(const QPixmap &pic)
{
    underLying->setPixmap(pic);
}

/**********************************/
/**** POKEBALLED ******************/
/**********************************/

QPixmap* Pokeballed::pokeball = NULL;

Pokeballed::Pokeballed(QWidget *w) {
    init(w);
}

Pokeballed::Pokeballed() {

}

void Pokeballed::init(QWidget *w)
{
    if (pokeball == NULL)
        pokeball = new QPixmap("db/Teambuilder/Pokeball.png");

    QHBoxLayout *h = new QHBoxLayout(this);
    h->setMargin(0);

    QLabel *icon = new QLabel();
    icon->setPixmap(*pokeball);
    h->addWidget(icon);

    h->addWidget(w,100,Qt::AlignLeft);
}

/////////////////////////////////////
//// TITLED WIDGET //////////////////
/////////////////////////////////////

TitledWidget::TitledWidget(const QString &title, QWidget *w)
{
    QVBoxLayout *v = new QVBoxLayout(this);
    v->setMargin(0);

    QLabel *l = new QLabel(title);
    l->setObjectName("NormalText");

    v->addWidget(new Pokeballed(l));
    v->addWidget(w,100,Qt::AlignTop);
    l->setBuddy(w);
}

/***********************************/
/**** TEAMBUILDER ******************/
/***********************************/


TeamBuilder::TeamBuilder(TrainerTeam *pub_team) : QImageBackground("db/Teambuilder/background.png"), m_team(pub_team)
{
    setWindowTitle(tr("Teambuilder"));

    memset(modified,false,6);

    /* Buttons of pokemons / trainers */
    QImageButton * m_trainer = new QImageButton("db/Teambuilder/Buttons/TrainerNorm.png", "db/Teambuilder/Buttons/TrainerGlow.png");
    m_trainer->setParent(this);
    m_trainer->move(4,1);

    QImageButton * m_team = new QImageButton("db/Teambuilder/Buttons/TeamNorm.png", "db/Teambuilder/Buttons/TeamGlow.png");
    m_team->setParent(this);
    m_team->move(4+138,1);

    QImageButton * m_box = new QImageButton("db/Teambuilder/Buttons/BoxNorm.png", "db/Teambuilder/Buttons/BoxGlow.png");
    m_box->setParent(this);
    m_box->move(4+2*138,1);

    QImageButton * m_pokedex = new QImageButton("db/Teambuilder/Buttons/PokedexNorm.png", "db/Teambuilder/Buttons/PokedexGlow.png");
    m_pokedex->setParent(this);
    m_pokedex->move(4+3*138,1);

    nextb = new QImageButton("db/Teambuilder/Buttons/NextNorm.png", "db/Teambuilder/Buttons/NextGlow.png");
    nextb->setParent(this);
    nextb->move(490,545);

    QImageButton * m_close = new QImageButton("db/Teambuilder/Buttons/CloseNorm.png", "db/Teambuilder/Buttons/CloseGlow.png");
    m_close->setParent(this);
    m_close->move(490+138,545);


    /* Starting doing the "body" */
    m_body = new QStackedWidget(this);
    m_body->setGeometry(2,60,778,460);
    m_body->layout()->setMargin(0);

    /* Trainer body */
    m_trainerBody = new TB_TrainerBody(this);
    m_body->addWidget(m_trainerBody);

    /* Team Body */
    m_teamBody = new TB_TeamBody(this);
    m_body->addWidget(m_teamBody);

    /* Pokemon Boxes */
    m_boxes = new TB_PokemonBoxes(this);
    m_body->addWidget(m_boxes);

    connect(m_trainer, SIGNAL(clicked()), SLOT(changeToTrainer()));
    connect(m_team, SIGNAL(clicked()), SLOT(changeToTeam()));
    connect(m_box, SIGNAL(clicked()), SLOT(changeToBoxes()));
    connect(nextb, SIGNAL(clicked()), SLOT(changeZone()));
    connect(m_close, SIGNAL(clicked()), SIGNAL(done()));
    connect(m_teamBody, SIGNAL(showDockAdvanced(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)), this,
            SIGNAL(showDock(Qt::DockWidgetArea,QDockWidget*,Qt::Orientation)));
    connect(m_boxes, SIGNAL(pokeChanged(int)), SLOT(pokeChanged(int)));

    updateAll();
}


Team* TeamBuilder::team()
{
    return & m_team->team();
}

TrainerTeam * TeamBuilder::trainerTeam()
{
    return m_team;
}

void TeamBuilder::pokeChanged(int poke)
{
    modified[poke] = true;
}

void TeamBuilder::changeZone()
{
    if (m_body->currentIndex() == TrainerW)
        changeToTeam();
    else
        changeToTrainer();
}

void TeamBuilder::changeToTeam()
{
    if (m_body->currentIndex() == TrainerW)
        nextb->changePics("db/Teambuilder/Buttons/GoBackNorm.png", "db/Teambuilder/Buttons/GoBackGlow.png");
    m_body->setCurrentIndex(TeamW);
    changePic("db/Teambuilder/Team/TeamBG.png");
    for (int i = 0; i < 6; i++) {
        if (modified[i]) {
            m_teamBody->updatePoke(i);
            modified[i] = false;
        }
    }
}

void TeamBuilder::changeToBoxes()
{
    if (m_body->currentIndex() == TrainerW)
        nextb->changePics("db/Teambuilder/Buttons/GoBackNorm.png", "db/Teambuilder/Buttons/GoBackGlow.png");
    m_body->setCurrentIndex(BoxesW);
    changePic("db/Teambuilder/Box/PokeboxBG.png");
    updateBox();
}

void TeamBuilder::changeToTrainer()
{
    if (m_body->currentIndex() != TrainerW) {
        nextb->changePics("db/Teambuilder/Buttons/NextNorm.png", "db/Teambuilder/Buttons/NextGlow.png");
        m_body->setCurrentIndex(TrainerW);
        changePic("db/Teambuilder/Trainer/TrainerBG.png");
    }
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
        updateAll();
    }
}

void TeamBuilder::newTeam()
{
    if (QMessageBox::question(this, tr("New Team"), tr("You sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        for (int i = 0; i < 6; i++) {
            team()->poke(i) = PokeTeam();
        }
        updateTeam();
    }
}

void TeamBuilder::clickOnDone()
{
    emit done();
}

void TeamBuilder::updateAll()
{
    updateTrainer();
    updateTeam();
    updateBox();
}

void TeamBuilder::updateTeam()
{
    m_teamBody->updateTeam();
}

void TeamBuilder::updateTrainer()
{
    m_trainerBody->updateTrainer();
}

void TeamBuilder::updateBox()
{
    m_boxes->updateBox();
}

QMenuBar * TeamBuilder::createMenuBar(MainEngine *w)
{
    QMenuBar *menuBar = new QMenuBar();
    QMenu *menuFichier = menuBar->addMenu(tr("&File"));
    menuFichier->addAction(tr("&New Team"),this,SLOT(newTeam()),Qt::CTRL+Qt::Key_N);
    menuFichier->addAction(tr("&Save Team"),this,SLOT(saveTeam()),Qt::CTRL+Qt::Key_S);
    menuFichier->addAction(tr("&Load Team"),this,SLOT(loadTeam()),Qt::CTRL+Qt::Key_L);
    menuFichier->addAction(tr("&Import From Txt"),this,SLOT(importFromTxt()),Qt::CTRL+Qt::Key_I);
    menuFichier->addAction(tr("&Quit"),qApp,SLOT(quit()),Qt::CTRL+Qt::Key_Q);
    QMenu * menuStyle = menuBar->addMenu(tr("&Style"));
    QStringList style = QStyleFactory::keys();
    for(QStringList::iterator i = style.begin();i!=style.end();i++)
    {
        menuStyle->addAction(*i,w,SLOT(changeStyle()));
    }
    menuStyle->addSeparator();
    menuStyle->addAction(tr("Reload StyleSheet"), w, SLOT(loadStyleSheet()));

    menuBar->setStyleSheet("background: #0ca3df");

    return menuBar;
}

void TeamBuilder::importFromTxt()
{
    if (m_import) {
        m_import->raise();
        return;
    }
    m_import=new TeamImporter();
    m_import->show();
    connect(m_import, SIGNAL(done(QString)), SLOT(importDone(QString)));
}

void TeamBuilder::importDone(const QString &text)
{
    trainerTeam()->importFromTxt(text);
    updateTeam();
}

QWidget * TeamBuilder::createButtonMenu()
{
    QWidget *w = new QWidget();
    QVBoxLayout *vl = new QVBoxLayout(w);
    QPushButton *newb, *saveb, *loadb, *importb;
    vl->setMargin(0);
    vl->addWidget(newb = new QPushButton(tr("&New Team")));
    vl->addWidget(saveb = new QPushButton(tr("&Save Team")));
    vl->addWidget(loadb = new QPushButton(tr("&Load Team")));
    vl->addWidget(importb = new QPushButton(tr("&Import from .txt")));

    connect(saveb, SIGNAL(clicked()), SLOT(saveTeam()));
    connect(loadb, SIGNAL(clicked()), SLOT(loadTeam()));
    connect(newb, SIGNAL(clicked()), SLOT(newTeam()));
    connect(importb, SIGNAL(clicked()), SLOT(importFromTxt()));

    return w;
}

TeamBuilder::~TeamBuilder()
{
}

/**************************************/
/******** TRAINER BODY ****************/
/**************************************/

TB_TrainerBody::TB_TrainerBody(TeamBuilder *teambuilder) : m_team(teambuilder->trainerTeam())
{
    QHBoxLayout *ml = new QHBoxLayout(this);
    ml->setMargin(0);

    //////////////// First Column  ///////////////////
    QVBoxLayout *col1 = new QVBoxLayout();
    ml->addLayout(col1);

    /* Avatar */
    col1->addWidget(new TitledWidget(tr("Avatar"), m_avatar=new AvatarBox()));

    /* Avatar Selection */
    col1->addWidget(m_avatarSelection = new QSpinBox(), 5, Qt::AlignTop);
    m_avatarSelection->setRange(1,167);

    /*** Buttons ***/
    /* New team, save team, load team, import from txt */
    col1->addWidget(teambuilder->createButtonMenu());

    //////////////// Second Column ///////////////////
    QVBoxLayout *col2 = new QVBoxLayout();
    ml->addLayout(col2,100);

    /* Trainer nickname */
    col2->addWidget(new TitledWidget(tr("Trainer &Name"),m_nick = new QLineEdit()));
    m_nick->setMaximumWidth(150);
    m_nick->setValidator(new QNickValidator(m_nick));

    /* Trainer name color */
    col2->addWidget(new TitledWidget(tr("Name Color"), m_colorButton = new QPushButton(tr("Change &Color"))));
    QSettings s;
    if (s.value("trainer_color").value<QColor>().name() != "#000000")
        m_colorButton->setStyleSheet(QString("background: %1;color:white").arg(s.value("trainer_color").value<QColor>().name()));
    m_colorButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    /* Trainer information */
    col2->addWidget(new TitledWidget(tr("Trainer I&nformation"), m_trainerInfo = new QTextEdit()));

    /* Trainer win, lose message */
    col2->addWidget(new TitledWidget(tr("&Winning Message"), m_winMessage = new QTextEdit()));
    col2->addWidget(new TitledWidget(tr("L&osing Message"), m_loseMessage = new QTextEdit()));

    //////////////// Third  Column ///////////////////
    QVBoxLayout *col3 = new QVBoxLayout();
    ml->addLayout(col3,0);

    QLabel *ash = new QLabel();
    ash->setPixmap(QPixmap("db/Teambuilder/Trainer/AshKetchup.png"));
    QVBoxLayout *vash = new QVBoxLayout(ash);
    QLabel *pikachu = new QLabel();
    pikachu->setPixmap(QPixmap("db/Teambuilder/Trainer/Pikachu.png"));
    vash->addWidget(pikachu,0, Qt::AlignBottom|Qt::AlignHCenter);

    col3->addWidget(ash,0,Qt::AlignBottom);

    m_winMessage->setTabChangesFocus(true);
    m_loseMessage->setTabChangesFocus(true);
    m_trainerInfo->setTabChangesFocus(true);

    connect (m_colorButton, SIGNAL(clicked()), SLOT(changeTrainerColor()));
    connect (m_nick, SIGNAL(textEdited(QString)), SLOT(setTrainerNick(QString)));
    connect (m_winMessage, SIGNAL(textChanged()), SLOT(changeTrainerWin()));
    connect (m_loseMessage, SIGNAL(textChanged()), SLOT(changeTrainerLose()));
    connect (m_trainerInfo, SIGNAL(textChanged()), SLOT(changeTrainerInfo()));
    connect (m_avatarSelection, SIGNAL(valueChanged(int)), SLOT(changeTrainerAvatar(int)));
}

void TB_TrainerBody::changeTrainerColor()
{
    QSettings s;
    QColor c = QColorDialog::getColor(s.value("trainer_color").value<QColor>().name());

    s.setValue("trainer_color", c);
    if (c.name() != "#000000" && c.lightness() <= 172)
        m_colorButton->setStyleSheet(QString("background: %1; color: white").arg(c.name()));
    else {
        s.setValue("trainer_color", "");
        m_colorButton->setStyleSheet("");
    }
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
    changeTrainerAvatar(trainerTeam()->avatar());
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

void TB_TrainerBody::changeTrainerAvatar(int newavatar)
{
    if (newavatar==0)
        newavatar=1;
    m_avatarSelection->setValue(newavatar);
    trainerTeam()->avatar() = newavatar;
    m_avatar->changePic(QPixmap(QString("db/Trainer Sprites/%1.png").arg(newavatar)));
}

/*********************************************/
/**************** POKE BUTTON ****************/
/*********************************************/

QPixmap *TeamPokeButton::teamBoxBall = NULL;

TeamPokeButton::TeamPokeButton(int num, int poke, int level, int item)
{
    if (teamBoxBall == NULL)
        teamBoxBall = new QPixmap("db/Teambuilder/Team/TeamBoxBall.png");

    this->m_num = num;
    setObjectName("PokeButton");
    setAcceptDrops(true);
    setCheckable(true);

    QGridLayout *ml = new QGridLayout(this);
    ml->setMargin(2);
    ml->setSpacing(2);

    QLabel *pokeBG = new QLabel();
    pokeBG->setPixmap(*teamBoxBall);
    pokeBG->setFixedSize(teamBoxBall->size());
    pokeIcon = new QLabel(pokeBG);
    pokeIcon->move(7,3);

    ml->addWidget(pokeBG,0,0,3,1);

    QLabel * pokeText = new QLabel(tr("Pokémon &%1").arg(num+1));
    pokeText->setObjectName("NormalText");
    pokeText->setBuddy(this);
    ml->addWidget(pokeText, 0,1,1,2);

    ml->addWidget(this->level = new QLabel(),1,1,1,2);
    this->level->setObjectName("SmallText");

    QLabel *iteml;
    ml->addWidget(iteml = new QLabel(tr("item")), 2,1,1,1, Qt::AlignLeft);
    iteml->setObjectName("SmallText");

    ml->addWidget(itemIcon = new QLabel(), 2,2,1,1, Qt::AlignRight);
    itemIcon->setFixedSize(24,24);

    changeInfos(poke, level, item);
}

void TeamPokeButton::changeInfos(int poke, int level, int item)
{
    pokeIcon->setPixmap(PokemonInfo::Icon(poke));
    this->level->setText(tr("Lv. %1").arg(level));
    itemIcon->setPixmap(ItemInfo::Icon(item));
}

void TeamPokeButton::mouseMoveEvent(QMouseEvent * event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos()-startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            startDrag();
        }
    }
    QPushButton::mouseMoveEvent(event);
}

void TeamPokeButton::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
    }
    QPushButton::mousePressEvent(event);
}


void TeamPokeButton::dragEnterEvent(QDragEnterEvent * event)
{
    if(event->source()->objectName()=="PokeButton" || event->source()->objectName()=="PokeChoice")
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void TeamPokeButton::dropEvent(QDropEvent * event)
{
    if(event->source()->objectName()=="PokeButton")
    {
        TeamPokeButton * source = qobject_cast<TeamPokeButton *>(event->source());
        if(source && source != this)
        {
            QPair<int,int> echange;
            echange.first = source->num();
            echange.second = this->num();

            emit changePokemonOrder(echange);
        }
    }
    else if(event->source()->objectName()=="PokeChoice")
    {
        TB_PokeChoice * source = qobject_cast<TB_PokeChoice *>(event->source());
        if(source)
        {
            const QMimeData * data = event->mimeData();
            emit changePokemonBase(this->num(),data->text().toInt());
        }
    }
}

void TeamPokeButton::startDrag()
{
    QMimeData * data = new QMimeData();
    data->setText(this->text());
    data->setImageData(*pokeIcon->pixmap());
    QDrag * drag = new QDrag(this);
    drag->setMimeData(data);
    drag->setPixmap(*pokeIcon->pixmap());
    drag->exec(Qt::MoveAction);

    emit clicked();
}

/*********************************************/
/**************** TEAM BODY ******************/
/*********************************************/

TB_TeamBody::TB_TeamBody(TeamBuilder *parent) : m_dockAdvanced(0), m_team(parent->trainerTeam())
{
    QVBoxLayout *ml = new QVBoxLayout(this);
    ml->setMargin(0);

    /* Pokemon buttons */
    QHBoxLayout *buttonsl = new QHBoxLayout;
    buttonsl->setSpacing(1);
    ml->addLayout(buttonsl);
    for (int i = 0; i < 6; i++) {
        buttonsl->addWidget(pokeButtons[i] = new TeamPokeButton(i));
    }

    /* Body! */
    body = new QStackedWidget();
    ml->addWidget(body);
    for (int i = 0; i < 6; i++) {
        body->addWidget(pokeBody[i] = new TB_PokemonBody(parent, &trainerTeam()->team().poke(i), i));
    }

    pokeButtons[0]->setChecked(true);
    for(int i = 0; i < 6; i++) {
        connect(pokeButtons[i], SIGNAL(clicked()), SLOT(changeIndex()));
        connect(pokeButtons[i], SIGNAL(changePokemonBase(int,int)), SLOT(changePokemonBase(int,int)));
        connect(pokeButtons[i], SIGNAL(changePokemonOrder(QPair<int,int>)), SLOT(changePokemonOrder(QPair<int,int>)));
        connect(pokeBody[i], SIGNAL(pokeChanged(int)), SLOT(updateButton()));
        connect(pokeBody[i], SIGNAL(itemChanged(int)), SLOT(updateButton()));
        connect(pokeBody[i], SIGNAL(levelChanged()), SLOT(updateButton()));
        connect(pokeBody[i], SIGNAL(advanced(int, bool)), SLOT(advancedClicked(int, bool)));
        connect(pokeBody[i], SIGNAL(pokeChanged(int)),SLOT(indexNumChanged(int)));
    }
}

void TB_TeamBody::changePokemonOrder(QPair<int, int>echange)
{
    int i1 = echange.first;
    int i2 = echange.second;
    //recuperation des widgets
    TB_PokemonBody * poke1 = pokeBody[i1];
    TB_PokemonBody * poke2 = pokeBody[i2];

    /* Replacing the pokemons in the team struct */
    std::swap(*(poke1->poke()), *(poke2->poke()));

    poke1->updateNum();
    poke2->updateNum();
}

void TB_TeamBody::changePokemonBase(int index,int pokenum)
{
    TB_PokemonBody * poke1 = pokeBody[index];

    poke1->setNum(pokenum);
    poke1->updateNum();
}

void TB_TeamBody::changeIndex()
{
    int num = ((TeamPokeButton*)sender())->num();
    body->setCurrentIndex(num);

    for (int i = 0; i < 6; i++) {
        pokeButtons[i]->setChecked(i==num);
    }

    if (dockAdvanced())
        m_dockAdvanced->setCurrentPokemon(num);
}

void TB_TeamBody::updateButton()
{
    int i = ((TB_PokemonBody*)sender())->num();
    pokeButtons[i]->changeInfos(trainerTeam()->team().poke(i).num(), trainerTeam()->team().poke(i).level(),
                                trainerTeam()->team().poke(i).item());
}

void TB_TeamBody::updateTeam()
{
    for(int i=0; i < 6; i++) {
        updatePoke(i);
    }
}

void TB_TeamBody::updatePoke(int num)
{
    pokeBody[num]->updateNum();
}

void TB_TeamBody::createDockAdvanced(bool sepWindow)
{
    m_dockAdvanced = new DockAdvanced(this);

    connect(m_dockAdvanced, SIGNAL(destroyed()), SLOT(advancedDestroyed()));
    connect(this, SIGNAL(destroyed()),m_dockAdvanced, SLOT(deleteLater()));

    if (!sepWindow)
        emit this->showDockAdvanced(Qt::RightDockWidgetArea,m_dockAdvanced,Qt::Vertical);
    else {
        m_dockAdvanced->setWindowFlags(Qt::Window);
        m_dockAdvanced->show();
    }
}

void TB_TeamBody::indexNumChanged(int pokeNum)
{
    if(dockAdvanced())
    {
        int index = ((TB_PokemonBody*)sender())->num();
        m_dockAdvanced->setPokemonNum(index,pokeNum);
    }
}

void TB_TeamBody::advancedClicked(int index, bool separateWindow)
{
    if(!dockAdvanced())
    {
        createDockAdvanced(separateWindow);
        dockAdvanced()->setCurrentPokemon(index);
    } else {
        m_dockAdvanced->close();
    }
}

void TB_TeamBody::advancedDestroyed()
{
    m_dockAdvanced = 0;
}


/**********************************************/
/************* POKEMON CHOICE *****************/
/**********************************************/

TB_PokeChoice::TB_PokeChoice() : QCompactTable(PokemonInfo::TrueCount(), 2)
{
    setObjectName("PokeChoice");

    setFixedWidth(150);

    horizontalHeader()->hide();
    horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

    /* Adding the poke names */
    for (int i = 0; i < rowCount(); i++)
    {
        setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        setItem(i, 1, new QTableWidgetItem(PokemonInfo::Name(i)));
    }

    resizeRowsToContents();
}

void TB_PokeChoice::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        startPos = event->pos();
        itemForDrag = this->itemAt(event->pos());
    }
    QCompactTable::mousePressEvent(event);
}

void TB_PokeChoice::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int distance = (event->pos()-startPos).manhattanLength();
        if(distance >= QApplication::startDragDistance())
        {
            startDrag();
        }
    }
    QCompactTable::mouseMoveEvent(event);
}

void TB_PokeChoice::startDrag()
{
    QMimeData * data = new QMimeData();
    if(itemForDrag)
    {
        itemForDrag = item(itemForDrag->row(),0);

        data->setText(itemForDrag->text());
        data->setImageData(PokemonInfo::Picture(itemForDrag->text().toInt()));

        QDrag * drag = new QDrag(this);
        drag->setMimeData(data);
        drag->setPixmap(PokemonInfo::Picture(itemForDrag->text().toInt()));
        drag->exec(Qt::MoveAction);
    }
}

/**********************************************/
/************ POKEMON BODY ********************/
/**********************************************/

TB_PokemonBody::TB_PokemonBody(TeamBuilder *upparent, PokeTeam *_poke, int num)
{
    m_poke = _poke;
    m_num = num;

    QGridLayout *ml = new QGridLayout(this);
    ml->setMargin(0);

    /*** Box 1 ***/
    QVBoxLayout *box1 = new QVBoxLayout();
    ml->addLayout(box1,0,0,2,1);

    box1->addWidget(new Pokeballed(m_pokeedit = new QLineEdit()));
    box1->addWidget(pokechoice = new TB_PokeChoice());

    box1->addWidget(new TitledWidget(tr("&Nickname"),m_nick = new QLineEdit()));
    m_nick->setValidator(new QNickValidator(m_nick));

    QWidget *itemw = new QWidget();
    QHBoxLayout *hitem = new QHBoxLayout(itemw);
    hitem->setMargin(0);
    hitem->addWidget(itemchoice = new QComboBox());
    hitem->addWidget(itemicon = new QLabel());
    itemicon->setFixedSize(24,24);
    box1->addWidget(new TitledWidget(tr("&Item"), itemw));

    box1->addSpacerItem(new QSpacerItem(0,6));
    QWidget *w = upparent->createButtonMenu();
    w->layout()->setSpacing(4);
    box1->addWidget(w);

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
    box21->addWidget(pokeImage = new AvatarBox(),0);
    QPushButton *advanced;
    box21->addWidget(advanced = new QPushButton(tr("&Advanced")), 10, Qt::AlignBottom);
    QMenu *m = new QMenu(advanced);
    QAction *a = m->addAction(tr("Side Window"), this, SLOT(goToAdvanced()));
    a->setProperty("window", 0);
    a = m->addAction(tr("Separate Window"), this, SLOT(goToAdvanced()));
    a->setProperty("window", 1);
    advanced->setMenu(m);

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

    QLabel *nature;
    box22->addWidget(nature = new QLabel(tr("N&ature")));
    box22->addWidget(naturechoice = new QComboBox());
    nature->setBuddy(naturechoice);
    nature->setObjectName("NormalText");
    for (int i = 0; i < NatureInfo::NumberOfNatures(); i++)
    {
        naturechoice->addItem(NatureInfo::Name(i));
    }

    /*** Box 3 ***/
    evchoice = new TB_EVManager(poke());  
    ml->addWidget(evchoice,0,2);


    /*** Box 4 ***/
    QGridLayout *box4 = new QGridLayout();
    ml->addLayout(box4, 1,1,1,2);
    ml->setRowStretch(1, 100);
    ml->setColumnStretch(2, 100);
    box4->setSpacing(0);

    movechoice = new MoveList();
    box4->addWidget(movechoice, 0, 0, 1, 4);
    for (int i = 0; i < 4; i++)
    {
        box4->addWidget(m_moves[i] = new QLineEdit(),1,i);
    }

    /* Init & connect */

    initPokemons();
    initItems();
    initMoves();

    connect(evchoice, SIGNAL(EVChanged(int)), SIGNAL(EVChanged(int)));
    connect(naturechoice, SIGNAL(activated(int)), SLOT(setNature(int)));
    connect(naturechoice, SIGNAL(activated(int)), SIGNAL(natureChanged()));
    connect(m_nick, SIGNAL(textEdited(QString)), SLOT(setNick(QString)));
    connect(m_nick, SIGNAL(textChanged(QString)),this,SLOT(setNick(QString)));
    connect(evchoice, SIGNAL(natureChanged(int, int)),SLOT(editNature(int,int)));
}

void TB_PokemonBody::initItems()
{
    QStringList itemList = ItemInfo::SortedNames();
    itemchoice->addItems(itemList);

    connect(itemchoice, SIGNAL(activated(QString)), SLOT(setItem(const QString &)));
}

void TB_PokemonBody::initPokemons(TB_PokemonBody *copy)
{
    QCompleter *completer = new QCompleter(m_pokeedit);
    completer->setModel(pokechoice->model());
    completer->setCompletionColumn(1);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    m_pokeedit->setCompleter(completer);

    connect(completer, SIGNAL(activated(QString)), this, SLOT(setPokeByNick()));
    connect(m_pokeedit, SIGNAL(returnPressed()), this, SLOT(setPokeByNick()));
    connect(pokechoice, SIGNAL(cellActivated(int,int)), SLOT(setNum(int)));
}

void TB_PokemonBody::initMoves()
{
    connect(movechoice, SIGNAL(cellActivated(int,int)), SLOT(moveEntered(int)));
    connect(this, SIGNAL(moveChosen(int)), SLOT(setMove(int)));

    QSignalMapper *mapper = new QSignalMapper(this);

    /* the four move choice items */
    for (int i = 0; i < 4; i++)
    {
        QCompleter *completer = new QCompleter(m_moves[i]);
        completer->setModel(movechoice->model());
        completer->setCompletionColumn(Name);
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

TB_PokemonBody::MoveList::MoveList() : QCompactTable(0,7)
{
    QStringList move_headers;
    move_headers << tr("Type") << tr("Name", "AttackName") << tr("Learning") << tr("PP") << tr("Pow") << tr("Acc") << tr("Category");
    setHorizontalHeaderLabels(move_headers);
    resizeRowsToContents();
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setResizeMode(PP, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Pow, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Acc, QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeMode(Name, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(Name, 125);
}

bool TB_PokemonBody::MoveList::event(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

        int row  = this->rowAt(helpEvent->pos().y() - horizontalHeader()->height());
        if (row != -1) {
            QToolTip::setFont(QFont("Verdana",10));
            QToolTip::showText(helpEvent->globalPos(), MoveInfo::Description(MoveInfo::Number(item(row, TB_PokemonBody::Name)->text())));
        }
    }
    return QCompactTable::event(event);
}


void TB_PokemonBody::connectWithAdvanced(TB_Advanced *ptr)
{
    connect(ptr, SIGNAL(levelChanged()), this, SLOT(updateLevel()));
    connect(ptr, SIGNAL(imageChanged()), this, SLOT(updateImage()));
    connect(ptr, SIGNAL(genderChanged()), this, SLOT(updateGender()));
    connect(ptr, SIGNAL(genderChanged()), this, SLOT(updateImage()));
    connect(ptr, SIGNAL(statChanged()), this, SLOT(updateEVs()));
    connect(ptr, SIGNAL(pokeFormChanged(int)), this, SLOT(changeForm(int)));
    connect(this, SIGNAL(EVChanged(int)), ptr, SLOT(updateStat(int)));
    connect(this, SIGNAL(natureChanged()), ptr, SLOT(updateStats()));
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

void TB_PokemonBody::setNum(int pokenum)
{
    setNum(pokenum, true);
}

void TB_PokemonBody::setNum(int pokenum, bool resetEverything)
{
    if (pokenum == poke()->num())
        return;

    if (resetEverything) {
        poke()->reset();
    }

    poke()->setNum(pokenum);
    poke()->load();

    updateNum();
}

void TB_PokemonBody::setPokeByNick()
{
    int number = PokemonInfo::Number(m_pokeedit->text());

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
    configureMoves();

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
    int original = PokemonInfo::OriginalForm(poke()->num());
    m_pokeedit->setText(PokemonInfo::Name(original));
    pokechoice->setCurrentCell(original, 1, QItemSelectionModel::Rows);
    pokechoice->scrollTo(pokechoice->currentIndex(), QAbstractItemView::PositionAtCenter);
}

void TB_PokemonBody::updateNickname()
{     
    m_nick->setText(poke()->nickname());
}

void TB_PokemonBody::updateTypes()
{
    int ttype1 = PokemonInfo::Type1(poke()->num());
    int ttype2 = PokemonInfo::Type2(poke()->num());

    type1->setPixmap(TypeInfo::Picture(ttype1));
    type2->setPixmap(TypeInfo::Picture(ttype2));

    if (ttype2 == Pokemon::Curse)  {
        type2->hide();
    } else {
        type2->show();
    }
}

void TB_PokemonBody::updateItem()
{
    itemchoice->setCurrentIndex(ItemInfo::SortedNumber(ItemInfo::Name(poke()->item())));
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

void TB_PokemonBody::changeForm(int pokenum)
{
    setNum(pokenum, false);
}

void TB_PokemonBody::updateImage()
{
    pokeImage->changePic(poke()->picture());
}

void TB_PokemonBody::updateGender()
{
    genderIcon->setPixmap(GenderInfo::Picture(poke()->gender()));
}

void TB_PokemonBody::updateMoves()
{
    for (int i = 0; i < 4; i++)
    {
        if (PokemonInfo::Moves(poke()->num()).contains(poke()->move(i)))
            m_moves[i]->setText(MoveInfo::Name(poke()->move(i)));
        else {
            poke()->setMove(0,i);
            m_moves[i]->setText(MoveInfo::Name(0));
        }
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
    QSet<int> already_loaded;

    int num = poke()->num();

    QSet<QPair<int, QString> > sets[] = {
        map_container_with_value(PokemonInfo::TMMoves(num), tr("TM/HM")),
        map_container_with_value(PokemonInfo::TutorMoves(num), tr("4G Tutor")),
        map_container_with_value(PokemonInfo::LevelMoves(num), tr("4G Level")),
        map_container_with_value(PokemonInfo::PreEvoMoves(num), tr("4G Pre Evo")),
        map_container_with_value(PokemonInfo::EggMoves(num), tr("4G Breeding")),
        map_container_with_value(PokemonInfo::SpecialMoves(num), tr("4G Special")),
        map_container_with_value(PokemonInfo::TutorMoves(num,3), tr("3G Tutor")),
        map_container_with_value(PokemonInfo::LevelMoves(num,3), tr("3G Level")),
        map_container_with_value(PokemonInfo::PreEvoMoves(num,3), tr("3G Pre Evo")),
        map_container_with_value(PokemonInfo::EggMoves(num,3), tr("3G Breeding")),
        map_container_with_value(PokemonInfo::SpecialMoves(num,3), tr("3G Special"))
    };

    typedef QPair<int, QString> qpair;

    for (int i = 0; i < 11; i++) {
        foreach(qpair pair, sets[i]) {
            if (already_loaded.contains(pair.first))
                continue;
            already_loaded.insert(pair.first);
            moves.push_back(pair);
        }
    }

    movechoice->setRowCount(moves.size());
    movechoice->setSortingEnabled(false);

    int i = -1;

    foreach (qpair pair, moves)
    {
        i++;

	QTableWidgetItem *witem;
        int movenum = pair.first;
	
	witem = new QTableWidgetItem(TypeInfo::Name(MoveInfo::Type(movenum)));
	witem->setForeground(QColor("white"));
        witem->setBackground(QColor(TypeInfo::Color(MoveInfo::Type(movenum))));
	movechoice->setItem(i, Type, witem);

        witem = new QTableWidgetItem(MoveInfo::Name(movenum));
	movechoice->setItem(i, Name, witem);

        witem = new QTableWidgetItem(pair.second);
	movechoice->setItem(i, Learning, witem);

        witem = new QTableWidgetItem(QString::number(MoveInfo::PP(movenum)).rightJustified(2));
	movechoice->setItem(i, PP, witem);

        witem = new QTableWidgetItem(MoveInfo::AccS(movenum).rightJustified(3));
	movechoice->setItem(i, Acc, witem);

        witem = new QTableWidgetItem(MoveInfo::PowerS(movenum).rightJustified(3));
	movechoice->setItem(i, Pow, witem);

	witem = new QTableWidgetItem(CategoryInfo::Name(MoveInfo::Category(movenum)));
        witem->setForeground(QColor(CategoryInfo::Color(MoveInfo::Category(movenum))));
	movechoice->setItem(i, Category, witem);
    }

    movechoice->sortItems(Name);
    movechoice->setSortingEnabled(true);
    movechoice->resizeColumnsToContents();
    movechoice->horizontalHeader()->setStretchLastSection(true);
    movechoice->horizontalHeader()->setResizeMode(PP, QHeaderView::ResizeToContents);
    movechoice->horizontalHeader()->setResizeMode(Pow, QHeaderView::ResizeToContents);
    movechoice->horizontalHeader()->setResizeMode(Acc, QHeaderView::ResizeToContents);
    movechoice->horizontalHeader()->setResizeMode(Name, QHeaderView::Fixed);
    movechoice->horizontalHeader()->resizeSection(Name,130);
}

void TB_PokemonBody::setItem(const QString &item)
{
    int it = ItemInfo::Number(item);

    if (it == Item::GriseousOrb && poke()->num() != Pokemon::Giratina)
        poke()->item() = 0;
    else {
        poke()->item() = ItemInfo::Number(item);
        if (poke()->item() == Item::GriseousOrb) {
            changeForm(Pokemon::Giratina_O);
        } else if (poke()->num() == Pokemon::Giratina_O) {
            changeForm(Pokemon::Giratina);
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
    setNature(NatureInfo::natureOf(up,down));
    updateNature();
}

/****************************************************/
/******* EV MANAGER *********************************/
/****************************************************/

TB_EVManager::TB_EVManager(PokeTeam *_poke)
{
    m_poke = _poke;

    myStatUp = -1;
    myStatDown = -1;
    QGridLayout *l = new QGridLayout(this);
    l->setSpacing(0);

    QString labels[6] = {tr("Hit Points:"), tr("Attack:"), tr("Defense:"), tr("Speed:"), tr("Special Attack:"), tr("Special Defense:")};

    for (int i = 0; i < 6; i++)
    {
        QLabel *lab;
        l->addWidget(lab = new QLabel(labels[i]), i, 0, Qt::AlignLeft);
        lab->setObjectName("SmallText");
        l->addWidget(m_stats[i] = new QLabel(), i, 2, Qt::AlignLeft);
        m_stats[i]->setObjectName("SmallText");
        l->addWidget(m_sliders[i] = new QSlider(Qt::Horizontal), i, 3);
        l->addWidget(m_evs[i] = new QLineEdit("0"), i, 4, Qt::AlignLeft);

        if (!i==0){
            l->addWidget(natureButtons[i-1] = new QImageButtonLR("db/Teambuilder/Team/=.png","db/Teambuilder/Team/=2.png"),i,1,Qt::AlignLeft);   
            natureButtons[i-1]->setFixedWidth(20);
            natureButtons[i-1]->setFixedHeight(14);
            connect(natureButtons[i-1],SIGNAL(rightClick()),SLOT(checkNButtonR()));
            connect(natureButtons[i-1],SIGNAL(leftClick()),SLOT(checkNButtonL()));
        }
        slider(i)->setTracking(true);
	slider(i)->setRange(0,255);
	slider(i)->setMinimumWidth(150);
        m_evs[i]->setFixedWidth(35);
        connect(slider(i),SIGNAL(valueChanged(int)),SLOT(changeEV(int)));
        connect(m_evs[i], SIGNAL(textChanged(QString)), SLOT(changeEV(QString)));

    }

    l->addWidget(m_mainSlider = new QSlider(Qt::Horizontal), 6, 0, 1, 4);
    l->addWidget(m_mainLabel = new QLabel(), 6, 4);
    m_mainLabel->setObjectName("SmallText");
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

QLineEdit * TB_EVManager::evLabel(int stat)
{
    return m_evs[stat];
}

const QLineEdit * TB_EVManager::evLabel(int stat) const
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
        if (sender == slider(i) || sender == evLabel(i))
	    return i;
    throw QString("Fatal Error in TB_EVManager, alert the developers");
}

void TB_EVManager::updateEVs()
{
    for (int i = 0; i < 6; i++)
	updateEV(i);

    updateMain();
}

void TB_EVManager::changeEV(const QString &newvalue)
{
    int mstat = stat(sender());

    poke()->setEV(mstat, std::max(std::min(newvalue.toInt(), 255), 0));


    slider(mstat)->blockSignals(true);
    updateEV(mstat);
    updateMain();
    slider(mstat)->blockSignals(false);

    emit EVChanged(mstat);
}

void TB_EVManager::changeEV(int newvalue)
{
    int mstat = stat(sender());

    if (sender() == slider(mstat)) {
        if (newvalue < 255) {
            newvalue = newvalue - (newvalue %4);
        }
        if (newvalue == poke()->EV(mstat)) {
            return;
        }
    }
    poke()->setEV(mstat, newvalue);

    updateEV(mstat);
    updateMain();

    emit EVChanged(mstat);
}


void TB_EVManager::checkNButtonL()
{
    int loc = 0;
    for (int i = 0; i < 5; i++)
        if (sender() == natureButtons[i])
            loc = i;
    if(myStatDown == loc+1){
        int temp = myStatUp;
        myStatUp = loc+1;
        myStatDown = temp;
    }
    else{
        myStatUp = loc+1;
    }
    if(myStatUp != -1 && myStatDown != -1)
           emit natureChanged(myStatUp,myStatDown);
}

void TB_EVManager::checkNButtonR()
{
    int loc = 0;
    for (int i = 0; i < 5; i++)
        if (sender() == natureButtons[i])
            loc = i;
    if(myStatUp == loc+1){
        int temp = myStatDown;
        myStatDown = loc+1;
        myStatUp = temp;
    }
    else{
        myStatDown = loc+1;
    }
    if(myStatUp != -1 && myStatDown != -1)
           emit natureChanged(myStatUp,myStatDown);
}

void TB_EVManager::updateEV(int stat)
{
    slider(stat)->setValue(poke()->EV(stat));

    /* first the color : red if the stat is hindered by the nature, black if normal, blue if the stat is enhanced */
    QColor colors[3] = {Qt::red, Qt::white, Qt::darkGreen};

    evLabel(stat)->setText(QString::number(poke()->EV(stat)));
    statLabel(stat)->setText(toColor(QString::number(poke()->stat(stat)), colors[poke()->natureBoost(stat)+1]));
}

void TB_EVManager::updateMain()
{
    m_mainSlider->setValue(poke()->EVSum());
    m_mainLabel->setText(QString::number(poke()->EVSum()));
}

void TB_EVManager::updateNatureButtons()
{
    bool upC = false;
    bool downC = false;
    for (int i = 0; i < 5; i++){
        if(NatureInfo::Boost(poke()->nature(),i+1) == 1){
            upC = true;
            myStatUp = i+1;
        }
        else if(NatureInfo::Boost(poke()->nature(),i+1) == -1){
            downC = true;
            myStatDown = i+1;
        }
    }
    if(!upC&&!downC){
        myStatUp = -1;
        myStatDown = -1;
    }
    for (int j = 0; j<5;j++){
        if (j+1 == myStatUp)
            natureButtons[j]->changePics("db/Teambuilder/Team/+.png","db/Teambuilder/Team/+hover.png");
        else if(j+1 == myStatDown)
            natureButtons[j]->changePics("db/Teambuilder/Team/-.png","db/Teambuilder/Team/-hover.png");
        else
            natureButtons[j]->changePics("db/Teambuilder/Team/=.png","db/Teambuilder/Team/=hover.png");
    }
}
