#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "pokedex.h"
#include "teambuilder.h"

Pokedex::Pokedex(TeamBuilder *parent)
    : QWidget(parent)
{
    QLabel *pokedexText = new QLabel(this);
    pokedexText->setPixmap(QPixmap("db/Teambuilder/PokeDex/PokeDex.png"));
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
    PokedexBody *body = new PokedexBody();
    secondCol->addWidget(body, 100);

    connect(body, SIGNAL(pokeChanged(int)), bop, SLOT(changeToPokemon(int)));
    connect(bop, SIGNAL(pokemonChanged(int)), body, SLOT(changeToPokemonFromExt(int)));
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
    setPixmap(QPixmap("db/Teambuilder/PokeDex/OMGHUGE.png"));
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
        nball->setPixmap(QPixmap("db/Teambuilder/PokeDex/NumberBall.png"));
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
        genderN->setPixmap(QPixmap("db/Teambuilder/PokeDex/gender0.png"));
        genderM = new QLabel();
        genderM->setPixmap(QPixmap("db/Teambuilder/PokeDex/gender1.png"));
        genderF = new QLabel();
        genderF->setPixmap(QPixmap("db/Teambuilder/PokeDex/gender2.png"));
        row2->addStretch(100);
        row2->addWidget(genderN);
        row2->addWidget(genderM);
        row2->addWidget(genderF);
        row2->addStretch(100);
    }

    ml->addWidget(evo = new QPushButton(QIcon("db/Teambuilder/PokeDex/EvoIcon.png"), tr("&Evolution")), 2,3);
    ml->addWidget(formes = new QPushButton(QIcon("db/Teambuilder/PokeDex/FormeIcon.png"), tr("&Other Formes")), 3,3);
    evo->setCheckable(true);
    evo->setChecked(true);
    formes->setCheckable(true);
    QButtonGroup *evoForme = new QButtonGroup(this);
    evoForme->addButton(evo, 0);
    evoForme->addButton(formes, 1);

    ml->addWidget(back = new GridBox(QPixmap(),true),0,2,2,2);

    QImageButton *toggleUp =
            new QImageButton("db/Teambuilder/PokeDex/Toggle buttons/UpArrowNormal.png", "db/Teambuilder/PokeDex/Toggle Buttons/UpArrowGlow.png");
    QImageButton *toggleDown =
            new QImageButton("db/Teambuilder/PokeDex/Toggle buttons/DownArrowNormal.png", "db/Teambuilder/PokeDex/Toggle Buttons/DownArrowGlow.png");

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

void BigOpenPokeBall::changeToPokemon(int poke)
{
    forme = 0;
    currentPoke = poke;
    evo->setDisabled(!PokemonInfo::IsInEvoChain(poke));
    formes->setDisabled(!PokemonInfo::HasAestheticFormes(poke) && !PokemonInfo::HasFormes(poke));

    update();
}

void BigOpenPokeBall::update()
{
    int n = currentPoke;
    num->setText(QString("%1").arg(n));
    name->setText(PokemonInfo::Name(n));
    specy->setText(PokemonInfo::Classification(n));
    height->setText(tr("<b>Ht:</b> %1").arg(PokemonInfo::Height(n)));
    weight->setText(tr("<b>Wt:</b> %1 lbs").arg(PokemonInfo::WeightS(n)));
    type1->setPixmap(TypeInfo::Picture(PokemonInfo::Type1(n)));
    int t2 = PokemonInfo::Type2(n);
    if (t2 != Type::Curse) {
        type2->setPixmap(TypeInfo::Picture(t2));
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
            if (PokemonInfo::HasAestheticFormes(currentPoke)) {
                if (forme + 1 == PokemonInfo::NumberOfAFormes(currentPoke)) {
                    forme = 0;
                    updatePicture();
                    return;
                }
                forme += 1;
                if (PokemonInfo::AestheticDesc(currentPoke, forme) != "") {
                    updatePicture();
                    return;
                } else {
                    changeToNext();
                }
            } else {
                QList<int> formes = PokemonInfo::Formes(currentPoke);
                for (int i = 0; i < formes.size() - 1; i++) {
                    if (formes[i] == currentPoke) {
                        emit pokemonChanged(formes[i+1]);
                        return;
                    }
                }
                emit pokemonChanged(formes[0]);
                return;
            }
        }
    } else if (evo->isChecked() && evo->isEnabled()) {
        QList<int> formes = PokemonInfo::Evos(currentPoke);
        for (int i = 0; i < formes.size() - 1; i++) {
            if (formes[i] == currentPoke) {
                emit pokemonChanged(formes[i+1]);
                return;
            }
        }
        emit pokemonChanged(formes[0]);
        return;
    }
}


