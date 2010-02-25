#include "basebattlewindow.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../Utilities/otherwidgets.h"
#include "client.h"

BaseBattleInfo::BaseBattleInfo(const QString &me, const QString &opp)
{
    name[0] = me;
    name[1] = opp;
    sub[0] = false;
    sub[1] = false;
    pokeAlive[0] = false;
    pokeAlive[1] = false;
}

BaseBattleWindow::BaseBattleWindow(const QString &me, const QString &opponent)
{
    myInfo = new BaseBattleInfo(me, opponent);
    mydisplay = new BaseBattleDisplay(info());
    init();
    show();
}

void BaseBattleWindow::init()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    QToolTip::setFont(QFont("Verdana",10));

    blankMessage = false;
    battleEnded = false;

    setWindowTitle(tr("Battle between %1 and %2").arg(info().name[0], info().name[1]));
    mylayout = new QGridLayout(this);

    mylayout->addWidget(mydisplay, 0, 0, 3, 2);
    mylayout->addWidget(mychat = new QScrollDownTextEdit(), 0, 2, 1, 2);
    mylayout->addWidget(myline = new QLineEdit(), 1, 2, 1, 2);
    mylayout->addWidget(myclose = new QPushButton(tr("&Close")), 2, 2);
    mylayout->addWidget(mysend = new QPushButton(tr("Sen&d")), 2, 3);

    connect(myclose, SIGNAL(clicked()), SLOT(clickClose()));
    connect(myline, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(mysend, SIGNAL(clicked()), SLOT(sendMessage()));

    printHtml(toBoldColor(tr("Battle between %1 and %2 started!"), Qt::blue).arg(name(true), name(false)));
    layout()->setSizeConstraint(QLayout::SetFixedSize);

}

QString BaseBattleWindow::name(int spot) const
{
    return info().name[spot];
}

QString BaseBattleWindow::nick(int player) const
{
    return tr("%1's %2").arg(name(player), rnick(player));
}

QString BaseBattleWindow::rnick(int player) const
{
    return info().pokes[player].nick();
}

void BaseBattleWindow::closeEvent(QCloseEvent *)
{
    emit closedBW(battleId());
    close();
}

void BaseBattleWindow::clickClose()
{
    emit closedBW(battleId());
    return;
}

void BaseBattleWindow::sendMessage()
{
    QString message = myline->text();

    if (message.size() != 0) {
        emit battleMessage(message, battleId());
        myline->clear();
    }
}

void BaseBattleWindow::receiveInfo(QByteArray inf)
{
    QDataStream in (&inf, QIODevice::ReadOnly);

    uchar command;
    qint8 player;

    in >> command >> player;

    dealWithCommandInfo(in, command, player);
}

void BaseBattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot)
{
    switch (command)
    {
    case SendOut:
        {
            in >> info().pokes[spot];
            info().pokeAlive[spot] = true;
            info().sub[spot] = false;
            mydisplay->updatePoke(spot);

            printLine(tr("%1 sent out %2!").arg(name(spot), rnick(spot)));

            break;
        }
    case SendBack:
        printLine(tr("%1 called %2 back!").arg(name(spot), rnick(spot)));
        switchToNaught(spot);
        break;
    case UseAttack:
        {
            qint16 attack;
            in >> attack;

            printHtml(tr("%1 used %2!").arg(escapeHtml(tu(nick(spot))), toBoldColor(MoveInfo::Name(attack), TypeInfo::Color(MoveInfo::Type(attack)))));
            break;
        }
    case BeginTurn:
        {
            int turn;
            in >> turn;
            printLine("");
            printHtml(toBoldColor(tr("Start of turn %1").arg(turn), Qt::blue));
            break;
        }
    case ChangeHp:
        {
            quint16 newHp;
            in >> newHp;
            info().pokes[spot].lifePercent() = newHp;
            mydisplay->updatePoke(spot);
            break;
        }
    case Ko:
        printHtml("<b>" + escapeHtml(tu(tr("%1 fainted!").arg(nick(spot)))) + "</b>");
        switchToNaught(spot);
        break;
    case Hit:
        printLine(tr("Hit!"));
        break;
    case Effective:
        {
            quint8 eff;
            in >> eff;
            switch (eff) {
     case 0:
                printLine(tr("It had no effect!"));
                break;
     case 1:
     case 2:
                printHtml(toColor(tr("It's not very effective..."), Qt::gray));
                break;
     case 8:
     case 16:
                printHtml(toColor(tr("It's super effective!"), Qt::blue));
     default:
                break;
            }
            break;
        }
    case CriticalHit:
        printHtml(toColor(tr("A critical hit!"), Qt::red));
        break;
    case Miss:
        printLine(tr("The attack of %1 missed!").arg(nick(spot)));
        break;
    case StatChange:
        qint8 stat, boost;
        in >> stat >> boost;

        printLine(tu(tr("%1's %2 %3%4!").arg(nick(spot), StatInfo::Stat(stat), abs(boost) > 1 ? tr("sharply ") : "", boost > 0 ? tr("rose") : tr("fell"))));
        break;
    case StatusChange:
        {
            static const QString statusChangeMessages[6] = {
                tr("%1 is paralyzed! It may be unable to move!"),
                tr("%1 was burned!"),
                tr("%1 was frozen solid!"),
                tr("%1 fell asleep!"),
                tr("%1 was poisoned!"),
                tr("%1 was badly poisoned!")
            };

            qint8 status;
            in >> status;
            if (status > 0) {
                printHtml(toColor(tu(statusChangeMessages[status-1].arg(nick(spot))), StatInfo::StatusColor(status)));
            } else if (status == -1) {
                printHtml(toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(spot)))), TypeInfo::Color(Move::Ghost).name()));
            }
            info().pokes[spot].status() = status;
            mydisplay->updatePoke(spot);
            break;
        }
    case StatusMessage:
        {
            qint8 status;
            in >> status;
            switch(status)
            {
     case FeelConfusion:
                printHtml(toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(spot)))), TypeInfo::Color(Move::Ghost).name()));
                break;
     case HurtConfusion:
                printHtml(toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), TypeInfo::Color(Move::Ghost).name()));
                break;
     case FreeConfusion:
                printHtml(toColor(escapeHtml(tu(tr("%1 snapped out its confusion!").arg(nick(spot)))), TypeInfo::Color(Move::Dark).name()));
                break;
     case PrevParalysed:
                printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Paralysed)));
                break;
     case FeelAsleep:
                printHtml(toColor(escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Asleep)));
                break;
     case FreeAsleep:
                printHtml(toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(spot)))), TypeInfo::Color(Move::Dark).name()));
                break;
     case HurtBurn:
                printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Burnt)));
                break;
     case HurtPoison:
                printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Poisoned)));
                break;
     case PrevFrozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(spot)))), StatInfo::StatusColor(Pokemon::Frozen)));
                break;
     case FreeFrozen:
                printHtml(toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(spot)))), TypeInfo::Color(Move::Dark).name()));
                break;
            }
        }
        break;
    case Failed:
        printLine(tr("But it failed!"));
        break;
    case BattleChat:
        {
            QString message;
            in >> message;
            printHtml(QString("<span style='color:") + (spot?"#5811b1":"green") + "'><b>" + escapeHtml(name(spot)) + ": </b></span>" + escapeHtml(message));
            break;
        }
    case Spectating:
        {
            bool come;
            qint32 id;
            in >> come >> id;
            QString mess = come ? tr("%1 is watching the battle.") : tr("%1 stopped watching the battle.");
            printHtml(toBoldColor(mess.arg(client()->name(id)), Qt::green));
            break;
        }
    case SpectatorChat:
        {
            qint32 id;
            QString message;
            in >> id >> message;
            printHtml(toBoldColor(client()->name(id), Qt::blue) + ": " + escapeHtml(message));
            break;
        }
    case MoveMessage:
        {
            quint16 move=0;
            uchar part=0;
            qint8 type(0), foe(0);
            qint16 other(0);
            QString q;
            in >> move >> part >> type >> foe >> other >> q;
            QString mess = MoveInfo::MoveMessage(move,part);
            mess.replace("%s", nick(spot));
            mess.replace("%ts", name(spot));
            mess.replace("%tf", name(!spot));
            mess.replace("%t", TypeInfo::Name(type));
            mess.replace("%f", nick(!spot));
            mess.replace("%m", MoveInfo::Name(other));
            mess.replace("%d", QString::number(other));
            mess.replace("%q", q);
            mess.replace("%i", ItemInfo::Name(other));
            mess.replace("%a", AbilityInfo::Name(other));
            mess.replace("%p", PokemonInfo::Name(other));
            printHtml(toColor(escapeHtml(tu(mess)), TypeInfo::Color(type)));
            break;
        }
    case NoOpponent:
        printLine(tr("But there is no target pokémon!"));
        break;
    case ItemMessage:
        {
            quint16 item=0;
            uchar part=0;
            qint8 foe = 0;
            qint16 berry = 0;
            in >> item >> part >> foe >> berry;
            QString mess = ItemInfo::Message(item, part);
            mess.replace("%s", nick(spot));
            mess.replace("%f", nick(!spot));
            mess.replace("%i", ItemInfo::Name(berry));
            mess.replace("%m", MoveInfo::Name(berry));
            printLine(tu(mess));
            break;
        }
    case Flinch:
        printLine(tu(tr("%1 flinched!").arg(nick(spot))));
        break;
    case Recoil:
        printLine(tu(tr("%1 is hit with recoil!").arg(nick(spot))));
        break;
    case WeatherMessage: {
            qint8 wstatus, weather;
            in >> wstatus >> weather;
            if (weather == NormalWeather)
                break;
            QColor c = (weather == Hail ? TypeInfo::Color(Type::Ice) : (weather == Sunny ? TypeInfo::Color(Type::Fire) : (weather == SandStorm ? TypeInfo::Color(Type::Rock) : TypeInfo::Color(Type::Water))));
            switch(wstatus) {
     case EndWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("The hail stopped!"),c)); break;
                case SandStorm: printHtml(toColor(tr("The sandstorm stopped!"),c)); break;
                case Sunny: printHtml(toColor(tr("The sunlight faded!"),c)); break;
                case Rain: printHtml(toColor(tr("The rain stopped!"),c)); break;
                } break;
                case HurtWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("%1 is buffeted by the hail!").arg(tu(nick(spot))),c)); break;
                case SandStorm: printHtml(toColor(tr("%1 is buffeted by the sandstorm!").arg(tu(nick(spot))),c)); break;
                } break;
                case StartWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("A hailstorm whipped up!"),c)); break;
                case SandStorm: printHtml(toColor(tr("A sandstorm whipped up!"),c)); break;
                case Sunny: printHtml(toColor(tr("The sunlight became harsh!"),c)); break;
                case Rain: printHtml(toColor(tr("It's started to rain!"),c)); break;
                } break;
                case ContinueWeather:
                switch(weather) {
                case Hail: printHtml(toColor(tr("Hail continues to fall!"),c)); break;
                case SandStorm: printHtml(toColor(tr("The sandstorm rages!"),c)); break;
                case Sunny: printHtml(toColor(tr("The sunlight is strong!"),c)); break;
                case Rain: printHtml(toColor(tr("Rain continues to fall!"),c)); break;
                } break;
            }
        } break;
    case StraightDamage :
        {
        qint16 damage;
        in >> damage;

        printLine(tu(tr("%1 lost %2% of its health!").arg(nick(spot)).arg(damage)));

        break;
    }
    case AbilityMessage:
        {
        quint16 ab=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        in >> ab >> part >> type >> foe >> other;
        QString mess = AbilityInfo::Message(ab,part);
        mess.replace("%s", nick(spot));
        //            mess.replace("%ts", name(spot));
        //            mess.replace("%tf", name(!spot));
        mess.replace("%t", TypeInfo::Name(type));
        mess.replace("%f", nick(!spot));
        mess.replace("%m", MoveInfo::Name(other));
        //            mess.replace("%d", QString::number(other));
        mess.replace("%i", ItemInfo::Name(other));
        mess.replace("%a", AbilityInfo::Name(other));
        //            mess.replace("%p", PokemonInfo::Name(other));
        if (type == Pokemon::Normal) {
            printLine(escapeHtml(tu(mess)));
        } else {
            printHtml(toColor(escapeHtml(tu(mess)),TypeInfo::Color(type)));
        }
        break;
    }
    case AbsStatusChange:
        {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        mydisplay->changeStatus(spot,poke,status);
        break;
    }
    case Substitute:
        in >> info().sub[spot];
        mydisplay->updatePoke(spot);
        break;
    case BattleEnd:
    {
            printLine("");
            qint8 res;
            in >> res;
            battleEnded = true;
            if (res == Tie) {
                printHtml(toBoldColor(tr("Tie between %1 and %2!").arg(name(Myself), name(Opponent)), Qt::blue));
            } else {
                printHtml(toBoldColor(tr("%1 won the battle!").arg(name(spot)), Qt::blue));
            }
            break;
    }
    case BlankMessage:
            printLine("");
            break;
    case Clause:
        {
            bool inBattle;
            in >> inBattle;

            if (inBattle) {
                printLine(ChallengeInfo::battleText(spot));
            } else {
                printHtml(toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(spot));
            }
            break;
        }
    case DynamicInfo:
        {
            in >> info().statChanges[spot];
            mydisplay->updateToolTip(spot);
            break;
        }
    default:
        break;
    }
}

