#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "../Utilities/qimagebuttonlr.h"
#include "pokemovesmodel.h"
#include "pokedex.h"
#include "theme.h"
#include "poketablemodel.h"
#include "modelenum.h"
#include "../pokechoice.h"

Pokedex::Pokedex(QWidget *parent, QAbstractItemModel *model)
    : QWidget(parent)
{
    QLabel *pokedexText = new QLabel(this);
    pokedexText->setPixmap(Theme::Pic("Teambuilder/PokeDex/PokeDex.png"));
    pokedexText->move(7,10);
    pokedexText->setFixedSize(pokedexText->pixmap()->size());

    QHBoxLayout *l = new QHBoxLayout(this);
    l->addSpacing(5);

    QVBoxLayout *firstCol = new QVBoxLayout();
    l->addLayout(firstCol);

    firstCol->setSpacing(5);
    firstCol->addSpacing(63);

    QPushButton *type, *damage;
    firstCol->addWidget(type = new QPushButton(tr("&Type Chart")));
    firstCol->addWidget(damage = new QPushButton(tr("&Damage Calculator")));
    firstCol->addStretch(100);
    QLabel *text = new QLabel(toBoldColor("The Pokédex is not complete yet.", Qt::blue));
    text->setWordWrap(true);
    firstCol->addWidget(text);

    damage->setDisabled(true);


    QVBoxLayout *secondCol = new QVBoxLayout();
    l->addLayout(secondCol, 100);
    secondCol->setSpacing(0);

    BigOpenPokeBall *bop = new BigOpenPokeBall();
    secondCol->addWidget(bop,0,Qt::AlignRight);
    PokedexBody *body = new PokedexBody(model);
    secondCol->addWidget(body, 100);

    connect(body, SIGNAL(pokeChanged(Pokemon::uniqueId)), bop, SLOT(changeToPokemon(Pokemon::uniqueId)));
    connect(bop, SIGNAL(pokemonChanged(Pokemon::uniqueId)), body, SLOT(changeToPokemonFromExt(Pokemon::uniqueId)));
    connect(type, SIGNAL(clicked()), SLOT(showTypeChart()));
}

void Pokedex::showTypeChart()
{
    if (typeChart) {
        typeChart->raise();
        typeChart->activateWindow();
        return;
    }
    typeChart = new TypeChart(this);
    typeChart->show();
}

