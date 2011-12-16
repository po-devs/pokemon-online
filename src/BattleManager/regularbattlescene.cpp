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

///Storage to use to construct battlescene

//class BaseGraphicsZone;

//class BaseBattleDisplay : public QWidget
//{
//    Q_OBJECT
//public:
//    BaseBattleInfo* myInfo;
//    BaseBattleInfo &info() const {
//        return *myInfo;
//    }

//    BaseBattleDisplay(BaseBattleInfo &i);

//    virtual void updatePoke(int spot);
//    virtual void updatePoke(int player, int index);
//    virtual void updateHp(int spot);
//    virtual void updateToolTip(int spot);
//    void changeStatus(int spot, int poke, int status);
//public slots:
//    void updateTimers();

//protected:
//    QString health(int lifePercent);

//    BaseGraphicsZone *zone;

//    QVector<QLabel *> nick;
//    QVector<QLabel *> level;
//    QVector<QLabel *> status;
//    QVector<QLabel *> gender;
//    QVector<QClickPBar *> bars;

//    QProgressBar *timers[2];
//    QLabel * trainers[2];

//    /* The pokeballs to indicate how well a team is doing */
//    QLabel *advpokeballs[6];
//    QLabel *mypokeballs[6];

//    BaseBattleWindow *parent;
//};

///* The graphics zone, where both pokes are displayed */
//class BaseGraphicsZone : public QGraphicsView
//{
//    Q_OBJECT
//public:
//    BaseGraphicsZone(BaseBattleInfo *info);
//    /* displays that poke */
//    template <class T>
//    void switchTo(const T &poke, int spot, bool sub, Pokemon::uniqueId specialSprite = Pokemon::NoPoke);
//    /* Display blank */
//    void switchToNaught(int spot);
//    /* For tool tips */
//    void mouseMoveEvent(QMouseEvent *e);
//    /* Updates the position of an item */
//    void updatePos(int spot);

//    /* Loads a pixmap if not loaded otherwise go see graphics */
//    QPixmap loadPixmap(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub);
//    /* We are using a qmap to store the graphics already loaded. So the key of the pixmap
//        is a combination of 2 bools, 1 quin8; and one quint16 */
//    quint64 key(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub) const;
//    QHash<qint32, QPixmap> graphics;
//    /* Current pixmaps displayed */
//    QVector<QGraphicsPixmapItem *> items;
//    QGraphicsScene scene;

//    QVector<QString> tooltips;
//    BaseBattleInfo *mInfo;

//    BaseBattleInfo & info() {
//        return *mInfo;
//    }
//};

///* Yeepee, at last templates */
//template <class T>
//void BaseGraphicsZone::switchTo(const T &poke, int spot, bool sub, Pokemon::uniqueId specialSprite)
//{
//    items[spot]->setPixmap(loadPixmap(specialSprite != Pokemon::NoPoke ? specialSprite:poke.num(), poke.shiny(),
//                                      info().player(spot) == info().myself , poke.gender(), sub));
//    updatePos(spot);
//}


//BaseBattleDisplay::BaseBattleDisplay(BaseBattleInfo &i)
//    : myInfo(&i)
//{
//    parent = NULL;

//    QVBoxLayout *l=  new QVBoxLayout(this);
//    l->setMargin(0);

//    /* As anyway the BaseGraphicsZone is a fixed size, it's useless to
//       resize that part, might as well let  the chat be resized */
//    l->setSizeConstraint(QLayout::SetFixedSize);

//    nick.resize(i.numberOfSlots);
//    level.resize(i.numberOfSlots);
//    gender.resize(i.numberOfSlots);
//    bars.resize(i.numberOfSlots);
//    status.resize(i.numberOfSlots);

//    QHBoxLayout *firstLine = new QHBoxLayout();
//    l->addLayout(firstLine);

//    QLabel* oppPoke = new QLabel();
//    oppPoke->setPixmap(Theme::Sprite("hpbar"));
//    oppPoke->setFixedSize(oppPoke->pixmap()->size());
//    firstLine->addWidget(oppPoke);