void BaseBattleWindow::switchToNaught(int spot)
{
    info().pokeAlive[spot] = false;

    mydisplay->updatePoke(spot);
}

void BaseBattleWindow::printLine(const QString &str)
{
    if (str == "" && blankMessage) {
        return;
    }
    blankMessage = str == "";

    mychat->insertPlainText(str + "\n");
}

void BaseBattleWindow::printHtml(const QString &str)
{
    blankMessage = false;
    mychat->insertHtml(str + "<br />");
}

BaseBattleDisplay::BaseBattleDisplay(BaseBattleInfo &i)
    : myInfo(&i)
{
    QVBoxLayout *l=  new QVBoxLayout(this);

    /* As anyway the BaseGraphicsZone is a fixed size, it's useless to
       resize that part, might as well let  the chat be resized */
    l->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *foeteam = new QHBoxLayout();
    l->addLayout(foeteam);
    for (int i = 0; i < 6; i++) {
        advpokeballs[i] = new QLabel();
        advpokeballs[i]->setPixmap(StatInfo::Icon(Pokemon::Fine));
        foeteam->addWidget(advpokeballs[i]);
    }

    gender[Opponent] = new QLabel();
    foeteam->addWidget(gender[Opponent], 100, Qt::AlignRight);

    nick[Opponent] = new QLabel(info().name[Opponent]);
    foeteam->addWidget(nick[Opponent], 0, Qt::AlignRight);

    status[Opponent] = new QLabel();
    foeteam->addWidget(status[Opponent]);

    foeteam->setSpacing(1);

    bars[Opponent] = new QProgressBar();
    bars[Opponent]->setObjectName("LifePoints"); /* for stylesheets */
    bars[Opponent]->setRange(0, 100);
    l->addWidget(bars[Opponent]);

    zone = new BaseGraphicsZone();
    l->addWidget(zone);

    bars[Myself] = new QProgressBar();
    bars[Myself]->setObjectName("LifePoints"); /* for stylesheets */
    bars[Myself]->setRange(0,100);
    l->addWidget(bars[Myself]);

    QHBoxLayout *team = new QHBoxLayout();
    team->setSpacing(1);

    l->addLayout(team);

    gender[Myself] = new QLabel();
    team->addWidget(gender[Myself]);

    nick[Myself] = new QLabel(info().name[Myself]);
    team->addWidget(nick[Myself], 0, Qt::AlignLeft);

    status[Myself] = new QLabel();
    team->addWidget(status[Myself], 100, Qt::AlignLeft);

    team->addWidget(new QLabel(), 100);
    for (int i = 0; i < 6; i++) {
        mypokeballs[i] = new QLabel();
        mypokeballs[i]->setPixmap(StatInfo::Icon(Pokemon::Fine));
        team->addWidget(mypokeballs[i]);
    }

    updatePoke(Myself);
    updatePoke(Opponent);
}