/****************************************************/
/*********** BIG OPEN POKEBALL **********************/
/****************************************************/
BigOpenPokeBall::BigOpenPokeBall()
{
    setPixmap(Theme::Pic("Teambuilder/PokeDex/OMGHUGE.png"));
    setFixedSize(pixmap()->size());

    QHBoxLayout *lll = new QHBoxLayout(this);
    lll->addSpacing(113);
    QGridLayout *ml = new QGridLayout();
    lll->addLayout(ml);
    lll->addSpacing(74);

    ml->setSpacing(1);
    ml->setMargin(0);

    {
        /* Name, num, species */
        QWidget *nameNum = new PokeBallDescBox();
        ml->addWidget(nameNum, 0,0,1,2);

        QVBoxLayout * rows = new QVBoxLayout(nameNum);
        rows->setMargin(2);
        QHBoxLayout * row1 = new QHBoxLayout();
        rows->addLayout(row1);
        QLabel *nball = new QLabel();
        nball->setPixmap(Theme::GreyBall());
        row1->addWidget(nball);
        row1->addWidget(num = new QLabel());

        name = new QLabel();
        name->setObjectName("PokeName");
        row1->addSpacing(5);
        row1->addWidget(name,100,Qt::AlignLeft);
        rows->addStretch(100);

        QHBoxLayout *row2 = new QHBoxLayout();
        rows->addLayout(row2);
        specy = new QLabel();
        specy->setObjectName("PokeSpec");
        row2->addWidget(specy, 0, Qt::AlignRight);
    }

    front = new GridBox();
    ml->addWidget(front, 1,0,3,1);

    PokeBallDescBox *heightWeight = new PokeBallDescBox();
    QVBoxLayout *hWl = new QVBoxLayout(heightWeight);
    hWl->setMargin(3);
    hWl->setSpacing(3);
    hWl->addWidget(height = new QLabel(),0,Qt::AlignLeft);
    hWl->addWidget(weight = new QLabel(),0,Qt::AlignLeft);
    ml->addWidget(heightWeight, 1,1);

    {
        PokeBallDescBox *typeGender = new PokeBallDescBox();
        ml->addWidget(typeGender,2,1,2,2);

        QVBoxLayout *tGl = new QVBoxLayout(typeGender);
        tGl->setMargin(1);
        QHBoxLayout *row1 = new QHBoxLayout();
        tGl->addLayout(row1);
        row1->setMargin(0);
        row1->addSpacing(3);
        row1->setSpacing(1);
        row1->addWidget(new QLabel(tr("Type: ")), 0, Qt::AlignLeft);

        row1->addWidget(type1 = new QLabel());
        row1->addWidget(type2 = new QLabel());

        QHBoxLayout *row2 = new QHBoxLayout();
        tGl->addLayout(row2);
        row2->setMargin(0);
        row2->addSpacing(3);
        row2->setSpacing(3);
        row2->addWidget(new QLabel(tr("Gender: ")), 0, Qt::AlignLeft);
        genderN = new QLabel();
        genderN->setPixmap(Theme::GenderPicture(Pokemon::Neutral, Theme::PokedexM));
        genderM = new QLabel();
        genderM->setPixmap(Theme::GenderPicture(Pokemon::Male, Theme::PokedexM));
        genderF = new QLabel();
        genderF->setPixmap(Theme::GenderPicture(Pokemon::Female, Theme::PokedexM));
        row2->addStretch(100);
        row2->addWidget(genderN);
        row2->addWidget(genderM);
        row2->addWidget(genderF);
        row2->addStretch(100);
    }

    ml->addWidget(evo = new QPushButton(QIcon(Theme::Sprite("greendisc")), tr("&Evolution")), 2,3);
    ml->addWidget(formes = new QPushButton(QIcon(Theme::Sprite("bluedisc")), tr("&Other Formes")), 3,3);
    evo->setCheckable(true);
    evo->setChecked(true);
    formes->setCheckable(true);
    QButtonGroup *evoForme = new QButtonGroup(this);
    evoForme->addButton(evo, 0);
    evoForme->addButton(formes, 1);

    ml->addWidget(back = new GridBox(QPixmap(),true),0,2,2,2);

    QImageButton *toggleUp = Theme::Button("pokedexArrowU");
    QImageButton *toggleDown = Theme::Button("pokedexArrowD");

    toggleUp->setParent(this);
    toggleUp->move(433, 17);
    toggleDown->setParent(this);
    toggleDown->move(439, 23);
    connect(toggleUp, SIGNAL(clicked()), SLOT(changeToPrevious()));
    connect(toggleDown, SIGNAL(clicked()), SLOT(changeToNext()));

    shinyBox = new QCheckBox(tr("Shiny"),this);
    shinyBox->move(435,65);

    changeToPokemon(1);

    connect(shinyBox, SIGNAL(toggled(bool)), SLOT(updatePicture()));
}

void BigOpenPokeBall::changeToPokemon(Pokemon::uniqueId poke)
{
    currentPoke = poke;
    evo->setDisabled(!PokemonInfo::IsInEvoChain(poke));
    formes->setDisabled(!PokemonInfo::HasFormes(poke));

    update();
}

void BigOpenPokeBall::update()
{
    Pokemon::uniqueId n = currentPoke;
    num->setText(QString("%1").arg(n.pokenum));
    name->setText(PokemonInfo::Name(n));
    specy->setText(PokemonInfo::Classification(n));
    height->setText(tr("<b>Ht:</b> %1 m").arg(PokemonInfo::Height(n)));
    weight->setText(tr("<b>Wt:</b> %1 kg").arg(PokemonInfo::WeightS(n)));
    type1->setPixmap(Theme::TypePicture(PokemonInfo::Type1(n)));
    int t2 = PokemonInfo::Type2(n);
    if (t2 != Type::Curse) {
        type2->setPixmap(Theme::TypePicture(t2));
        type2->show();
    } else {
        type2->hide();
    }
    genderF->hide();
    genderN->hide();
    genderM->hide();
    int genderAvail = PokemonInfo::Gender(n);

    if (genderAvail == Pokemon::MaleAvail) {
        genderM->show();
    } else if (genderAvail == Pokemon::FemaleAvail) {
        genderF->show();
    } else if (genderAvail == Pokemon::MaleAndFemaleAvail) {
        genderF->show();
        genderM->show();
    } else {
        genderN->show();
    }

    updatePicture();
}

void BigOpenPokeBall::changeToNext()
{
    if (formes->isChecked()) {
        if (formes->isEnabled()) {
            QList<Pokemon::uniqueId> formes = PokemonInfo::Formes(currentPoke);
            for (int i = 0; i < formes.size() - 1; i++) {
                if (formes[i] == currentPoke) {
                    emit pokemonChanged(formes[i+1]);
                    return;
                }
            }
            emit pokemonChanged(formes[0]);
            return;
        }
    } else if (evo->isChecked() && evo->isEnabled()) {
        QList<int> formes = PokemonInfo::Evos(currentPoke.pokenum);
        for (int i = 0; i < formes.size() - 1; i++) {
            if (formes[i] == currentPoke.pokenum) {
                emit pokemonChanged(Pokemon::uniqueId(formes[i+1]));
                return;
            }
        }
        emit pokemonChanged(Pokemon::uniqueId(formes[0]));
        return;
    }
}


