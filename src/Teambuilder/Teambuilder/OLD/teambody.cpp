#include "teambody.h"

#include <QGridLayout>
#include <QLabel>
#include "theme.h"
#include <QMouseEvent>
#include "pokechoice.h"
#include <QApplication>
#include <QSplitter>
#include <QStringListModel>
#include <QStackedWidget>
#include "pokebody.h"
#include "pokebodywidget.h"
#include "dockinterface.h"
#include "teamholder.h"

/*********************************************/
/**************** POKE BUTTON ****************/
/*********************************************/

TeamPokeButton::TeamPokeButton(int num, Pokemon::uniqueId poke, int level, int item)
{
    this->m_num = num;
    setObjectName("PokeButton");
    setAcceptDrops(true);
    setCheckable(true);
    setAccessibleName(tr("Pokemon slot %1", "TB accessible name").arg(QString::number(num)));

    QGridLayout *ml = new QGridLayout(this);
    ml->setMargin(2);
    ml->setSpacing(2);

    QLabel *pokeBG = new QLabel();
    pokeBG->setPixmap(Theme::FrameBall());
    pokeBG->setFixedSize(pokeBG->pixmap()->size());
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
    ml->addWidget(iteml = new QLabel(tr("Item")), 2,1,1,1, Qt::AlignLeft);
    iteml->setObjectName("SmallText");

    ml->addWidget(itemIcon = new QLabel(), 2,2,1,1, Qt::AlignRight);
    itemIcon->setFixedSize(24,24);

    changeInfos(poke, level, item);
}

void TeamPokeButton::changeInfos(Pokemon::uniqueId poke, int level, int item)
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
            emit changePokemonBase(this->num(), Pokemon::uniqueId(data->text().toInt()));
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

TB_TeamBody::TB_TeamBody(QWidget *parent, TeamHolder *team, int gen, QAbstractItemModel *pokeModel) :
    m_dockAdvanced(0), m_team(team), gen(gen), pokeModel(pokeModel), upParent(parent)
{
    QHBoxLayout *hh = new QHBoxLayout(this);
    hh->setMargin(0);
    splitter = new QSplitter();
    hh->addWidget(splitter);
    splitter->setChildrenCollapsible(false);

    QSettings s;
    QStringList itemList = s.value("show_all_items").toBool() ? ItemInfo::SortedNames(gen) : ItemInfo::SortedUsefulNames(gen);
    itemsModel = new QStringListModel(itemList, this);

    QStringList natures;
    for (int i = 0; i < NatureInfo::NumberOfNatures(); i++) {
        natures.push_back(NatureInfo::Name(i));
    }
    natureModel = new QStringListModel(natures, this);

    QWidget *props = new QWidget();
    QVBoxLayout *ml = new QVBoxLayout(props);

    ml->setMargin(0);

    /* Pokemon buttons */
    QHBoxLayout *buttonsl = new QHBoxLayout;
    buttonsl->setSpacing(1);
    ml->addLayout(buttonsl);
    for (int i = 0; i < 6; i++) {
        const PokeTeam &poke = team->team().poke(i);
        buttonsl->addWidget(pokeButtons[i] = new TeamPokeButton(i,poke.num(),poke.level(), poke.item()));
    }

    /* Body! */
    body = new QStackedWidget();
    ml->addWidget(body);
    for (int i = 0; i < 6; i++) {
        pokeBody[i] = new TB_PokemonBody(&trainerTeam()->team().poke(i), i);
    }

    PokeBodyWidget *widget = new PokeBodyWidget(parent,gen,itemsModel,pokeModel, natureModel);
    pokeBody[0]->setWidget(widget);

    body->addWidget(widget);
    connect(widget, SIGNAL(advanceMenuOpen(bool)), SLOT(advancedClicked(bool)));

    pokeButtons[0]->setChecked(true);
    for(int i = 0; i < 6; i++) {
        connect(pokeButtons[i], SIGNAL(clicked()), SLOT(changeIndex()));
        connect(pokeButtons[i], SIGNAL(changePokemonBase(int,Pokemon::uniqueId)), SLOT(changePokemonBase(int,Pokemon::uniqueId)));
        connect(pokeButtons[i], SIGNAL(changePokemonOrder(QPair<int,int>)), SLOT(changePokemonOrder(QPair<int,int>)));
        connect(pokeBody[i], SIGNAL(pokeChanged(Pokemon::uniqueId)), SLOT(updateButton()));
        connect(pokeBody[i], SIGNAL(itemChanged(int)), SLOT(updateButton()));
        connect(pokeBody[i], SIGNAL(levelChanged()), SLOT(updateButton()));
        connect(pokeBody[i], SIGNAL(pokeChanged(Pokemon::uniqueId)),SLOT(indexNumChanged(Pokemon::uniqueId)));
    }

    splitter->addWidget(props);

    restoreAdvancedState();
}