void BaseBattleDisplay::updatePoke(int spot)
{
    if (info().pokeAlive[spot]) {
        const ShallowBattlePoke &poke = info().pokes[spot];
        zone->switchTo(poke, spot, info().sub[spot]);
        nick[spot]->setText(tr("%1 Lv.%2").arg(poke.nick()).arg(poke.level()));
        bars[spot]->setValue(poke.lifePercent());
        bars[spot]->setStyleSheet(health(poke.lifePercent()));
        gender[spot]->setPixmap(GenderInfo::Picture(poke.gender(), true));
        int status = poke.status();
        this->status[spot]->setPixmap(StatInfo::BattleIcon(status));
    }  else {
        zone->switchToNaught(spot);
        nick[spot]->setText("");
        this->status[spot]->setPixmap(StatInfo::BattleIcon(Pokemon::Fine));
        gender[spot]->setPixmap(QPixmap());
        bars[spot]->setValue(0);
    }
}

void BaseBattleDisplay::updateToolTip(int spot)
{
    QString tooltip;

    QString stats[7] = {
        tu(StatInfo::Stat(1)),
        tu(StatInfo::Stat(2)),
        tu(StatInfo::Stat(3)),
        tu(StatInfo::Stat(4)),
        tu(StatInfo::Stat(5)),
        tu(StatInfo::Stat(6)),
        tu(StatInfo::Stat(7))
    };
    int max = 0;
    for (int i = 0; i < 7; i++) {
        max = std::max(max, stats[i].length());
    }
    for (int i = 0; i < 7; i++) {
        stats[i] = stats[i].leftJustified(max, '.', false);
    }

    const ShallowBattlePoke &poke = info().pokes[spot];

    tooltip += poke.nick() + "\n";

    for (int i = 0; i < 5; i++) {
        tooltip += "\n" + stats[i] + " ";
        int boost = info().statChanges[spot].boosts[i];
        if (boost >= 0) {
            tooltip += QString("+%1").arg(boost);
        } else if (boost < 0) {
            tooltip += QString("%1").arg(boost);
        }
    }
    for (int i = 5; i < 7; i++) {
        int boost = info().statChanges[spot].boosts[i];
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

    int flags = info().statChanges[spot].flags;

    int spikes[3] = {BattleDynamicInfo::Spikes, BattleDynamicInfo::SpikesLV2 ,BattleDynamicInfo::SpikesLV3};
    for (int i = 0; i < 3; i++) {
        if (flags & spikes[i]) {
            tooltip += "\n" + tr("Spikes level %1").arg(i+1);
            break;
        }
    }

    int tspikes[2] = {BattleDynamicInfo::ToxicSpikes, BattleDynamicInfo::ToxicSpikesLV2};
    for (int i = 0; i < 2; i++) {
        if (flags & tspikes[i]) {
            tooltip += "\n" + tr("Toxic Spikes level %1").arg(i+1);
            break;
        }
    }

    if (flags & BattleDynamicInfo::StealthRock) {
        tooltip += "\n" + tr("Stealth Rock");
    }

    zone->tooltips[spot] = tooltip;
}

void BaseBattleDisplay::changeStatus(int spot, int poke, int status) {
    if (spot == Myself) {
        mypokeballs[poke]->setPixmap(StatInfo::Icon(status));
    } else {
        advpokeballs[poke]->setPixmap(StatInfo::Icon(status));
    }
}

QString BaseBattleDisplay::health(int lifePercent)
{
    return lifePercent > 50 ? "::chunk{background-color: #05B8CC;}" : (lifePercent >= 26 ? "::chunk{background-color: #F8DB17;}" : "::chunk{background-color: #D40202;}");
}

BaseGraphicsZone::BaseGraphicsZone()
{
    setScene(&scene);

    scene.setSceneRect(0,0,257,145);
    scene.addItem(new QGraphicsPixmapItem(QPixmap(QString("db/battle_fields/%1.png").arg((rand()%11)+1))));

    mine = new QGraphicsPixmapItem();
    foe = new QGraphicsPixmapItem();

    scene.addItem(mine);
    mine->setPos(10, 145-79);

    scene.addItem(foe);
    foe->setPos(257-105, 16);
}

void BaseGraphicsZone::switchToNaught(int spot)
{
    if (spot == Myself)
        mine->setPixmap(QPixmap());
    else
        foe->setPixmap(QPixmap());
}

QPixmap BaseGraphicsZone::loadPixmap(quint16 num, bool shiny, bool back, quint8 gender, bool sub)
{
    qint32 key = this->key(num, shiny, back, gender, sub);

    if (!graphics.contains(key)) {
        if (sub) {
            graphics.insert(key, PokemonInfo::Sub(back));
        } else {
            graphics.insert(key, PokemonInfo::Picture(num, gender, shiny, back));
        }
    }

    return graphics[key];
}

qint32 BaseGraphicsZone::key(quint16 num, bool shiny, bool back, quint8 gender, bool sub) const
{
    return sub ? ((1 << 31) + (back << 30)) : (num + (gender << 16) + (back << 24) + (shiny<<25));
}

bool BaseGraphicsZone::event(QEvent * event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);

        int spot  = !(helpEvent->pos().x() < width() / 2);
        QToolTip::setFont(QFont("Courier New",8));
        QToolTip::showText(helpEvent->globalPos(), tooltips[spot]);
    }
    return QGraphicsView::event(event);
}

