#include "../PokemonInfo/pokemoninfo.h"
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
    firstCol->addWidget(new QPushButton(tr("&Type Chart")));
    firstCol->addWidget(new QPushButton(tr("&Damage Calculator")));
    firstCol->addStretch(100);


    QVBoxLayout *secondCol = new QVBoxLayout();
    l->addLayout(secondCol);
    secondCol->setSpacing(0);

    BigOpenPokeBall *bop = new BigOpenPokeBall();
    secondCol->addWidget(bop, 0, Qt::AlignRight | Qt::AlignBottom);
    PokedexBody *body = new PokedexBody();
    secondCol->addWidget(body, 100, Qt::AlignRight | Qt::AlignTop);

    connect(body, SIGNAL(pokeChanged(int)), bop, SLOT(changeToPokemon(int)));
}

/****************************************************/
/*********** BIG OPEN POKEBALL **********************/
/****************************************************/
BigOpenPokeBall::BigOpenPokeBall()
{
    setPixmap(QPixmap("db/Teambuilder/PokeDex/OMGHUGE.png"));

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

    ml->addWidget(new QPushButton(QIcon("db/Teambuilder/PokeDex/EvoIcon.png"), tr("&Evolution")), 2,3);
    ml->addWidget(new QPushButton(QIcon("db/Teambuilder/PokeDex/FormeIcon.png"), tr("&Other Formes")), 3,3);

    ml->addWidget(back = new GridBox(QPixmap(),true),0,2,2,2);

    QImageButton *toggleUp =
            new QImageButton("db/Teambuilder/PokeDex/Toggle buttons/UpArrowNormal.png", "db/Teambuilder/PokeDex/Toggle Buttons/UpArrowGlow.png");
    QImageButton *toggleDown =
            new QImageButton("db/Teambuilder/PokeDex/Toggle buttons/DownArrowNormal.png", "db/Teambuilder/PokeDex/Toggle Buttons/DownArrowGlow.png");

    toggleUp->setParent(this);
    toggleUp->move(433, 17);
    toggleDown->setParent(this);
    toggleDown->move(439, 23);

    shinyBox = new QCheckBox(tr("Shiny"),this);
    shinyBox->move(435,65);

    changeToPokemon(1);

    connect(shinyBox, SIGNAL(toggled(bool)), SLOT(updatePicture()));
}

void BigOpenPokeBall::changeToPokemon(int poke)
{
    currentPoke = poke;

    update();
}

void BigOpenPokeBall::update()
{
    int n = currentPoke;
    num->setText(QString("%1").arg(n));
    name->setText(PokemonInfo::Name(n));
    specy->setText("Pokemon Specy");
    height->setText("Height");
    weight->setText(tr("<b>Wt:</b> %1 lbs.").arg(PokemonInfo::WeightS(n)));
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

void BigOpenPokeBall::updatePicture()
{
    front->changePic(PokemonInfo::Picture(currentPoke, 0, PokemonInfo::BaseGender(currentPoke),shiny(),false));
    back->changePic(PokemonInfo::Picture(currentPoke, 0, PokemonInfo::BaseGender(currentPoke),shiny(),true));
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
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setMargin(0);
    QVBoxLayout *col1 = new QVBoxLayout();
    col1->setSpacing(2);
    col1->setMargin(5);
    hl->addLayout(col1);
    col1->addWidget(new QPushButton(QIcon("db/Teambuilder/PokeDex/advsrchicon.png"), tr("&Advanced Search")));
    col1->addWidget(pokeEdit = new QLineEdit());
    pokeList = new TB_PokeChoice(false);
    pokeList->verticalHeader()->setDefaultSectionSize(30);
    QCompleter *comp = new QCompleter(pokeEdit);
    comp->setModel(pokeList->model());
    comp->setCompletionColumn(1);
    pokeEdit->setCompleter(comp);
    col1->addWidget(pokeList);
    col1->addWidget(new PokeBallText("db/Teambuilder/PokeDex/Orangeball.png", tr("Sort Pokemon List")));

    connect(comp, SIGNAL(activated(QString)), this, SLOT(changeToPokemon(QString)));
    connect(pokeEdit, SIGNAL(returnPressed()), SLOT(changePokemon()));
    connect(pokeList, SIGNAL(cellActivated(int,int)), SLOT(changePokemonFromRow(int)));

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
    hl->addWidget(tabw, 100);

    ProfileTab *pt;
    tabw->addTab(pt = new ProfileTab(), tr("PROFILE"));
    tabw->addTab(new QFrame(), tr("STATS"));
    tabw->addTab(new QFrame(), tr("MOVES"));

    connect(this, SIGNAL(pokeChanged(int)), pt, SLOT(changeDesc(int)));

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

/****************************************************/
/*********** PROFILE TAB ****************************/
/****************************************************/

ProfileTab::ProfileTab()
{
    QHBoxLayout *l= new QHBoxLayout(this);

    l->setSpacing(0);
    l->setMargin(2);

    QVBoxLayout *firstCol = new QVBoxLayout();
    l->addLayout(firstCol, 100);

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

    QLabel *oak = new QLabel();
    oak->setPixmap(QPixmap("db/Teambuilder/PokeDex/Prof.Oak.png"));
    l->addWidget(oak);

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

    int ability1 = PokemonInfo::Abilities(poke)[0];
    ab1->setText(QString("<b>%1</b> - %2").arg(AbilityInfo::Name(ability1), AbilityInfo::Desc(ability1)));
    int ability2 = PokemonInfo::Abilities(poke)[1];
    if (ability2 == 0) {
        ab2->hide();
    } else {
        ab2->setText(QString("<b>%1</b> - %2").arg(AbilityInfo::Name(ability2), AbilityInfo::Desc(ability2)));
        ab2->show();
    }
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
