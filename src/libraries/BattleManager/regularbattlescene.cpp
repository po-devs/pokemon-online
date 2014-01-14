#include "regularbattlescene.h"
#include "battledata.h"
#include "battledataaccessor.h"
#include "pokemoninfoaccessor.h"
#include "proxydatacontainer.h"

#include <QLayout>
#include <Utilities/otherwidgets.h>
#include <QMouseEvent>
#include <QToolTip>

RegularBattleScene::RegularBattleScene(battledata_ptr dat, BattleDefaultTheme *theme, bool logNames) : mData(dat), unpausing(false),
    pauseCount(0), info(dat->numberOfSlots()), mLogNames(logNames)
{
    gui.theme = theme;

    /* Sets the bar in non-percentage mode for players */
    for (int i = 0; i < data()->numberOfSlots(); i++) {
        if (isPlayer(i)) {
            info.percentage[i] = false;
        }
    }

    setupGui();
    updateTimers();
}

QHBoxLayout* RegularBattleScene::createTeamLayout(QLabel **labels)
{
    QHBoxLayout *foeteam = new QHBoxLayout();
    foeteam->addStretch(100);
    for (int i = 0; i < 6; i++) {
        labels[i] = new QLabel();
        labels[i]->setPixmap(gui.theme->statusIcon(Pokemon::Fine));
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
    HPIcon->setPixmap(gui.theme->sprite("hpsquare"));
    HPIcon->setFixedSize(HPIcon->pixmap()->size());
    barL->addWidget(HPIcon);
    gui.bars[slot] = new QClickPBar();
    gui.bars[slot]->setObjectName("LifePoints"); /* for stylesheets */
    gui.bars[slot]->setRange(0, 100);
    barL->addWidget(gui.bars[slot]);

    inside->addLayout(barL,1,0,1,4);

    int player = data()->player(slot);

    if (data()->role(player) == BattleConfiguration::Player) {
        gui.bars[slot]->setFormat("%v / %m");
        connect(gui.bars[slot], SIGNAL(clicked()), SLOT(changeBarMode()));
    }

    return inside;
}

QWidget *RegularBattleScene::createFullBarLayout(int nslots, int player)
{
    QLabel* oppPoke = new QLabel();
    oppPoke->setPixmap(gui.theme->sprite("hpbar"));
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

    /* As anyway the GraphicsZone is a fixed size, it's useless to
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

    firstLine->addWidget(gui.fullBars[opponent()] = createFullBarLayout(nslots, opponent()));
    firstLine->addLayout(teamAndName[opponent()]);

    gui.zone = new GraphicsZone(data(), gui.theme);

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
    mybox->setPixmap(gui.theme->trainerSprite(data()->avatar(myself())));
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
    oppbox->setPixmap(gui.theme->trainerSprite(data()->avatar(opponent())));
    oppbox->setObjectName("OppTrainerBox");
    oppbox->setFixedSize(82,82);
    midopp->addWidget(oppbox);
    midopp->addWidget(gui.timers[opponent()]);

    lastLine->addLayout(teamAndName[myself()]);
    lastLine->addWidget(gui.fullBars[myself()] = createFullBarLayout(nslots, myself()));

    QTimer *t = new QTimer (this);
    t->start(200);
    connect(t, SIGNAL(timeout()), SLOT(updateTimers()));
}

void RegularBattleScene::onClockStart(int player, int time)
{
    info.time[player] = time;
    info.startingTime[player] = ::time(NULL);
    info.ticking[player] = true;
}

void RegularBattleScene::onClockStop(int player, int time)
{
    info.time[player] = time;
    info.ticking[player] = false;
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

RegularBattleScene::battledata_ptr RegularBattleScene::data() const
{
    return mData;
}

ProxyDataContainer * RegularBattleScene::getDataProxy()
{
    return data()->exposedData();
}

void RegularBattleScene::animateHpBar()
{
    int spot = info.animatedSpot;

    if (spot == -1) {
        return;
    }

    int current = info.animatedValue;

    const int goal = data()->poke(spot).life();

    QSettings s;
    if (!s.value("Battle/AnimateHp").toBool()) {
        updateHp(spot);
        info.animatedSpot = -1;
        unpause();
        return;
    }

    if (goal == current) {
        info.animatedSpot = -1;
        QTimer::singleShot(120, this, SLOT(unpause()));
        return;
    }

    /* We deal with true HP. 30 msec per 3 hp / % */
    bool trueHP = isPlayer(spot);
    int incr = trueHP ? 3 : 1;

    int newHp = goal < current ? std::max(goal, current - incr) : std::min(goal, current+incr);
    info.animatedValue = newHp;

    updateHp(spot, newHp);

    //Recursive call to update the hp bar 30msecs later
    QTimer::singleShot(30, this, SLOT(animateHpBar()));
}

bool RegularBattleScene::isPlayer(int spot) const
{
    return data()->role(data()->player(spot)) == BattleConfiguration::Player;
}

void RegularBattleScene::pause()
{
    pauseCount += 1;
    baseClass::pause();
}

void RegularBattleScene::unpause()
{
    pauseCount -= 1;

    if (pauseCount == 0 && !unpausing) {
        unpausing = true;
        while (commands.size() > 0) {
            AbstractCommand *command = *commands.begin();
            commands.pop_front();
            command->apply();
            delete command;
        }
        unpausing = false;
    }

    baseClass::unpause();
}

void RegularBattleScene::onUseAttack(int spot, int attack, bool) {
    emit attackUsed(spot, attack);
}

void RegularBattleScene::onPokeballStatusChanged(int player, int poke, int)
{
    updateBallStatus(player, poke);

    if (poke < data()->numberOfSlots()/2) {
        updatePoke(data()->spot(player, poke));
    }
}

void RegularBattleScene::onHpChange(int spot, int)
{
    if (!data()->isOut(spot)) {
        return;
    }
    info.animatedSpot = spot;
    if (isPlayer(spot) && info.percentage[spot]) {
        info.animatedValue = gui.bars[spot]->value() * data()->poke(spot).totalLife() / 100;
    } else {
        info.animatedValue = gui.bars[spot]->value();
    }
    pause();
    animateHpBar();
}

void RegularBattleScene::updateBall(int player, int index)
{
    auto &poke = *data()->team(player).poke(index);

    gui.pokeballs[player][index]->setToolTip(tr("%1 lv %2 -- %3%").arg(PokemonInfo::Name(poke.num())).arg(poke.level()).arg(poke.lifePercent()));
    updateBallStatus(player, index);
}

void RegularBattleScene::onShiftSpots(int player, int spot1, int spot2, bool)
{
    gui.zone->updatePoke(data()->spot(player, spot1));
    gui.zone->updatePoke(data()->spot(player, spot2));
    pause();
    QTimer::singleShot(500, this, SLOT(unpause()));
}

QString RegularBattleScene::nick(int spot) const
{
    if (mLogNames) {
        return data()->poke(spot).nickname();
    } else {
        return PokemonInfo::Name(data()->poke(spot).num());
    }
}

void RegularBattleScene::updatePoke(int spot)
{
    int player = data()->player(spot);
    int slot = data()->slotNum(spot);

    auto &poke = *data()->team(player).poke(slot);

    if (!poke.isKoed()) {
        //zone->switchTo(poke, spot, info()->sub[spot], info()->specialSprite[spot]);
        gui.nick[spot]->setText(nick(spot));
        gui.level[spot]->setText(tr("Lv. %1").arg(poke.level()));
        updateHp(spot);
        gui.gender[spot]->setPixmap(gui.theme->battleGenderPicture(poke.gender()));
        int status = poke.status();
        gui.status[spot]->setPixmap(gui.theme->battleStatusIcon(status));
    }  else {
        //zone->switchToNaught(spot);
        gui.nick[spot]->setText("");
        gui.status[spot]->setPixmap(gui.theme->battleStatusIcon(Pokemon::Fine));
        gui.gender[spot]->setPixmap(QPixmap());
        gui.bars[spot]->setValue(0);
        gui.level[spot]->setText("");
    }
    gui.fullBars[player]->update(); //needed because qt5 doesn't do its job properly

    updateBall(player, slot);
}

void RegularBattleScene::updateHp(int spot, int val)
{
    int value = val == -1 ? data()->poke(spot).life() : val;
    int pvalue = value * 100 / data()->poke(spot).totalLife();
    if (pvalue == 0 && value > 0) {
        pvalue = 1;
    }

    gui.bars[spot]->setStyleSheet(health(pvalue));
    if (info.percentage[spot]) {
        gui.bars[spot]->setValue(pvalue);
    } else {
        gui.bars[spot]->setRange(0, data()->poke(spot).totalLife());
        gui.bars[spot]->setValue(value);
    }
}

void RegularBattleScene::updateBallStatus(int player, int index)
{
    gui.pokeballs[player][index]->setPixmap(gui.theme->statusIcon(data()->team(player).poke(index)->status()));
}

QString RegularBattleScene::health(int lifePercent)
{
    return lifePercent > 50 ? "::chunk{background-color: #1fc42a;}" : (lifePercent >= 26 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
}

void RegularBattleScene::changeBarMode()
{
    int i;
    for (i = 0; i < data()->numberOfSlots(); i++) {
        if (gui.bars[i] == sender()) {
            break;
        }
    }

    gui.bars[i]->setFormat(info.percentage[i] ? "%v / %m" : "%p%");
    info.percentage[i] = !info.percentage[i];

    if (info.percentage[i])
        gui.bars[i]->setRange(0,100);

    if (info.animatedSpot == i) {
        updateHp(i, info.animatedValue);
    } else {
        updateHp(i);
    }
}

void RegularBattleScene::updateTimers()
{
    for (int i = 0; i <= 1; i++) {
        int ctime = std::max(long(0), info.ticking[i] ? info.time[i] + info.startingTime[i] - time(NULL) : info.time[i]);
        if (ctime <= 5*60) {
            gui.timers[i]->setValue(ctime);
        } else {
            gui.timers[i]->setValue(300);
        }
        gui.timers[i]->setFormat(QString("%1 : %2").arg(ctime/60).arg(QString::number(ctime%60).rightJustified(2,'0')));
        if (ctime > 60) {
            gui.timers[i]->setStyleSheet("::chunk{background-color: #55a8fc;}");
        }else if (ctime > 30) {
            gui.timers[i]->setStyleSheet("::chunk{background-color: #F8DB17;;}");
        } else {
            gui.timers[i]->setStyleSheet("::chunk{background-color: #D40202;}");
        }
    }
}

RegularBattleScene::Info::Info(int nslots)
{
    for (int i = 0; i < 2; i++) {
        time.push_back(300);
        startingTime.push_back(0);
        ticking.push_back(false);
    }

    for (int i = 0; i < nslots; i++) {
        percentage.push_back(true);
    }

    animatedSpot = -1;
    animatedValue = 0;
}

GraphicsZone::GraphicsZone(battledata_ptr i, BattleDefaultTheme *theme) : mInfo(i)
{
    setScene(&scene);
    setMouseTracking(true);

    int nslots = info()->numberOfSlots();

    tooltips.resize(nslots);
    items.resize(nslots);

    scene.setSceneRect(0,0,257,145);
    scene.addItem(new QGraphicsPixmapItem(theme->pic(QString("battle_fields/%1.png").arg((rand()%11)+1))));

    for (int i = 0; i < nslots; i++) {
        items[i] = new QGraphicsPixmapItem();
        scene.addItem(items[i]);
    }

    int size = Version::avatarSize[info()->gen().num-1];

    if (!info()->multiples()) {
        items[info()->spot(myself())]->setPos(50 - size/2, 146 - size);
        items[info()->spot(opponent())]->setPos(184- size/2, 96 - size);
    } else {
        for (int i = 0; i < nslots/2; i++) {
            items[info()->spot(myself(), i)]->setPos(i*60, 146-size);
            int base = 257-80-(nslots/2 - 1)*60;
            items[info()->spot(opponent(), i)]->setPos(base+i*60, 96 - size);
        }
    }
}

void GraphicsZone::updateToolTip(int spot)
{
    items[spot]->setToolTip(tooltips[spot]);
}

void GraphicsZone::updatePos(int spot)
{
    int player = info()->player(spot);

    int width = items[spot]->pixmap().width();
    int height = items[spot]->pixmap().height();

    if (player == myself()) {
        if (!info()->multiples()) {
            items[spot]->setPos(50 - width/2, 146 - height);
        } else {
            items[spot]->setPos(info()->slotNum(spot)*60, 146-height);
        }
    } else {
        if (!info()->multiples()) {
            items[spot]->setPos(184 - width/2, 96 - height);
        } else {
            int base = 257-80-(info()->numberOfSlots()/2 - 1)*60;
            items[spot]->setPos(base + info()->slotNum(spot)*60, 96-height);
        }
    }
}

void GraphicsZone::switchToNaught(int spot)
{
    items[spot]->setPixmap(QPixmap());
}

bool GraphicsZone::reversed() const
{
    return info()->role(1) == BattleConfiguration::Player;
}

int GraphicsZone::opponent() const
{
    return reversed() ? 0 : 1;
}

int GraphicsZone::myself() const
{
    return reversed() ? 1 : 0;
}

QPixmap GraphicsZone::loadPixmap(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub)
{
    quint64 key = this->key(num, shiny, back, gender, sub);

    if (!graphics.contains(key)) {
        QPixmap p;
        if (sub) {
            p = PokemonInfo::Sub(info()->gen(), back);
        } else {
            p = PokemonInfo::Picture(num, info()->gen(), gender, shiny, back);
        }

        QImage img = p.toImage();
        cropImage(img);
        p = QPixmap::fromImage(img);

        graphics.insert(key, p);
    }

    return graphics[key];
}

quint64 GraphicsZone::key(Pokemon::uniqueId num, bool shiny, bool back, quint8 gender, bool sub) const
{
    return sub ? ((1 << 27) + (back << 28)) : (num.pokenum + (num.subnum << 16) + (gender << 24) + (back << 26) + (shiny<<27));
}

void GraphicsZone::mouseMoveEvent(QMouseEvent * e)
{
    QGraphicsItem *it = this->itemAt(e->pos());

    for (int i = 0; i < items.size(); i++) {
        if (items[i] == it) {
            QToolTip::setFont(QFont("Courier New",8));
            QToolTip::showText(e->globalPos(), tooltips[i]);
            break;
        }
    }
}

void GraphicsZone::updatePoke(int spot)
{
    const auto &fieldInfo = info()->fieldPoke(spot);
    if (fieldInfo.showing && fieldInfo.onTheField && !info()->poke(spot).isKoed()) {
        switchTo(info()->poke(spot), spot, fieldInfo.substitute, fieldInfo.alternateSprite);
    } else {
        switchToNaught(spot);
    }
}

void RegularBattleScene::updateToolTip(int spot)
{
    QString tooltip;

    QString stats[7] = {
        tu(StatInfo::Stat(1, data()->gen())),
        tu(StatInfo::Stat(2, data()->gen())),
        tu(StatInfo::Stat(3, data()->gen())),
        tu(StatInfo::Stat(4, data()->gen())),
        tu(StatInfo::Stat(5, data()->gen())),
        tu(StatInfo::Stat(6, data()->gen())),
        tu(StatInfo::Stat(7, data()->gen()))
    };

    /* Putting dots after stat names so the ":" is always at the same place */
    int max = 0;
    for (int i = 0; i < 7; i++) {
        max = std::max(max, stats[i].length());
    }
    for (int i = 0; i < 7; i++) {
        stats[i] = stats[i].leftJustified(max, '.', false);
    }

    const auto &poke = data()->poke(spot);

    tooltip += nick(spot) + "\n";
    tooltip += TypeInfo::Name(PokemonInfo::Type1(poke.num(), data()->gen()));
    int type2 = PokemonInfo::Type2(poke.num(), data()->gen());
    if (type2 != Pokemon::Curse) {
        tooltip += " " + TypeInfo::Name(PokemonInfo::Type2(poke.num(), data()->gen()));
    }
    tooltip += "\n";

    for (int i = 0; i < 5; i++) {
        // Gen 1 only has Special, and we treat SAtk as Special hiding SDef.
        if (data()->gen().num == 1) {
            switch (i) {
            case 3: continue;
            default: tooltip += "\n" + stats[i] + " ";
            }
        } else {
            tooltip += "\n" + stats[i] + " ";
        }
        int boost = data()->fieldPoke(spot).statBoost(i+1);
        int stat = data()->fieldPoke(spot).stat(i+1);

        if (stat == 0) {
            if (boost >= 0) {
                tooltip += QString("+%1").arg(boost);
            } else if (boost < 0) {
                tooltip += QString("%1").arg(boost);
            }
        } else {
            if (stat == -1) {
                tooltip += "???";
            } else {
                tooltip += QString::number(stat);
            }
            if (boost >= 0) {
                tooltip += QString("(+%1)").arg(boost);
            } else if (boost < 0) {
                tooltip += QString("(%1)").arg(boost);
            }
        }
    }
    for (int i = 5; i < 7; i++) {
        int boost = data()->fieldPoke(spot).statBoost(i+1);
        if (boost) {
            tooltip += "\n" + stats[i] + " ";

            if (boost > 0) {
                tooltip += QString("+%1").arg(boost);
            } else if (boost < 0) {
                tooltip += QString("%1").arg(boost);
            }
        }
    }

    tooltip += "\n";

    const auto &zone = *data()->field().zone(data()->player(spot));

    if (zone.spikesLevel() > 0) {
        tooltip += "\n" + tr("Spikes level %1").arg(zone.spikesLevel());
    }

    if (zone.tspikesLevel() > 0) {
        tooltip += "\n" + tr("Toxic Spikes level %1").arg(zone.tspikesLevel());
    }

    if (zone.stealthRocks()) {
        tooltip += "\n" + tr("Stealth Rock");
    }

    if (zone.stickyWeb()) {
        tooltip += "\n" + tr("Sticky Web");
    }

    if (data()->field().weather() != Weather::NormalWeather) {
        tooltip += "\n" + tr("Weather: %1").arg(TypeInfo::weatherName(data()->field().weather()));
    }

    gui.zone->tooltips[spot] = tooltip;
}