void BigOpenPokeBall::changeToPrevious()
{
    if (formes->isChecked()) {
        if (formes->isEnabled()) {
            if (PokemonInfo::HasAestheticFormes(currentPoke)) {
                if (forme == 0) {
                    forme = PokemonInfo::NumberOfAFormes(currentPoke) - 1;
                    updatePicture();
                    return;
                }
                forme -= 1;
                if (PokemonInfo::AestheticDesc(currentPoke, forme) != "") {
                    updatePicture();
                    return;
                } else {
                    changeToPrevious();
                }
            } else {
                QList<int> formes = PokemonInfo::Formes(currentPoke);
                for (int i = 1; i < formes.size(); i++) {
                    if (formes[i] == currentPoke) {
                        emit pokemonChanged(formes[i-1]);
                        return;
                    }
                }
                emit pokemonChanged(formes[formes.size()-1]);
                return;
            }
        }
    }else if (evo->isChecked() && evo->isEnabled()) {
        QList<int> formes = PokemonInfo::Evos(currentPoke);
        for (int i = 1; i < formes.size(); i++) {
            if (formes[i] == currentPoke) {
                emit pokemonChanged(formes[i-1]);
                return;
            }
        }
        emit pokemonChanged(formes[formes.size()-1]);
        return;
    }
}

void BigOpenPokeBall::updatePicture()
{
    front->changePic(PokemonInfo::Picture(currentPoke, forme, PokemonInfo::BaseGender(currentPoke),shiny(),false));
    back->changePic(PokemonInfo::Picture(currentPoke, forme, PokemonInfo::BaseGender(currentPoke),shiny(),true));
}

bool BigOpenPokeBall::shiny() const
{
    return shinyBox->isChecked();
}

/****************************************************/
/*********** POKEDEX BODY ***************************/
/****************************************************/
PokedexBody::PokedexBody()
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setMargin(0);
    QVBoxLayout *col1 = new QVBoxLayout();
    col1->setSpacing(2);
    col1->setMargin(5);
    hl->addLayout(col1);
    QPushButton *advSearch;
    col1->addWidget(advSearch = new QPushButton(QIcon("db/Teambuilder/PokeDex/advsrchicon.png"), tr("&Advanced Search")));
    col1->addWidget(pokeEdit = new QLineEdit());
    pokeList = new TB_PokeChoice(4, false);
    pokeList->verticalHeader()->setDefaultSectionSize(30);
    QCompleter *comp = new QCompleter(pokeEdit);
    comp->setModel(pokeList->model());
    comp->setCompletionColumn(1);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    pokeEdit->setCompleter(comp);
    col1->addWidget(pokeList,100);
    col1->addWidget(new PokeBallText("db/Teambuilder/PokeDex/Orangeball.png", tr("Sort Pokemon List")));

    connect(comp, SIGNAL(activated(QString)), this, SLOT(changeToPokemon(QString)));
    connect(pokeEdit, SIGNAL(returnPressed()), SLOT(changePokemon()));
    connect(pokeList, SIGNAL(cellActivated(int,int)), SLOT(changePokemonFromRow(int)));
    connect(advSearch, SIGNAL(clicked()), SLOT(openAdvancedSearch()));

    /* Buttons at the bottom */
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setMargin(0);
    QPushButton *sortByAlph, *sortByNum;
    buttons->addWidget(sortByAlph = new QPushButton(tr("A-Z")));
    buttons->addWidget(sortByNum = new QPushButton(QString("%1 - %2").arg(1).arg(PokemonInfo::TrueCount()-1)));
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

    connect(this, SIGNAL(pokeChanged(int)), pt, SLOT(changeDesc(int)));
    connect(this, SIGNAL(pokeChanged(int)), st, SLOT(changePoke(int)));
    connect(this, SIGNAL(pokeChanged(int)), mt, SLOT(changePoke(int)));

    changeToPokemon(1);
}

void PokedexBody::sortByColumn(int col)
{
    pokeList->sortByColumn(col, Qt::AscendingOrder);
}

void PokedexBody::changeToPokemon(const QString &poke)
{
    int num = PokemonInfo::Number(poke);

    if (num != 0 && currentPoke != num) {
        currentPoke = num;
        emit pokeChanged(num);

        changeToPokemon(num);
    }
}

void PokedexBody::changeToPokemonFromExt(int poke)
{
    int num = poke;

    if (num != 0 && currentPoke != num) {
        currentPoke = num;
        emit pokeChanged(num);

        changeToPokemon(num);
    }
}