void BigOpenPokeBall::changeToPrevious()
{
    if (formes->isChecked()) {
        if (formes->isEnabled()) {
            QList<Pokemon::uniqueId> formes = PokemonInfo::Formes(currentPoke);
            for (int i = 1; i < formes.size(); i++) {
                if (formes[i] == currentPoke) {
                    emit pokemonChanged(formes[i-1]);
                    return;
                }
            }
            emit pokemonChanged(formes[formes.size()-1]);
            return;
        }
    }else if (evo->isChecked() && evo->isEnabled()) {
        QList<int> formes = PokemonInfo::Evos(currentPoke.pokenum);
        for (int i = 1; i < formes.size(); i++) {
            if (formes[i] == currentPoke.pokenum) {
                emit pokemonChanged(Pokemon::uniqueId(formes[i-1]));
                return;
            }
        }
        emit pokemonChanged(Pokemon::uniqueId(formes[formes.size()-1]));
        return;
    }
}

void BigOpenPokeBall::updatePicture()
{
    front->changePic(PokemonInfo::Picture(currentPoke, 5, PokemonInfo::BaseGender(currentPoke),shiny(),false));
    back->changePic(PokemonInfo::Picture(currentPoke, 5, PokemonInfo::BaseGender(currentPoke),shiny(),true));
}

bool BigOpenPokeBall::shiny() const
{
    return shinyBox->isChecked();
}

/****************************************************/
/*********** POKEDEX BODY ***************************/
/****************************************************/
PokedexBody::PokedexBody(QAbstractItemModel *pokeModel)
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setMargin(0);
    QVBoxLayout *col1 = new QVBoxLayout();
    col1->setSpacing(2);
    col1->setMargin(5);
    hl->addLayout(col1);
    QPushButton *advSearch;
    col1->addWidget(advSearch = new QPushButton(QIcon(Theme::Sprite("orangedisc")), tr("&Advanced Search")));
    col1->addWidget(pokeEdit = new QLineEdit());
    pokeEdit->setAccessibleName(tr("Pokemon search field", "TB accessible name"));
    pokeList = new TB_PokeChoice(pokeModel, false);
    pokeList->setAccessibleName(tr("Pokemon list", "TB accessible name"));
    pokeList->verticalHeader()->setDefaultSectionSize(30);
    QCompleter *comp = new QCompleter(pokeEdit);
    comp->setModel(pokeList->model());
    comp->setCompletionColumn(1);
    comp->setCompletionRole(Qt::DisplayRole);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    pokeEdit->setCompleter(comp);
    col1->addWidget(pokeList,100);
    col1->addWidget(new PokeBallText(Theme::OrangeBall(), tr("Sort Pokemon List")));

    connect(comp, SIGNAL(activated(QString)), this, SLOT(changeToPokemon(QString)));
    connect(pokeEdit, SIGNAL(returnPressed()), SLOT(changePokemon()));
    connect(pokeList, SIGNAL(activated(QModelIndex)), SLOT(changePokemonFromRow(QModelIndex)));
    connect(advSearch, SIGNAL(clicked()), SLOT(openAdvancedSearch()));

    /* Buttons at the bottom */
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setMargin(0);
    QPushButton *sortByAlph, *sortByNum;
    buttons->addWidget(sortByAlph = new QPushButton(tr("A-Z")));
    sortByAlph->setAccessibleName(tr("Sort pokemon alphabetically", "TB accessible name"));
    buttons->addWidget(sortByNum = new QPushButton(QString("%1 - %2").arg(1).arg(PokemonInfo::TrueCount()-1)));
    sortByNum->setAccessibleName(tr("Sort pokemon by National Pokedex number", "TB accessible name"));
    sortByAlph->setCheckable(true);
    sortByNum->setCheckable(true);
    sortByNum->setChecked(true);
    QButtonGroup *g = new QButtonGroup(this);
    g->addButton(sortByAlph, SortByAlph);
    g->addButton(sortByNum, SortByNum);

    connect(g, SIGNAL(buttonClicked(int)), this, SLOT(sortByColumn(int)));

    col1->addLayout(buttons);

    QLabel *sep = new QLabel();
    sep->setObjectName("Separator");
    hl->addWidget(sep);


    QTabWidget *tabw = new QTabWidget();
    tabw->setObjectName("Modified");
    hl->addWidget(tabw, 100);

    ProfileTab *pt;
    StatTab *st;
    MoveTab *mt;
    tabw->addTab(pt = new ProfileTab(), tr("PROFILE"));
    tabw->addTab(st = new StatTab(), tr("STATS"));
    tabw->addTab(mt = new MoveTab(), tr("MOVES"));

    connect(this, SIGNAL(pokeChanged(Pokemon::uniqueId)), pt, SLOT(changeDesc(Pokemon::uniqueId)));
    connect(this, SIGNAL(pokeChanged(Pokemon::uniqueId)), st, SLOT(changePoke(Pokemon::uniqueId)));
    connect(this, SIGNAL(pokeChanged(Pokemon::uniqueId)), mt, SLOT(changePoke(Pokemon::uniqueId)));

    changeToPokemon(1);
}