//    QVBoxLayout *oppl = new QVBoxLayout(oppPoke);
//    oppl->setMargin(5);
//    oppl->setSpacing(0);
//    oppl->addSpacing(3);

//    QHBoxLayout *oppl2 = new QHBoxLayout();
//    oppl->addLayout(oppl2);
//    oppl2->setMargin(0);
//    oppl2->setSpacing(6);

//    for (int i = 0; i < info().numberOfSlots/2; i++) {
//        QGridLayout *inside = new QGridLayout();
//        oppl2->addLayout(inside);
//        inside->setMargin(0);
//        inside->setSpacing(4);

//        int slot = info().slot(info().opponent, i);

//        nick[slot] = new QLabel();
//        nick[slot]->setObjectName("PokemonNick");
//        inside->addWidget(nick[slot], 0, 0, 1, 1, Qt::AlignLeft);
//        inside->setColumnStretch(0, 100);

//        status[slot] = new QLabel();
//        inside->addWidget(status[slot], 0, 1);

//        gender[slot] = new QLabel();
//        inside->addWidget(gender[slot], 0, 2);

//        level[slot] = new QLabel();
//        level[slot]->setObjectName("PokemonLevel");
//        inside->addWidget(level[slot], 0, 3);


//        QHBoxLayout *barL = new QHBoxLayout();
//        barL->setMargin(0);
//        barL->setSpacing(0);
//        QLabel *HPIcon = new QLabel();
//        HPIcon->setPixmap(Theme::Sprite("hpsquare"));
//        HPIcon->setFixedSize(HPIcon->pixmap()->size());
//        barL->addWidget(HPIcon);
//        bars[slot] = new QClickPBar();
//        bars[slot]->setObjectName("LifePoints"); /* for stylesheets */
//        bars[slot]->setRange(0, 100);
//        barL->addWidget(bars[slot]);

//        inside->addLayout(barL,1,0,1,4);
//    }

//    QHBoxLayout *foeteam = new QHBoxLayout();
//    foeteam->addStretch(100);
//    for (int i = 0; i < 6; i++) {
//        advpokeballs[i] = new QLabel();
//        advpokeballs[i]->setPixmap(Theme::StatusIcon(Pokemon::Fine));
//        foeteam->addWidget(advpokeballs[i],0,Qt::AlignTop);
//    }
//    foeteam->setSpacing(1);

//    QVBoxLayout * oppTeamAndName = new QVBoxLayout();
//    oppTeamAndName->addLayout(foeteam);
//    oppTeamAndName->addWidget(trainers[info().opponent] = new QLabel(info().name(info().opponent)),0, Qt::AlignRight);
//    trainers[info().opponent]->setObjectName("TrainerNick");
//    firstLine->addLayout(oppTeamAndName);


//    zone = new BaseGraphicsZone(&info());

//    QHBoxLayout *midzone = new QHBoxLayout();
//    QVBoxLayout *midme = new QVBoxLayout();
//    midzone->addLayout(midme);
//    midme->addStretch(100);
//    timers[info().myself] = new QProgressBar();
//    timers[info().myself]->setObjectName("TimeOut"); //for style sheets
//    timers[info().myself]->setRange(0,300);
//    midme->addWidget(timers[info().myself]);
//    QLabel *mybox = new QLabel();
//    mybox->setObjectName("MyTrainerBox");
//    mybox->setFixedSize(82,82);
//    mybox->setPixmap(Theme::TrainerSprite(info().pInfo[info().myself].avatar));
//    midme->addWidget(mybox);
//    midzone->addWidget(zone);
//    QVBoxLayout *midopp = new QVBoxLayout();
//    midzone->addLayout(midopp);
//    QLabel *oppbox = new QLabel();
//    oppbox->setPixmap(Theme::TrainerSprite(info().pInfo[info().opponent].avatar));
//    oppbox->setObjectName("OppTrainerBox");
//    oppbox->setFixedSize(82,82);
//    midopp->addWidget(oppbox);
//    timers[info().opponent] = new QProgressBar();
//    timers[info().opponent]->setObjectName("TimeOut"); //for style sheets
//    timers[info().opponent]->setRange(0,300);
//    midopp->addWidget(timers[info().opponent]);
//    midopp->addStretch(100);