void PokedexBody::changePokemon()
{
    changeToPokemon(pokeEdit->text());
}

void PokedexBody::changeToPokemon(int poke)
{
    pokeEdit->setText(PokemonInfo::Name(poke));
}

void PokedexBody::changePokemonFromRow(int row)
{
    changeToPokemon(pokeList->item(row, SortByAlph)->text());
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
    connect(aSearch, SIGNAL(pokeSelected(int)), SLOT(changeToPokemonFromExt(int)));
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
    firstCol->addWidget(ab1 = new QLabel());
    firstCol->addWidget(ab2 = new QLabel());
    ab1->setWordWrap(true);
    ab2->setWordWrap(true);

    changeDesc(1);
}

void ProfileTab::changeDesc(int poke)
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

    ab1->setText(QString("<b>%1</b> - %2").arg(AbilityInfo::Name(ab.ab1), AbilityInfo::Desc(ab.ab1)));

    if (ab.ab2 == 0) {
        ab2->hide();
    } else {
        ab2->setText(QString("<b>%1</b> - %2").arg(AbilityInfo::Name(ab.ab2), AbilityInfo::Desc(ab.ab2)));
        ab2->show();
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
    icon->setPixmap(QPixmap("db/Teambuilder/Team/=.png"));
    stats->addWidget(icon, 0, 3);
    stats->addWidget(title3 = new QLabel(tr("Max")), 0, 4);

    title1->setObjectName("Title");
    title2->setObjectName("Title");
    title3->setObjectName("Title");

    QStringList statLabels =
            QStringList() << tr("Hit Points") << tr("Attack") << tr("Defense") << tr("Speed") << tr("Special Attack") << tr("Special Defense");

    /*QFrame *greenBG = new QFrame();
    greenBG->setObjectName("GreenBackground");
    stats->addWidget(greenBG, 1, 2, 6, 3);*/
    for (int i = 0; i < 6; i++) {
        stats->addWidget(new QLabel(statLabels[i]), i+1, 0);
        stats->addWidget(baseStats[i] = new QProgressBar(), i+1, 1);
        stats->addWidget(min[i] = new QLabel(), i+1, 2);
        if (i != Hp) {
            stats->addWidget(buttons[i] = new QImageButtonLR("db/Teambuilder/Team/=.png", ""), i+1, 3);
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

void StatTab::changePoke(int poke) {
    this->poke = poke;

    for (int i = 1; i < 6; i++) {
        if (boost[i] != 0)  {
            buttons[i]->changePics("db/Teambuilder/Team/=.png", "db/Teambuilder/Team/=hover.png");
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

        min[i]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, NatureInfo::NatureOf(i > 1 ? 1 : 2, i), i, 100, 31, 0)));
        max[i]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, NatureInfo::NatureOf(i, i > 1 ? 1 : 2), i, 100, 31, 252)));
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
            typePic->setPixmap(TypeInfo::Picture(i));
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

    min[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, NatureInfo::NatureOf(stat > 1 ? 1 : 2, stat), stat, 100, 31, 0) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));
    max[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, NatureInfo::NatureOf(stat, stat > 1 ? 1 : 2), stat, 100, 31, 252) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));

    if (boost[stat] < 0) {
        return;
    }
    if (boost[stat] == 0) {
        buttons[stat]->changePics("db/Teambuilder/Team/=.png", "db/Teambuilder/Team/=hover.png");
    } else if (boost[stat] == 1) {
        buttons[stat]->changePics("db/Teambuilder/Team/+.png", "db/Teambuilder/Team/+hover.png");
    }
}


void StatTab::decreaseBoost()
{
    int stat = sender()->property("Stat").toInt();

    if (boost[stat] == -6) {
        return;
    }

    boost[stat] -= 1;

    min[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, NatureInfo::NatureOf(stat > 1 ? 1 : 2, stat), stat, 100, 31, 0) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));
    max[stat]->setText(QString("%1").arg(PokemonInfo::FullStat(poke, NatureInfo::NatureOf(stat, stat > 1 ? 1 : 2), stat, 100, 31, 252) * std::max(2, 2+boost[stat]) / std::max(2, 2-boost[stat])));

    if (boost[stat] > 0) {
        return;
    }
    if (boost[stat] == 0) {
        buttons[stat]->changePics("db/Teambuilder/Team/=.png", "db/Teambuilder/Team/=hover.png");
    } else if (boost[stat] == -1) {
        buttons[stat]->changePics("db/Teambuilder/Team/-.png", "db/Teambuilder/Team/-hover.png");
    }
}

/****************************************************/
/*********** Move Tab *******************************/
/****************************************************/