void PokedexBody::sortByColumn(int col)
{
    pokeList->sortByColumn(col, Qt::AscendingOrder);
}

void PokedexBody::changeToPokemon(const QString &poke)
{
    Pokemon::uniqueId num = PokemonInfo::Number(poke);

    if (num != Pokemon::uniqueId(Pokemon::NoPoke) && currentPoke != num) {
        currentPoke = num;
        emit pokeChanged(num);

        changeToPokemon(num);
    }
}

void PokedexBody::changeToPokemonFromExt(Pokemon::uniqueId poke)
{
    Pokemon::uniqueId num = poke;

    if (num != Pokemon::uniqueId(Pokemon::NoPoke) && currentPoke != num) {
        currentPoke = num;
        emit pokeChanged(num);

        changeToPokemon(num);
    }
}

void PokedexBody::changePokemon()
{
    changeToPokemon(pokeEdit->text());
}

void PokedexBody::changeToPokemon(Pokemon::uniqueId poke)
{
    pokeEdit->setText(PokemonInfo::Name(poke));
}

void PokedexBody::changePokemonFromRow(const QModelIndex &index)
{
    changeToPokemon(index.data(CustomModel::PokenameRole).toString());
}

void PokedexBody::openAdvancedSearch()
{
    if (aSearch) {
        aSearch->raise();
        aSearch->activateWindow();
        return;
    }

    aSearch = new AdvancedSearch();
    aSearch->show();
    connect(aSearch, SIGNAL(pokeSelected(Pokemon::uniqueId)), SLOT(changeToPokemonFromExt(Pokemon::uniqueId)));
    connect(this, SIGNAL(destroyed()), aSearch, SLOT(close()));
}

/****************************************************/
/*********** PROFILE TAB ****************************/
/****************************************************/

ProfileTab::ProfileTab()
{
    QVBoxLayout *firstCol = new QVBoxLayout(this);

    QLabel *descHeader = new QLabel(tr("Description"));
    descHeader->setObjectName("BlueHeader");
    firstCol->addWidget(descHeader);
    ssDesc = new QLabel();
    ssDesc->setWordWrap(true);
    hgDesc = new QLabel();
    hgDesc->setWordWrap(true);;
    ptDesc = new QLabel();
    ptDesc->setWordWrap(true);
    ptDesc->hide();

    firstCol->addWidget(ptDesc);
    firstCol->addWidget(ssDesc);
    firstCol->addWidget(hgDesc);

    QLabel *abilityHeader = new QLabel(tr("Abilities"));
    abilityHeader->setObjectName("GreenHeader");
    firstCol->addWidget(abilityHeader);

    for (int i = 0; i < 3; i++) {
        firstCol->addWidget(abs[i] = new QLabel());
        abs[i]->setWordWrap(true);
    }

    changeDesc(1);
}

void ProfileTab::changeDesc(Pokemon::uniqueId poke)
{
    QString hg = PokemonInfo::Desc(poke, Version::HeartGold);
    QString ss = PokemonInfo::Desc(poke, Version::SoulSilver);
    QString pt = PokemonInfo::Desc(poke, Version::Platinum);

    ssDesc->setText(tr("<b>Soul Silver:</b> %1").arg(ss));
    hgDesc->setText(tr("<b>Heart Gold:</b> %1").arg(hg));
    ptDesc->setText(tr("<b>Platinum:</b> %1").arg(pt));

    if (ss == hg) {
        hgDesc->hide();
        ptDesc->show();
    } else {
        hgDesc->show();
        ptDesc->hide();
    }

    AbilityGroup ab = PokemonInfo::Abilities(poke);

    abs[0]->setText(QString("<b>%1</b> - %2").arg(AbilityInfo::Name(ab.ab(0)), AbilityInfo::Desc(ab.ab(0))));

    for (int i = 1; i < 3; i++) {
        if (ab.ab(i) == 0) {
            abs[i]->hide();
        } else {
            abs[i]->show();
            abs[i]->setText(QString("<b>%1</b> - %2").arg(AbilityInfo::Name(ab.ab(i)), AbilityInfo::Desc(ab.ab(i))));
        }
    }
}