//    l->addLayout(midzone);


//    QHBoxLayout *lastLine = new QHBoxLayout();
//    l->addLayout(lastLine);

//    QHBoxLayout *team = new QHBoxLayout();
//    for (int i = 0; i < 6; i++) {
//        mypokeballs[i] = new QLabel();
//        mypokeballs[i]->setPixmap(Theme::StatusIcon(Pokemon::Fine));
//        team->addWidget(mypokeballs[i],0,Qt::AlignBottom);
//    }
//    team->setSpacing(1);
//    team->addStretch(100);

//    QVBoxLayout * myTeamAndName = new QVBoxLayout();
//    myTeamAndName->addWidget(trainers[info().myself] = new QLabel(info().name(info().myself)),0, Qt::AlignLeft);
//    myTeamAndName->addLayout(team);
//    trainers[info().myself]->setObjectName("TrainerNick");

//    lastLine->addLayout(myTeamAndName);

//    QLabel* myPoke = new QLabel();
//    myPoke->setPixmap(Theme::Sprite("hpbar"));
//    myPoke->setFixedSize(myPoke->pixmap()->size());
//    lastLine->addWidget(myPoke);

//    QVBoxLayout *myl = new QVBoxLayout(myPoke);
//    myl->setMargin(5);
//    myl->setSpacing(0);
//    myl->addSpacing(3);

//    QHBoxLayout *myl2 = new QHBoxLayout();
//    myl->addLayout(myl2);
//    myl2->setMargin(0);
//    myl2->setSpacing(6);

//    for (int i = 0; i < info().numberOfSlots/2; i++) {

//        QGridLayout *inside = new QGridLayout();
//        myl2->addLayout(inside);
//        inside->setMargin(0);
//        inside->setSpacing(4);

//        int slot = info().slot(info().myself, i);

//        nick[slot] = new QLabel();
//        nick[slot]->setObjectName("PokemonNick");
//        inside->addWidget(nick[slot], 0, 0, 1, 1, Qt::AlignLeft);
//        inside->setColumnStretch(0, 100);

//        status[slot] = new QLabel();
//        inside->addWidget(status[slot], 0, 1);

//        gender[slot] = new QLabel();
//        inside->addWidget(gender[slot], 0, 2);

//        level[slot] = new QLabel();
//        level[slot]->setObjectName("PokemonLevel");
//        inside->addWidget(level[slot], 0, 3);


//        QHBoxLayout *barL = new QHBoxLayout();
//        barL->setMargin(0);
//        barL->setSpacing(0);
//        QLabel *HPIcon = new QLabel();
//        HPIcon->setPixmap(Theme::Sprite("hpsquare"));
//        HPIcon->setFixedSize(HPIcon->pixmap()->size());
//        barL->addWidget(HPIcon);
//        bars[slot] = new QClickPBar();
//        bars[slot]->setObjectName("LifePoints"); /* for stylesheets */
//        bars[slot]->setRange(0, 100);
//        barL->addWidget(bars[slot]);

//        inside->addLayout(barL,1,0,1,4);
//    }

//    for (int i = 0; i < info().numberOfSlots; i++) {
//        updatePoke(i);
//    }

//    updateTimers();

//    QTimer *t = new QTimer (this);
//    t->start(200);
//    connect(t, SIGNAL(timeout()), SLOT(updateTimers()));
//}