void TB_TeamBody::reloadItems(bool showAll) {
    QStringList itemList = showAll ? ItemInfo::SortedNames(gen) : ItemInfo::SortedUsefulNames(gen);
    itemsModel->setStringList(itemList);

    for(int i =0; i < 6; i++) {
        pokeBody[i]->updateItem();
    }
}

TB_TeamBody::~TB_TeamBody() {
    saveAdvancedState();
}

void TB_TeamBody::saveAdvancedState()
{
    QSettings settings;

    if (!isAdvancedOpen())
        settings.setValue("advanced_open", false);
    else {
        settings.setValue("advanced_open", true);
        settings.setValue("advanced_separate_window", advSepWindow);
    }
}

void TB_TeamBody::restoreAdvancedState()
{
    QSettings settings;

    if (settings.value("advanced_open").toBool()) {
        advancedClicked(0, settings.value("advanced_separate_window").toBool());
    }
}

bool TB_TeamBody::isAdvancedOpen() const
{
    return bool(m_dockAdvanced);
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

void TB_TeamBody::changePokemonBase(int index, const Pokemon::uniqueId & pokenum)
{
    TB_PokemonBody * poke1 = pokeBody[index];

    poke1->setNum(pokenum);
    poke1->updateNum();
}

void TB_TeamBody::changeIndex()
{
    int num = ((TeamPokeButton*)sender())->num();

    if (!pokeBody[num]->hasWidget()) {
        PokeBodyWidget *widget = new PokeBodyWidget(upParent,gen,itemsModel,pokeModel, natureModel);
        connect(widget, SIGNAL(advanceMenuOpen(bool)), SLOT(advancedClicked(bool)));
        pokeBody[num]->setWidget(widget);

        body->addWidget(widget);
    }
    body->setCurrentWidget(pokeBody[num]->getWidget());

    for (int i = 0; i < 6; i++) {
        pokeButtons[i]->setChecked(i==num);
    }

    if (dockAdvanced())
        m_dockAdvanced->setCurrentPokemon(num);
}

void TB_TeamBody::updateButton()
{
    int i = ((TB_PokemonBody*)sender())->num();
    pokeButtons[i]->changeInfos(trainerTeam()->team().poke(i).num(), trainerTeam()->team().poke(i).level(),trainerTeam()->team().poke(i).item());
}

void TB_TeamBody::updateTeam()
{
    changeGeneration(trainerTeam()->team().gen().num);

    for(int i=0; i < 6; i++) {
        updatePoke(i);
    }
}

void TB_TeamBody::updatePoke(int num)
{
    pokeBody[num]->updateNum();
}

void TB_TeamBody::changeGeneration(int gen)
{
    if (this->gen == gen)
        return;

    this->gen = gen;
    trainerTeam()->team().setGen(gen);

    QSettings s;
    reloadItems(s.value("show_all_items").toBool());

    for(int i = 0; i < 6; i++) {
        pokeBody[i]->changeGeneration(gen);
    }

    if (dockAdvanced()) {
        dockAdvanced()->changeGeneration(gen);
    }
}

void TB_TeamBody::createDockAdvanced(bool sepWindow)
{
    advSepWindow = sepWindow;
    m_dockAdvanced = new DockAdvanced(this);

    connect(m_dockAdvanced, SIGNAL(destroyed()), SLOT(advancedDestroyed()));
    connect(this, SIGNAL(destroyed()),m_dockAdvanced, SLOT(deleteLater()));

    if (!sepWindow) {
        splitter->addWidget(m_dockAdvanced);
        topLevelWidget()->resize(topLevelWidget()->width() + m_dockAdvanced->width() + 70, topLevelWidget()->height());
    }
    else {
        m_dockAdvanced->setObjectName("OwnWindow");
        m_dockAdvanced->setWindowFlags(Qt::Window);
        m_dockAdvanced->show();
    }
}

void TB_TeamBody::indexNumChanged(Pokemon::uniqueId)
{
    if(dockAdvanced())
    {
        int index = sender()->property("num").toInt();
        m_dockAdvanced->updatePokemonNum(index);

        /* When loading a team, in order to restore the index right */
        if (index == 5 && index != body->currentIndex()) {
            m_dockAdvanced->setCurrentPokemon(body->currentWidget()->property("num").toInt());
        }
    }
}

void TB_TeamBody::advancedClicked(bool separateWindow)
{
    advancedClicked(body->currentWidget()->property("num").toInt(), separateWindow);
}

void TB_TeamBody::advancedClicked(int index, bool separateWindow)
{
    if(!dockAdvanced())
    {
        createDockAdvanced(separateWindow);
        dockAdvanced()->setCurrentPokemon(index);
    } else {
        int width = m_dockAdvanced->width();
        m_dockAdvanced->close();
        if (!advSepWindow)
            topLevelWidget()->resize(topLevelWidget()->width()- width, topLevelWidget()->height());
    }
}

void TB_TeamBody::advancedDestroyed()
{
    m_dockAdvanced = 0;
}