/****************************************************/
/*********** STATTAB ********************************/
/****************************************************/

StatTab::StatTab() {
    QVBoxLayout *vl = new QVBoxLayout(this);

    QGridLayout *stats = new QGridLayout();

    vl->addLayout(stats);

    stats->setMargin(0);
    stats->setSpacing(4);

    QLabel *showoff = new QLabel(tr("Statistics"));
    showoff->setObjectName("Title");
    stats->addWidget(showoff, 0,0);

    QLabel *title1, *title2, *title3;
    stats->addWidget(title1 = new QLabel(tr("Base Stats")), 0, 1);
    stats->addWidget(title2 = new QLabel(tr("Min")), 0, 2);
    QLabel *icon = new QLabel();
    icon->setPixmap(Theme::Pic("Teambuilder/Team/=.png"));
    stats->addWidget(icon, 0, 3);
    stats->addWidget(title3 = new QLabel(tr("Max")), 0, 4);

    title1->setObjectName("Title");
    title2->setObjectName("Title");
    title3->setObjectName("Title");

    QStringList statLabels =
            QStringList() << tr("Hit Points") << tr("Attack") << tr("Defense") << tr("Special Attack") << tr("Special Defense") << tr("Speed");

    /*QFrame *greenBG = new QFrame();
    greenBG->setObjectName("GreenBackground");
    stats->addWidget(greenBG, 1, 2, 6, 3);*/
    for (int i = 0; i < 6; i++) {
        stats->addWidget(new QLabel(statLabels[i]), i+1, 0);
        stats->addWidget(baseStats[i] = new QProgressBar(), i+1, 1);
        stats->addWidget(min[i] = new QLabel(), i+1, 2);
        if (i != Hp) {
            stats->addWidget(buttons[i] = Theme::LRButton("equal"), i+1, 3);
            buttons[i]->setProperty("Stat", i);

            connect(buttons[i], SIGNAL(leftClick()), SLOT(increaseBoost()));
            connect(buttons[i], SIGNAL(rightClick()), SLOT(decreaseBoost()));
        }
        stats->addWidget(max[i] = new QLabel(), i+1, 4);

        min[i]->setObjectName("Stat");
        max[i]->setObjectName("Stat");
    }

    weakImmTab = new QTabWidget();
    vl->addWidget(weakImmTab);

    changePoke(1);
}

void StatTab::changePoke(Pokemon::uniqueId poke) {
    this->poke = poke;

    for (int i = 1; i < 6; i++) {
        if (boost[i] != 0)  {
            Theme::ChangePics(buttons[i], "equal");
            boost[i] = 0;
        }
    }

    int ranges[] = {30, 50, 60, 70, 80, 90, 100, 200, 300};
    QString colors[] = {"#ff0505", "#fd5300", "#ff7c49", "#ffaf49", "#ffd749", "#b9d749", "#5ee70a", "#3093ff", "#6c92bd"};
    for (int i = 0; i < 6; i++) {
        int bs = PokemonInfo::BaseStats(poke).baseStat(i);

        baseStats[i]->setValue(bs%100);
        baseStats[i]->setFormat(QString("%1").arg(bs));

        QString bg = bs < 100 ? "white" : (bs < 200 ? "#30c7ff" : "#3093ff");

        QString color;
        for (unsigned j = 0; j < sizeof(ranges)/sizeof(int); j++) {
            if (bs < ranges[j]) {
                color = colors[j];
                break;
            }
        }
        baseStats[i]->setStyleSheet(QString("QProgressBar{	background: %1;"
                                    "border: 1px solid #393737;"
                                    "height: 14px;"
                                    "text-align: center;}"
                                    "QProgressBar::chunk {"
                                    " width: 1px;"
                                    " background-color: %2;"
                                    "}").arg(bg, color));

        min[i]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, 5, NatureInfo::NatureOf(i > 1 ? 1 : 2, i), i, 100, 31, 0)));
        max[i]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, 5, NatureInfo::NatureOf(i, i > 1 ? 1 : 2), i, 100, 31, 252)));
    }

    int t1 = PokemonInfo::Type1(poke);
    int t2 = PokemonInfo::Type2(poke);

    weakImmTab->clear();

    QFrame *weaknesses = new QFrame();
    weaknesses->setObjectName("WeakResFrame");
    weakImmTab->addTab(weaknesses, tr("Weakness"));
    QGridLayout *wl = new QGridLayout(weaknesses);
    wl->setMargin(4);
    wl->setSpacing(5);

    int wCount = 0;
    for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
        int eff = TypeInfo::Eff(i,t1) * TypeInfo::Eff(i, t2);
        if (eff > 4) {
            wl->addWidget(new TypeText(i, QString("x %1").arg(eff/4)), wCount/4, wCount%4);
            wCount += 1;
        }
    }

    QFrame *res = new QFrame();
    res->setObjectName("WeakResFrame");
    weakImmTab->addTab(res, tr("Resistance"));
    QGridLayout *rl = new QGridLayout(res);
    rl->setMargin(4);
    rl->setSpacing(5);

    int rCount = 0;
    for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
        int eff = TypeInfo::Eff(i,t1) * TypeInfo::Eff(i, t2);
        if (eff < 4 && eff > 0) {
            rl->addWidget(new TypeText(i, QString("/ %1").arg(4/eff)), rCount/4, rCount%4);
            rCount += 1;
        }
    }

    QFrame *imm = new QFrame();
    imm->setObjectName("WeakResFrame");
    weakImmTab->addTab(imm, tr("Immunity"));
    QGridLayout *il = new QGridLayout(imm);
    il->setMargin(4);
    il->setSpacing(5);

    int iCount = 0;
    for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
        int eff = TypeInfo::Eff(i,t1) * TypeInfo::Eff(i, t2);
        if (eff == 0) {
            QLabel *typePic = new QLabel();
            typePic->setPixmap(Theme::TypePicture(i));
            il->addWidget(typePic, iCount/4, iCount%4, 1, 1, Qt::AlignHCenter);
            iCount += 1;
        }
    }
}