//void BaseBattleDisplay::updateTimers()
//{
//    for (int i = 0; i <= 1; i++) {
//        int ctime = std::max(long(0), info().ticking[i] ? info().time[i] + info().startingTime[i] - time(NULL) : info().time[i]);
//        if (ctime <= 5*60) {
//            timers[i]->setValue(ctime);
//        } else {
//            timers[i]->setValue(300);
//        }
//        timers[i]->setFormat(QString("%1 : %2").arg(ctime/60).arg(QString::number(ctime%60).rightJustified(2,'0')));
//        if (ctime > 60) {
//            timers[i]->setStyleSheet("::chunk{background-color: #55a8fc;}");
//        }else if (ctime > 30) {
//            timers[i]->setStyleSheet("::chunk{background-color: #F8DB17;;}");
//        } else {
//            timers[i]->setStyleSheet("::chunk{background-color: #D40202;}");
//        }
//    }
//}

//void BaseBattleDisplay::updatePoke(int spot)
//{
//    if (!parent) {
//        parent = dynamic_cast<BaseBattleWindow*>(QWidget::parent());

//        if (!parent)
//            return;
//    }

//    if (info().pokeAlive[spot]) {
//        const ShallowBattlePoke &poke = info().currentShallow(spot);
//        zone->switchTo(poke, spot, info().sub[spot], info().specialSprite[spot]);
//        nick[spot]->setText(parent->rnick(spot));
//        level[spot]->setText(tr("Lv. %1").arg(poke.level()));
//        updateHp(spot);
//        bars[spot]->setStyleSheet(health(poke.lifePercent()));
//        gender[spot]->setPixmap(Theme::GenderPicture(poke.gender(), Theme::BattleM));
//        int status = poke.status();
//        this->status[spot]->setPixmap(Theme::BattleStatusIcon(status));

//        if (info().player(spot) == info().myself) {
//            mypokeballs[info().slotNum(spot)]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
//        } else {
//            advpokeballs[info().slotNum(spot)]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
//        }
//    }  else {
//        zone->switchToNaught(spot);
//        nick[spot]->setText("");
//        this->status[spot]->setPixmap(Theme::BattleStatusIcon(Pokemon::Fine));
//        gender[spot]->setPixmap(QPixmap());
//        bars[spot]->setValue(0);
//    }
//}

//void BaseBattleDisplay::updatePoke(int player, int index)
//{
//    ShallowBattlePoke &poke = info().pokemons[player][index];

//    if (player == info().myself) {
//        mypokeballs[index]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
//    } else {
//        advpokeballs[index]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
//    }

//    changeStatus(player, index, poke.status());
//}

//void BaseBattleDisplay::updateHp(int spot)
//{
//    bars[spot]->setValue(info().currentShallow(spot).lifePercent());
//}

//void BaseBattleDisplay::updateToolTip(int spot)
//{
//    if (!parent) {
//        parent = dynamic_cast<BaseBattleWindow*>(QWidget::parent());

//        if (!parent)
//            return;
//    }
//    QString tooltip;

//    QString stats[7] = {
//        tu(StatInfo::Stat(1)),
//        tu(StatInfo::Stat(2)),
//        tu(StatInfo::Stat(3)),
//        tu(StatInfo::Stat(4)),
//        tu(StatInfo::Stat(5)),
//        tu(StatInfo::Stat(6)),
//        tu(StatInfo::Stat(7))
//    };
//    int max = 0;
//    for (int i = 0; i < 7; i++) {
//        max = std::max(max, stats[i].length());
//    }
//    for (int i = 0; i < 7; i++) {
//        stats[i] = stats[i].leftJustified(max, '.', false);
//    }

//    const ShallowBattlePoke &poke = info().currentShallow(spot);

//    tooltip += parent->rnick(spot) + "\n";
//    tooltip += TypeInfo::Name(PokemonInfo::Type1(poke.num(), info().gen));
//    int type2 = PokemonInfo::Type2(poke.num());
//    if (type2 != Pokemon::Curse) {
//        tooltip += " " + TypeInfo::Name(PokemonInfo::Type2(poke.num(), info().gen));
//    }
//    tooltip += "\n";