MoveTab::MoveTab()
{
    QVBoxLayout *v = new QVBoxLayout(this);
    moves = new QCompactTable(0, 6);

    v->addWidget(moves);
    moves->setIconSize(QSize(48,19));

    QStringList move_headers;
    move_headers << tr("Type") << tr("Name", "AttackName") << tr("PP") << tr("Pow") << tr("Acc") << tr("Category");
    moves->setHorizontalHeaderLabels(move_headers);
    moves->resizeRowsToContents();
    moves->horizontalHeader()->setStretchLastSection(true);
    moves->horizontalHeader()->setResizeMode(TypeCol, QHeaderView::Fixed);
    moves->horizontalHeader()->resizeSection(TypeCol, 54);
    moves->horizontalHeader()->setResizeMode(PPCol, QHeaderView::Fixed);
    moves->horizontalHeader()->resizeSection(PPCol, 25);
    moves->horizontalHeader()->setResizeMode(PowerCol, QHeaderView::Fixed);
    moves->horizontalHeader()->resizeSection(PowerCol, 32);
    moves->horizontalHeader()->setResizeMode(AccCol, QHeaderView::Fixed);
    moves->horizontalHeader()->resizeSection(AccCol, 32);
    moves->horizontalHeader()->setResizeMode(NameCol, QHeaderView::Fixed);
    moves->horizontalHeader()->resizeSection(NameCol, 100);

    changePoke(1);
}

void MoveTab::changePoke(int poke)
{
    moves->setSortingEnabled(false);

    QSet<int> moveList = PokemonInfo::Moves(poke);

    moves->setRowCount(moveList.count());

    QSet<int>::iterator it = moveList.begin();


    QFont invisible("Verdana", 0);

    for (int i = 0; it != moveList.end(); ++it, ++i)
    {
        int move = *it;

        /* Invisible text used for sorting types */
        int type = MoveInfo::Type(move);
        QTableWidgetItem *w = new QTableWidgetItem(QIcon(TypeInfo::Picture(type)), QString::number(type));
        w->setFont(invisible);
        moves->setItem(i, TypeCol, w);

        moves->setItem(i, NameCol,new QTableWidgetItem(MoveInfo::Name(move)));
        moves->setItem(i, PPCol,new QTableWidgetItem(QString::number(MoveInfo::PP(move))));
        moves->setItem(i, PowerCol,new QTableWidgetItem(MoveInfo::PowerS(move, 4)));
        moves->setItem(i, AccCol,new QTableWidgetItem(MoveInfo::AccS(move)));

        QTableWidgetItem *witem = new QTableWidgetItem(CategoryInfo::Name(MoveInfo::Category(move, 4)));
        witem->setForeground(QColor(CategoryInfo::Color(MoveInfo::Category(move, 4))));
        moves->setItem(i, CategoryCol, witem);
    }

    moves->sortByColumn(NameCol, Qt::AscendingOrder);
    moves->setSortingEnabled(true);
}

/****************************************************/
/*********** QGRIDBOX *******************************/
/****************************************************/

QPixmap *GridBox::background = NULL;

GridBox::GridBox(const QPixmap &pic, bool shiftToBottom)
{
    if (background == NULL)
        background = new QPixmap("db/Teambuilder/PokeDex/ImageGrid.png");

    setPixmap(*background);
    setFixedSize(background->size());

    underLying = new QLabel(this);
    underLying->setPixmap(pic);
    if (shiftToBottom)
        underLying->move(7,7);
    else
        underLying->move(7,3);
}

void GridBox::changePic(const QPixmap &pic)
{
    underLying->setPixmap(pic);
}

/******************************************************/
/*********** POKEBALL TEXT ****************************/
/*****************************************************/

PokeBallText::PokeBallText(const QString &filename, const QString &text)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    QLabel *l = new QLabel();
    l->setPixmap(QPixmap(filename));
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
    typeL->setPixmap(TypeInfo::Picture(type));
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
        t1->setIcon(QIcon(TypeInfo::Picture(i)));
        t2->setIcon(QIcon(TypeInfo::Picture(i)));

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
                l->setText("x 0");
                l->setForeground(Qt::darkGray);
            } else if (eff == 1) {
                l->setText("/ 2");
                l->setForeground(Qt::gray);;
            } else if (eff == 4) {
                l->setText("* 2");
                l->setForeground(Qt::red);
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
    for (int i =0; i < AbilityInfo::NumberOfAbilities(); i++) {
        abilityCb->addItem(AbilityInfo::Name(i));
    }
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
    ability = abilityCb->currentIndex();
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
            if (ability != 0 && ab.ab1 != ability && ab.ab2 != ability)
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