void StatTab::increaseBoost()
{
    int stat = sender()->property("Stat").toInt();

    if (boost[stat] == 6) {
        return;
    }

    boost[stat] += 1;

    min[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, 5, NatureInfo::NatureOf(stat > 1 ? 1 : 2, stat), stat, 100, 31, 0) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));
    max[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, 5, NatureInfo::NatureOf(stat, stat > 1 ? 1 : 2), stat, 100, 31, 252) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));

    if (boost[stat] < 0) {
        return;
    }
    if (boost[stat] == 0) {
        Theme::ChangePics(buttons[stat], "equal");
    } else if (boost[stat] == 1) {
        Theme::ChangePics(buttons[stat], "plus");
    }
}


void StatTab::decreaseBoost()
{
    int stat = sender()->property("Stat").toInt();

    if (boost[stat] == -6) {
        return;
    }

    boost[stat] -= 1;

    min[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, 5, NatureInfo::NatureOf(stat > 1 ? 1 : 2, stat), stat, 100, 31, 0) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));
    max[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, 5, NatureInfo::NatureOf(stat, stat > 1 ? 1 : 2), stat, 100, 31, 252) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));

    if (boost[stat] > 0) {
        return;
    }
    if (boost[stat] == 0) {
        Theme::ChangePics(buttons[stat], "equal");
    } else if (boost[stat] == -1) {
        Theme::ChangePics(buttons[stat], "minus");
    }
}

/****************************************************/
/*********** Move Tab *******************************/
/****************************************************/

MoveTab::MoveTab()
{
    QVBoxLayout *v = new QVBoxLayout(this);
    moves = new QTableView();
    moves->setAccessibleName(tr("Pokemon moves", "TB accessible name"));
    QSortFilterProxyModel *filter=  new QSortFilterProxyModel(this);
    filter->setSourceModel(movesModel = new PokeMovesModel(1, GEN_MAX, this));
    moves->setModel(filter);
    moves->hideColumn(PokeMovesModel::Learning);

    v->addWidget(moves);
    moves->setIconSize(Theme::TypePicture(Type::Normal).size());
    moves->verticalHeader()->hide();
    moves->setShowGrid(false);
    moves->setSelectionBehavior(QAbstractItemView::SelectRows);
    moves->setSelectionMode(QAbstractItemView::SingleSelection);

    moves->horizontalHeader()->setStretchLastSection(true);
    moves->horizontalHeader()->resizeSection(PokeMovesModel::Type, Theme::TypePicture(Type::Normal).width()+5);
    moves->horizontalHeader()->resizeSection(PokeMovesModel::PP, 25);
    moves->horizontalHeader()->resizeSection(PokeMovesModel::Pow, 32);
    moves->horizontalHeader()->resizeSection(PokeMovesModel::Acc, 32);
    moves->horizontalHeader()->resizeSection(PokeMovesModel::Name, 100);
}

void MoveTab::changePoke(Pokemon::uniqueId poke)
{
    movesModel->setPokemon(poke, GEN_MAX);
}

/****************************************************/
/*********** QGRIDBOX *******************************/
/****************************************************/

GridBox::GridBox(const QPixmap &pic, bool shiftToBottom)
{
    setPixmap(Theme::Sprite("grid"));
    setFixedSize(pixmap()->size());

    underLying = new QLabel(this);
    underLying->setPixmap(pic);
    if (shiftToBottom)
        underLying->move(7-8,7-16);
    else
        underLying->move(7-8,3-16);
}