//    for (int i = 0; i < 5; i++) {
//        // Gen 1 only has Special, and we treat SAtk as Special hiding SDef.
//        if (info().gen == 1) {
//            switch (i) {
//            case 2: tooltip += QString("\n%1 ").arg(tr("Special")); break;
//            case 3: continue;
//            default: tooltip += "\n" + stats[i] + " ";
//            }
//        } else {
//            tooltip += "\n" + stats[i] + " ";
//        }
//        int boost = info().statChanges[spot].boosts[i];
//        if (boost >= 0) {
//            tooltip += QString("+%1").arg(boost);
//        } else if (boost < 0) {
//            tooltip += QString("%1").arg(boost);
//        }
//    }
//    for (int i = 5; i < 7; i++) {
//        int boost = info().statChanges[spot].boosts[i];
//        if (boost) {
//            tooltip += "\n" + stats[i] + " ";

//            if (boost > 0) {
//                tooltip += QString("+%1").arg(boost);
//            } else if (boost < 0) {
//                tooltip += QString("%1").arg(boost);
//            }
//        }
//    }

//    tooltip += "\n";

//    int flags = info().statChanges[spot].flags;

//    int spikes[3] = {BattleDynamicInfo::Spikes, BattleDynamicInfo::SpikesLV2 ,BattleDynamicInfo::SpikesLV3};
//    for (int i = 0; i < 3; i++) {
//        if (flags & spikes[i]) {
//            tooltip += "\n" + tr("Spikes level %1").arg(i+1);
//            break;
//        }
//    }

//    int tspikes[2] = {BattleDynamicInfo::ToxicSpikes, BattleDynamicInfo::ToxicSpikesLV2};
//    for (int i = 0; i < 2; i++) {
//        if (flags & tspikes[i]) {
//            tooltip += "\n" + tr("Toxic Spikes level %1").arg(i+1);
//            break;
//        }
//    }

//    if (flags & BattleDynamicInfo::StealthRock) {
//        tooltip += "\n" + tr("Stealth Rock");
//    }

//    zone->tooltips[spot] = tooltip;
//}

//void BaseBattleDisplay::changeStatus(int spot, int poke, int status) {
//    if (info().player(spot)==info().myself) {
//        mypokeballs[poke]->setPixmap(Theme::StatusIcon(status));
//    } else {
//        advpokeballs[poke]->setPixmap(Theme::StatusIcon(status));
//    }
//}

//QString BaseBattleDisplay::health(int lifePercent)
//{
//    return lifePercent > 50 ? "::chunk{background-color: #1fc42a;}" : (lifePercent >= 26 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
//}

//BaseGraphicsZone::BaseGraphicsZone(BaseBattleInfo *i) : mInfo(i)
//{
//    setScene(&scene);
//    setMouseTracking(true);

//    tooltips.resize(info().numberOfSlots);
//    items.resize(info().numberOfSlots);

//    scene.setSceneRect(0,0,257,145);
//    scene.addItem(new QGraphicsPixmapItem(Theme::Pic(QString("battle_fields/%1.png").arg((rand()%11)+1))));

//    for (int i = 0; i < info().numberOfSlots; i++) {
//        items[i] = new QGraphicsPixmapItem();
//        scene.addItem(items[i]);
//    }

//    int size = Version::avatarSize[info().gen-1];

//    if (!info().multiples()) {
//        items[info().slot(info().myself)]->setPos(50 - size/2, 146 - size);
//        items[info().slot(info().opponent)]->setPos(184- size/2, 96 - size);
//    } else {
//        for (int i = 0; i < info().numberOfSlots/2; i++) {
//            items[info().slot(info().myself, i)]->setPos(i*60, 146-size);
//            int base = 257-80-(info().numberOfSlots/2 - 1)*60;
//            items[info().slot(info().opponent, i)]->setPos(base+i*60, 96 - size);
//        }
//    }
//}

//void BaseGraphicsZone::updatePos(int spot)
//{
//    int player = info().player(spot);

