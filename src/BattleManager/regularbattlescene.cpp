#include "regularbattlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "pokemoninfoaccessor.h"
#include "proxydatacontainer.h"

#include <QLayout>
#include "../Utilities/otherwidgets.h"

RegularBattleScene::RegularBattleScene(battledata_ptr dat, BattleDefaultTheme *theme) : mData(dat), peeking(false),
    pauseCount(0)
{
    gui.theme = theme;

    setupGui();
}

QHBoxLayout* RegularBattleScene::createTeamLayout(QLabel **labels)
{
    QHBoxLayout *foeteam = new QHBoxLayout();
    foeteam->addStretch(100);
    for (int i = 0; i < 6; i++) {
        labels[i] = new QLabel();
        labels[i]->setPixmap(gui.theme->StatusIcon(Pokemon::Fine));
        foeteam->addWidget(labels[i],0,Qt::AlignTop);
    }
    foeteam->setSpacing(1);

    return foeteam;
}

QGridLayout* RegularBattleScene::createHPBarLayout(int slot)
{
    QGridLayout *inside = new QGridLayout();
    inside->setMargin(0);
    inside->setSpacing(4);

    gui.nick[slot] = new QLabel();
    gui.nick[slot]->setObjectName("PokemonNick");
    inside->addWidget(gui.nick[slot], 0, 0, 1, 1, Qt::AlignLeft);
    inside->setColumnStretch(0, 100);

    gui.status[slot] = new QLabel();
    inside->addWidget(gui.status[slot], 0, 1);

    gui.gender[slot] = new QLabel();
    inside->addWidget(gui.gender[slot], 0, 2);

    gui.level[slot] = new QLabel();
    gui.level[slot]->setObjectName("PokemonLevel");
    inside->addWidget(gui.level[slot], 0, 3);


    QHBoxLayout *barL = new QHBoxLayout();
    barL->setMargin(0);
    barL->setSpacing(0);
    QLabel *HPIcon = new QLabel();
    HPIcon->setPixmap(gui.theme->Sprite("hpsquare"));
    HPIcon->setFixedSize(HPIcon->pixmap()->size());
    barL->addWidget(HPIcon);
    gui.bars[slot] = new QClickPBar();
    gui.bars[slot]->setObjectName("LifePoints"); /* for stylesheets */
    gui.bars[slot]->setRange(0, 100);
    barL->addWidget(gui.bars[slot]);

    inside->addLayout(barL,1,0,1,4);

    return inside;
}

QWidget *RegularBattleScene::createFullBarLayout(int nslots, int player)
{
    QLabel* oppPoke = new QLabel();
    oppPoke->setPixmap(gui.theme->Sprite("hpbar"));
    oppPoke->setFixedSize(oppPoke->pixmap()->size());

    QVBoxLayout *oppl = new QVBoxLayout(oppPoke);
    oppl->setMargin(5);
    oppl->setSpacing(0);
    oppl->addSpacing(3);

    QHBoxLayout *oppl2 = new QHBoxLayout();
    oppl->addLayout(oppl2);
    oppl2->setMargin(0);
    oppl2->setSpacing(6);

    for (int i = 0; i < nslots/2; i++) {
        oppl2->addLayout(createHPBarLayout(data()->spot(player, i)));
    }

    return oppPoke;
}

void RegularBattleScene::setupGui()
{
    int nslots = data()->numberOfSlots();

    gui.nick.resize(nslots);
    gui.level.resize(nslots);
    gui.gender.resize(nslots);
    gui.bars.resize(nslots);
    gui.status.resize(nslots);

    QVBoxLayout *l=  new QVBoxLayout(this);
    l->setMargin(0);

    /* As anyway the BaseGraphicsZone is a fixed size, it's useless to
       resize that part, might as well let  the chat be resized */
    l->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *firstLine = new QHBoxLayout();
    l->addLayout(firstLine);

    QHBoxLayout *midzone = new QHBoxLayout();
    l->addLayout(midzone);

    QHBoxLayout *lastLine = new QHBoxLayout();
    l->addLayout(lastLine);

    QVBoxLayout *teamAndName[2];

    for (int i = 0; i < 2; i++) {
        teamAndName[i] = new QVBoxLayout();
        teamAndName[i]->addLayout(createTeamLayout(gui.pokeballs[i]));
        teamAndName[i]->addWidget(gui.trainers[i] = new QLabel(data()->name(i)),0, Qt::AlignRight);
        gui.trainers[i]->setObjectName("TrainerNick");
    }

    firstLine->addWidget(createFullBarLayout(nslots, opponent()));
    firstLine->addLayout(teamAndName[opponent()]);

    gui.zone = new QWidget();

    /* Make the code below more generic? */
    QVBoxLayout *midme = new QVBoxLayout();
    midzone->addLayout(midme);
    midme->addStretch(100);
    gui.timers[myself()] = new QProgressBar();
    gui.timers[myself()]->setObjectName("TimeOut"); //for style sheets
    gui.timers[myself()]->setRange(0,300);
    QLabel *mybox = new QLabel();
    mybox->setObjectName("MyTrainerBox");
    mybox->setFixedSize(82,82);
    mybox->setPixmap(gui.theme->TrainerSprite(data()->team(myself()).avatar()));
    midme->addWidget(gui.timers[myself()]);
    midme->addWidget(mybox);

    midzone->addWidget(gui.zone);

    QVBoxLayout *midopp = new QVBoxLayout();
    midzone->addLayout(midopp);
    midopp->addStretch(100);
    gui.timers[opponent()] = new QProgressBar();
    gui.timers[opponent()]->setObjectName("TimeOut"); //for style sheets
    gui.timers[opponent()]->setRange(0,300);
    QLabel *oppbox = new QLabel();
    oppbox->setPixmap(gui.theme->TrainerSprite(data()->team(opponent()).avatar()));
    oppbox->setObjectName("OppTrainerBox");
    oppbox->setFixedSize(82,82);
    midopp->addWidget(oppbox);
    midopp->addWidget(gui.timers[opponent()]);

    lastLine->addLayout(teamAndName[myself()]);
    lastLine->addWidget(createFullBarLayout(nslots, myself()));
}

RegularBattleScene::~RegularBattleScene()
{
}

bool RegularBattleScene::reversed() const
{
    return data()->role(1) == BattleConfiguration::Player;
}

int RegularBattleScene::opponent() const
{
    return reversed() ? 0 : 1;
}

int RegularBattleScene::myself() const
{
    return reversed() ? 1 : 0;
}

RegularBattleScene::battledata_ptr RegularBattleScene::data()
{
    return mData;
}

const RegularBattleScene::battledata_ptr RegularBattleScene::data() const
{
    return mData;
}

ProxyDataContainer * RegularBattleScene::getDataProxy()
{
    return data()->exposedData();
}

void RegularBattleScene::pause()
{
    pauseCount =+ 1;
    baseClass::pause();
}

void RegularBattleScene::unpause()
{
    pauseCount -= 1;

    if (pauseCount == 0) {
        if (commands.size() > 0) {
            commands[0]->apply();
            delete commands[0];
            commands.erase(commands.begin(), commands.begin()+1);
        }
    }

    baseClass::unpause();
}

void RegularBattleScene::onUseAttack(int spot, int attack) {
    emit attackUsed(spot, attack);
}