void GridBox::changePic(const QPixmap &pic)
{
    underLying->setPixmap(pic);
}

/******************************************************/
/*********** POKEBALL TEXT ****************************/
/*****************************************************/

PokeBallText::PokeBallText(const QPixmap &pic, const QString &text)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    QLabel *l = new QLabel();
    l->setPixmap(pic);
    layout->addWidget(l);
    layout->addWidget(new QLabel(text));
    layout->addStretch(100);
}


/******************************************************/
/*************** TYPE TEXT ****************************/
/******************************************************/

TypeText::TypeText(int type, const QString &text)
{
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setMargin(0);
    hl->setSpacing(5);

    QLabel *typeL = new QLabel();
    typeL->setPixmap(Theme::TypePicture(type));
    QLabel *textL = new QLabel();
    textL->setText(text);

    hl->addWidget(typeL,0,Qt::AlignRight);
    hl->addWidget(textL,0,Qt::AlignLeft);
}

/*********************************************************/
/*************** TYPE CHART ******************************/
/*********************************************************/

TypeChart::TypeChart(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::Window);
    QVBoxLayout *vl = new QVBoxLayout(this);
    QCompactTable *gl = new QCompactTable(TypeInfo::NumberOfTypes(), TypeInfo::NumberOfTypes());
    vl->addWidget(gl);
    gl->setIconSize(QSize(48,19));
    gl->verticalHeader()->hide();
    gl->horizontalHeader()->hide();
    gl->setShowGrid(true);

    for (int i = 0; i < TypeInfo::NumberOfTypes(); i++) {
        gl->horizontalHeader()->resizeSection(i, 54);
    }

    gl->setItem(0,0, new QTableWidgetItem(tr("A \\ D")));

    for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
        QTableWidgetItem *t1, *t2;
        t1 = new QTableWidgetItem();
        t2 = new QTableWidgetItem();
        t1->setIcon(QIcon(Theme::TypePicture(i)));
        t2->setIcon(QIcon(Theme::TypePicture(i)));

        gl->setItem(0, i+1, t1);
        gl->setItem(i+1, 0, t2);
    }

    for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
        for (int j = 0; j < TypeInfo::NumberOfTypes() - 1; j++) {
            int eff = TypeInfo::Eff(i, j);
            if (eff == 2) {
                continue;
            }
            QTableWidgetItem *l = new QTableWidgetItem();
            if (eff == 0) {
                l->setText(" 0 ×");
                l->setForeground(Qt::white);
                l->setBackgroundColor(Qt::black);
            } else if (eff == 1) {
                l->setText(" ½ ×");
                l->setForeground(Qt::white);;
                l->setBackgroundColor(Qt::red);
            } else if (eff == 4) {
                l->setText(" 2 ×");
                l->setForeground(Qt::white);
                l->setBackgroundColor(QColor(0,153,0));
            }
            gl->setItem(i+1, j+1, l);
        }
    }

    resize(55* TypeInfo::NumberOfTypes()+10, 24*TypeInfo::NumberOfTypes()-3);
}

/*****************************************************************
  ****************** ADVANCED SEARCH *****************************
  ****************************************************************/