//    int width = items[spot]->pixmap().width();
//    int height = items[spot]->pixmap().height();

//    if (player == info().myself) {
//        if (!info().multiples()) {
//            items[spot]->setPos(50 - width/2, 146 - height);
//        } else {
//            items[spot]->setPos(info().slotNum(spot)*60, 146-height);
//        }
//    } else {
//        if (!info().multiples()) {
//            items[spot]->setPos(184 - width/2, 96 - height);
//        } else {
//            int base = 257-80-(info().numberOfSlots/2 - 1)*60;
//            items[spot]->setPos(base + info().slotNum(spot)*60, 96-height);
//        }
//    }
//}

//void BaseGraphicsZone::switchToNaught(int spot)
//{
//    items[spot]->setPixmap(QPixmap());
//}

//QPixmap BaseGraphicsZone::loadPixmap(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub)
//{
//    quint64 key = this->key(num, shiny, back, gender, sub);

//    if (!graphics.contains(key)) {
//        QPixmap p;
//        if (sub) {
//            p = PokemonInfo::Sub(info().gen, back);
//        } else {
//            p = PokemonInfo::Picture(num, info().gen, gender, shiny, back);
//        }

//        QImage img = p.toImage();
//        cropImage(img);
//        p = QPixmap::fromImage(img);

//        graphics.insert(key, p);
//    }

//    return graphics[key];
//}

//quint64 BaseGraphicsZone::key(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub) const
//{
//    return sub ? ((1 << 27) + (back << 28)) : (num.pokenum + (num.subnum << 16) + (gender << 24) + (back << 26) + (shiny<<27));
//}

//void BaseGraphicsZone::mouseMoveEvent(QMouseEvent * e)
//{
//    QGraphicsItem *it = this->itemAt(e->pos());

//    for (int i = 0; i < items.size(); i++) {
//        if (items[i] == it) {
//            QToolTip::setFont(QFont("Courier New",8));
//            QToolTip::showText(e->globalPos(), tooltips[i]);
//            break;
//        }
//    }
//}


//class BattleDisplay : public BaseBattleDisplay
//{
//    Q_OBJECT
//public:
//    BattleDisplay(BattleInfo &i);

//    void updateHp(int spot);
//    void updateToolTip(int spot);

//    BattleInfo &info() const {
//        return *(BattleInfo *)(&BaseBattleDisplay::info());
//    }
//public slots:
//    void changeBarMode();

//protected:
//    const PokeBattle &mypoke(int spot) const {return info().currentPoke(spot); }
//    const ShallowBattlePoke &foe(int spot) const {return info().currentShallow(spot); }

//    QList<bool> percentageMode;
//};


//BattleDisplay::BattleDisplay(BattleInfo &i)
//    : BaseBattleDisplay(i)
//{
//    for (int i = 0; i < info().numberOfSlots; i++) {
//        if (info().player(i) == info().myself) {
//            percentageMode.push_back(false);
//            bars[i]->setRange(0,100);
//            bars[i]->setFormat("%v / %m");
//            connect(bars[i], SIGNAL(clicked()), SLOT(changeBarMode()));
//        } else {
//            percentageMode.push_back(true);
//        }
//    }


//    for (int i = 0; i < 6; i++) {
//        mypokeballs[i]->setToolTip(info().myteam.poke(i).nick());
//    }

//    for (int i = 0; i < info().numberOfSlots/2; i++) {
//        updatePoke(info().slot(info().myself, i));
//    }
//}

//void BattleDisplay::updateHp(int spot)
//{
//    if (percentageMode[spot])
//        BaseBattleDisplay::updateHp(spot);
//    else {
//        bars[spot]->setRange(0, mypoke(spot).totalLifePoints());
//        bars[spot]->setValue(mypoke(spot).lifePoints());
//    }
//}

//void BattleDisplay::changeBarMode()
//{
//    int i;
//    for (i = 0; i < info().numberOfSlots; i++) {
//        if (bars[i] == sender()) {
//            break;
//        }
//    }

//    bars[i]->setFormat(percentageMode[i] ? "%v / %m" : "%p%");
//    percentageMode[i] = !percentageMode[i];

//    if (percentageMode[i])
//        bars[i]->setRange(0,100);

//    updateHp(i);
//}

//void BattleDisplay::updateToolTip(int spot)
//{
//    if (info().player(spot) == info().opponent) {
//        BaseBattleDisplay::updateToolTip(spot);
//        return;
//    }

//    QString tooltip;

//    QString stats[7] = {
//        tu(StatInfo::Stat(1)),
//        tu(StatInfo::Stat(2)),
//        tu(StatInfo::Stat(3)),
//        tu(StatInfo::Stat(4)),
//        tu(StatInfo::Stat(5)),
//        tu(StatInfo::Stat(6)),
//        tu(StatInfo::Stat(7))
//    };
//    int max = 0;
//    for (int i = 0; i < 7; i++) {
//        max = std::max(max, stats[i].length());
//    }
//    for (int i = 0; i < 7; i++) {
//        stats[i] = stats[i].leftJustified(max, '.', false);
//    }

//    tooltip += info().currentPoke(spot).nick() + "\n";
//    Pokemon::uniqueId num = info().currentPoke(spot).num();
//    tooltip += TypeInfo::Name(PokemonInfo::Type1(num, info().gen));
//    int type2 = PokemonInfo::Type2(num);
//    if (type2 != Pokemon::Curse) {
//        tooltip += " " + TypeInfo::Name(PokemonInfo::Type2(num, info().gen));
//    }
//    tooltip += "\n";

//    for (int i = 0; i < 5; i++) {
//        // Gen 1 only has Special, and we treat SAtk as Special hiding SDef.
//        if (info().gen == 1) {
//            switch (i) {
//            case 2: tooltip += QString("\n%1 ").arg(tr("Special")); break;
//            case 3: continue;
//            default: tooltip += "\n" + stats[i] + " ";
//            }
//        } else {
//            tooltip += "\n" + stats[i] + " ";
//        }
//        int stat = info().mystats[info().number(spot)].stats[i];
//        if (stat == -1) {
//            tooltip += "???";
//        } else {
//            tooltip += QString::number(stat);
//        }
//        int boost = info().statChanges[spot].boosts[i];
//        if (boost > 0) {
//            tooltip += QString("(+%1)").arg(boost);
//        } else if (boost < 0) {
//            tooltip += QString("(%1)").arg(boost);
//        }
//    }
//    for (int i = 5; i < 7; i++) {
//        int boost = info().statChanges[spot].boosts[i];

//        if (boost != 0) {
//            tooltip += "\n" + stats[i] + " ";

//            if (boost > 0) {
//                tooltip += QString("+%1").arg(boost);
//            } else {
//                tooltip += QString("%1").arg(boost);
//            }
//        }
//    }

//    tooltip += "\n";

//    int flags = info().statChanges[spot].flags;

//    int spikes[3] = {BattleDynamicInfo::Spikes, BattleDynamicInfo::SpikesLV2 ,BattleDynamicInfo::SpikesLV3};
//    for (int i = 0; i < 3; i++) {
//        if (flags & spikes[i]) {
//            tooltip += "\n" + tr("Spikes level %1").arg(i+1);
//            break;
//        }
//    }

//    int tspikes[2] = {BattleDynamicInfo::ToxicSpikes, BattleDynamicInfo::ToxicSpikesLV2};
//    for (int i = 0; i < 2; i++) {
//        if (flags & tspikes[i]) {
//            tooltip += "\n" + tr("Toxic Spikes level %1").arg(i+1);
//            break;
//        }
//    }

//    if (flags & BattleDynamicInfo::StealthRock) {
//        tooltip += "\n" + tr("Stealth Rock");
//    }

//    zone->tooltips[spot] = tooltip;
//}