AdvancedSearch::AdvancedSearch()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QHBoxLayout *hl = new QHBoxLayout(this);

    QVBoxLayout *col1 = new QVBoxLayout();
    hl->addLayout(col1);

    QGroupBox *types = new QGroupBox(tr("Types"));
    col1->addWidget(types);
    QGridLayout *typeL = new QGridLayout(types);

    typeBoxes[0] = new QCheckBox(tr("Type 1"));
    typeBoxes[1] = new QCheckBox(tr("Type 2"));

    typeCb[0] = new QComboBox();
    typeCb[1] = new QComboBox();

    for (int i = 0; i < TypeInfo::NumberOfTypes() - 1; i++) {
        typeCb[0]->addItem(TypeInfo::Name(i));
        typeCb[1]->addItem(TypeInfo::Name(i));
    }

    typeL->addWidget(typeBoxes[0], 0, 0);
    typeL->addWidget(typeBoxes[1], 1, 0);
    typeL->addWidget(typeCb[0], 0, 1);
    typeL->addWidget(typeCb[1], 1, 1);

    QGroupBox *abilities = new QGroupBox(tr("Ability"));
    QVBoxLayout *v = new QVBoxLayout(abilities);

    abilityCb = new QComboBox();
    v->addWidget(abilityCb);
    QStringList abs;
    for (int i = 0; i < AbilityInfo::NumberOfAbilities(); i++) {
        abs.push_back(AbilityInfo::Name(i));
    }
    abs.sort();
    abilityCb->addItems(abs);
    col1->addWidget(abilities);

    QGroupBox *statB = new QGroupBox(tr("Base Stats"));
    QGridLayout *gl = new QGridLayout(statB);
    for (int i = 0; i < 6; i++) {
        gl->addWidget(new QLabel(StatInfo::Stat(i)), i, 0);
        statSymbols[i] = new QComboBox();
        statSymbols[i]->addItem(tr(" "));
        statSymbols[i]->addItem(tr(">="));
        statSymbols[i]->addItem(tr("="));
        statSymbols[i]->addItem(tr("<="));
        gl->addWidget(statSymbols[i], i, 1);
        stats[i] = new QLineEdit();
        gl->addWidget(stats[i], i, 2);
    }
    col1->addWidget(statB);

    QVBoxLayout *col2 = new QVBoxLayout();
    hl->addLayout(col2);

    QGroupBox *moves = new QGroupBox(tr("Moves"));

    QGridLayout *moveL = new QGridLayout (moves);
    QCompleter *p = new QCompleter(MoveInfo::MoveList(), this);
    p->setCaseSensitivity(Qt::CaseInsensitive);

    for (int i =0 ; i < 4; i++) {
        move[i] = new QLineEdit();
        move[i]->setCompleter(p);
        moveL->addWidget(move[i], i/2, i%2);
    }

    col2->addWidget(moves);
    QPushButton *search;
    col2->addWidget(search = new QPushButton(tr("&Search !")));
    connect(search, SIGNAL(clicked()), SLOT(search()));

    QGroupBox *results = new QGroupBox(tr("&Results"));
    this->results = new QListWidget();
    QVBoxLayout *vv = new QVBoxLayout(results);
    vv->addWidget(this->results);
    col2->addWidget(results);

    connect(this->results, SIGNAL(activated(QModelIndex)), SLOT(pokeClicked(QModelIndex)));
}

void AdvancedSearch::pokeClicked(QModelIndex i)
{
    emit pokeSelected(PokemonInfo::Number(results->item(i.row())->text()));
}

void AdvancedSearch::search()
{
    QVector<int> types;
    QSet<int> moves;
    int ability;
    QVector<QPair<int,int> > equalStats;
    QVector<QPair<int,int> >  minStats;
    QVector<QPair<int,int> >  maxStats;

    if (typeBoxes[0]->isChecked()) {
        types.push_back(typeCb[0]->currentIndex());
    }
    if (typeBoxes[1]->isChecked()) {
        types.push_back(typeCb[1]->currentIndex());
    }
    ability = AbilityInfo::Number(abilityCb->currentText());
    for(int i = 0; i < 4; i++) {
        moves.insert(MoveInfo::Number(move[i]->text()));
    }
    moves.remove(0);

    for (int i = 0; i < 6; i++) {
        if (statSymbols[i]->currentIndex() == 1) {
            minStats.push_back(QPair<int, int> (i,stats[i]->text().toInt()));
        } else if (statSymbols[i]->currentIndex() == 2) {
            equalStats.push_back(QPair<int, int> (i,stats[i]->text().toInt()));
        } else if (statSymbols[i]->currentIndex() == 3) {
            maxStats.push_back(QPair<int, int> (i,stats[i]->text().toInt()));
        }
    }

    QSet<int> resultingList;

    for (int i = 1; i < PokemonInfo::NumberOfPokemons(); i++) {
        if (moves.size() > 0 && !PokemonInfo::Moves(i).contains(moves)) {
            continue;
        }
        for (int j = 0; j < types.size(); j++) {
            if (PokemonInfo::Type1(i) != types[j] && PokemonInfo::Type2(i) != types[j])
                goto loopend;
        }

        {
            AbilityGroup ab = PokemonInfo::Abilities(i);
            if (ability != 0 && ab.ab(0) != ability && ab.ab(1) != ability && ab.ab(2) != ability)
                goto loopend;
            {
                PokeBaseStats b = PokemonInfo::BaseStats(i);
                for (int j = 0; j < equalStats.size(); j++) {
                    if (b.baseStat(equalStats[j].first) != equalStats[j].second)
                        goto loopend;
                }
                for (int j = 0; j < minStats.size(); j++) {
                    if (b.baseStat(minStats[j].first) < minStats[j].second)
                        goto loopend;
                }
                for (int j = 0; j < maxStats.size(); j++) {
                    if (b.baseStat(maxStats[j].first) > maxStats[j].second)
                        goto loopend;
                }
            }
        }

        resultingList.insert(i);
        loopend:
        ;
    }

    results->clear();
    foreach(int poke, resultingList) {
        results->addItem(PokemonInfo::Name(poke));
    }
    results->sortItems(Qt::AscendingOrder);
}
