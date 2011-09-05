#include <memory>
#include "battleclientlog.h"
#include "battledata.h"

typedef std::shared_ptr<ShallowBattlePoke> shallowpoke;

BattleClientLog::BattleClientLog(BattleData *dat) : mData(dat)
{
    pushHtml("<!DOCTYPE html>");
    pushHtml("<!-- Pokemon Online battle spectator log (version 1.1) -->");
    pushHtml(QString("<head>\n\t<title>%1 vs %2</title>\n</head>").arg(data()->name(BattleData::Player1), data()->name(BattleData::Player2)));
    pushHtml("<body>");
    printHtml(toBoldColor(tr("Battle between %1 and %2 is underway!"), Qt::blue).arg(data()->name(BattleData::Player1), data()->name(BattleData::Player2)));
}

void BattleClientLog::onKo(int spot)
{
    printHtml("<b>" + escapeHtml(tu(tr("%1 fainted!").arg(nick(spot)))) + "</b>");
}

void BattleClientLog::onSendOut(int spot, int prevIndex, shallowpoke poke, bool silent)
{
    QString pokename = PokemonInfo::Name(data()->poke(spot).num());
    if (pokename != rnick(spot))
        printLine(tr("%1 sent out %2! (%3)").arg(data()->name(spot), rnick(spot), pokename), silent);
    else
        printLine(tr("%1 sent out %2!").arg(data()->name(spot), rnick(spot)), silent);

    printLine(tr("%1's previous position in the team: %2.").arg(nick(spot)).arg(prevIndex), true);
    printLine(tr("%1's new place on the field: %2.").arg(nick(spot)).arg(data()->slotNum(spot)), true);
    printLine(tr("%1's life: %2%.").arg(nick(spot)).arg(data()->poke(spot).lifePercent()), true);
    printLine(tr("%1's status: %2.").arg(nick(spot), StatInfo::Status(data()->poke(spot).status())), true);
    printLine(tr("%1's level: %2.").arg(nick(spot)).arg(data()->poke(spot).level()), true);
    printLine(tr("%1's shininess: %2.").arg(nick(spot)).arg(data()->poke(spot).shiny()), true);
    printLine(tr("%1's gender: %2.").arg(nick(spot)).arg(GenderInfo::Name(data()->poke(spot).gender())), true);
}

void BattleClientLog::printLine(const QString &str, bool silent)
{
    if (str == "" && blankMessage) {
        return;
    }

    if (str == "") {
        blankMessage = true;
    } else if (!silent) {
        blankMessage = false;
    }

    QString html = str + "<br />";
    if (!silent) {
        pushHtml(str+"<br/>");
        emit lineToBePrinted(log.back());
    } else {
        pushHtml("<!--"+html+"-->");
    }
}

void BattleClientLog::pushHtml(const QString &html)
{
    log.push_back(html);
}

void BattleClientLog::printHtml(const QString &str, bool silent)
{
    if (!silent) {
        blankMessage = false;
    }

    if (!silent) {
        pushHtml(str+"<br/>");
        emit lineToBePrinted(log.back());
    } else {
        pushHtml("<!--"+str+"-->");
    }
}

QString BattleClientLog::nick(int spot)
{
    return data()->poke(spot).nick();
}

QString BattleClientLog::rnick(int spot)
{
    return data()->poke(spot).nick();
}

BattleData * BattleClientLog::data()
{
    return mData;
}


//void BaseBattleWindow::dealWithCommandInfo(QDataStream &in, int command, int spot, int truespot)
//{
//    switch (command)
//    {
//    case SendBack:
//        printLine(tr("%1 called %2 back!").arg(data()->name(player(spot)), rnick(spot)));
//        switchToNaught(spot);
//        break;
//    case UseAttack:
//    {
//        qint16 attack;
//        in >> attack;

//        printHtml(tr("%1 used %2!").arg(escapeHtml(tu(nick(spot))), toBoldColor(MoveInfo::Name(attack), Theme::TypeColor(MoveInfo::Type(attack, gen())))));
//        break;
//    }
//    case BeginTurn:
//    {
//        int turn;
//        in >> turn;
//        printLine("");
//        printHtml(toBoldColor(tr("Start of turn %1").arg(turn), Qt::blue));
//        break;
//    }
//    case ChangeHp:
//    {
//        quint16 newHp;
//        in >> newHp;

//        printLine(tr("%1's new HP is %2%.").arg(nick(spot)).arg(newHp), true);

//        animatedHpSpot() = spot;
//        animatedHpGoal() = newHp;
//        animateHPBar();
//        break;
//    }
//    case Hit:
//    {
//        quint8 number;
//        in >> number;
//        printLine(tr("Hit %1 times!").arg(int(number)));
//        break;
//    }
//    case Effective:
//    {
//        quint8 eff;
//        in >> eff;
//        switch (eff) {
//        case 0:
//            printLine(tr("It had no effect!"));
//            break;
//        case 1:
//        case 2:
//            printHtml(toColor(tr("It's not very effective..."), Qt::gray));
//            break;
//        case 8:
//        case 16:
//            printHtml(toColor(tr("It's super effective!"), Qt::blue));
//        default:
//            break;
//        }
//        break;
//    }
//    case CriticalHit:
//        printHtml(toColor(tr("A critical hit!"), "#6b0000"));
//        break;
//    case Miss:
//        printLine(tr("The attack of %1 missed!").arg(nick(spot)));
//        break;
//    case Avoid:
//        printLine(tr("%1 avoided the attack!").arg(tu(nick(spot))));
//        break;
//    case StatChange:
//        qint8 stat, boost;
//        in >> stat >> boost;

//        printLine(tu(tr("%1's %2 %3%4!").arg(nick(spot), StatInfo::Stat(stat), abs(boost) > 1 ? tr("sharply ") : "", boost > 0 ? tr("rose") : tr("fell"))));
//        break;
//    case StatusChange:
//    {
//        static const QString statusChangeMessages[6] = {
//            tr("%1 is paralyzed! It may be unable to move!"),
//            tr("%1 fell asleep!"),
//            tr("%1 was frozen solid!"),
//            tr("%1 was burned!"),
//            tr("%1 was poisoned!"),
//            tr("%1 was badly poisoned!")
//        };

//        qint8 status;
//        in >> status;
//        bool multipleTurns;
//        in >> multipleTurns;
//        if (status > Pokemon::Fine && status <= Pokemon::Poisoned) {
//            printHtml(toColor(tu(statusChangeMessages[status-1 + (status == Pokemon::Poisoned && multipleTurns)].arg(nick(spot))), Theme::StatusColor(status)));
//        } else if (status == Pokemon::Confused) {
//            printHtml(toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(spot)))), Theme::TypeColor(Type::Ghost).data()->name()));
//        }
//        printLine(tr("%1 had its status changed to: %2.").arg(nick(spot), StatInfo::Status(status)), true);

//        break;
//    }
//    case AbsStatusChange:
//    {
//        qint8 poke, status;
//        in >> poke >> status;

//        if (poke < 0 || poke >= 6)
//            break;

//        printLine(tr("Pokemon number %1 of %2 had its status changed to: %3.").arg(poke).arg(data()->name(spot), StatInfo::Status(status)), true);

//        if (status != Pokemon::Confused) {
//            data()->pokemons[spot][poke].changeStatus(status);
//            if (data()->isOut(spot, poke))
//                mydisplay->updatePoke(data()->slot(spot, poke));
//        }
//        mydisplay->changeStatus(spot,poke,status);
//        break;
//    }
//    case AlreadyStatusMessage:
//    {
//        quint8 status;
//        in >> status;
//        printHtml(toColor(tr("%1 is already %2.").arg(tu(nick(spot)), StatInfo::Status(status)),
//                          Theme::StatusColor(status)));
//        break;
//    }
//    case StatusMessage:
//    {
//        qint8 status;
//        in >> status;
//        switch(status)
//        {
//        case FeelConfusion:
//            printHtml(toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(spot)))), Theme::TypeColor(Type::Ghost).data()->name()));
//            break;
//        case HurtConfusion:
//            printHtml(toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), Theme::TypeColor(Type::Ghost).data()->name()));
//            break;
//        case FreeConfusion:
//            printHtml(toColor(escapeHtml(tu(tr("%1 snapped out its confusion!").arg(nick(spot)))), Theme::TypeColor(Type::Dark).data()->name()));
//            break;
//        case PrevParalysed:
//            printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Paralysed)));
//            break;
//        case FeelAsleep:
//            printHtml(toColor(escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Asleep)));
//            break;
//        case FreeAsleep:
//            printHtml(toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(spot)))), Theme::TypeColor(Type::Dark).data()->name()));
//            break;
//        case HurtBurn:
//            printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Burnt)));
//            break;
//        case HurtPoison:
//            printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Poisoned)));
//            break;
//        case PrevFrozen:
//            printHtml(toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(spot)))), Theme::StatusColor(Pokemon::Frozen)));
//            break;
//        case FreeFrozen:
//            printHtml(toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(spot)))), Theme::TypeColor(Type::Dark).data()->name()));
//            break;
//        }
//    }
//    break;
//    case Failed:
//        printLine(tr("But it failed!"));
//        break;
//    case BattleChat:
//    case EndMessage:
//    {
//        if (ignoreSpecs == IgnoreAll && data()->name(spot) != client()->data()->name(ownid()))
//            return;
//        QString message;
//        in >> message;
//        if (message=="")
//            return;
//        printHtml(QString("<span style='color:") + (spot?"#5811b1":"green") + "'><b>" + escapeHtml(data()->name(spot)) + ": </b></span>" + escapeHtml(message));
//        break;
//    }
//    case Spectating:
//    {
//        bool come;
//        qint32 id;
//        in >> come >> id;
//        addSpectator(come, id);
//        break;
//    }
//    case SpectatorChat:
//    {
//        qint32 id;
//        QString message;
//        in >> id >> message;
//        if (id != ownid() && (ignoreSpecs != NoIgnore))
//            return;
//        printHtml(toColor(client()->data()->name(id), Qt::blue) + ": " + escapeHtml(message));
//        break;
//    }
//    case MoveMessage:
//    {
//        quint16 move=0;
//        uchar part=0;
//        qint8 type(0), foe(0);
//        qint16 other(0);
//        QString q;
//        in >> move >> part >> type >> foe >> other >> q;
//        QString mess = MoveInfo::MoveMessage(move,part);
//        mess.replace("%s", nick(spot));
//        mess.replace("%ts", data()->name(player(spot)));
//        mess.replace("%tf", data()->name(opponent(player(spot))));
//        mess.replace("%t", TypeInfo::Name(type));
//        mess.replace("%f", nick(foe));
//        mess.replace("%m", MoveInfo::Name(other));
//        mess.replace("%d", QString::number(other));
//        mess.replace("%q", q);
//        mess.replace("%i", ItemInfo::Name(other));
//        mess.replace("%a", AbilityInfo::Name(other));
//        mess.replace("%p", PokemonInfo::Name(other));
//        printHtml(toColor(escapeHtml(tu(mess)), Theme::TypeColor(type)));
//        break;
//    }
//    case NoOpponent:
//        printLine(tr("But there was no target..."));
//        break;
//    case ItemMessage:
//    {
//        quint16 item=0;
//        uchar part=0;
//        qint8 foe = 0;
//        qint16 other=0;
//        qint16 berry = 0;
//        in >> item >> part >> foe >> berry >> other;
//        QString mess = ItemInfo::Message(item, part);
//        mess.replace("%st", StatInfo::Stat(other));
//        mess.replace("%s", nick(spot));
//        mess.replace("%f", nick(foe));
//        mess.replace("%i", ItemInfo::Name(berry));
//        mess.replace("%m", MoveInfo::Name(other));
//        /* Balloon gets a really special treatment */
//        if (item == 35)
//            printHtml(QString("<b>%1</b>").arg(escapeHtml(tu(mess))));
//        else
//            printLine(tu(mess));
//        break;
//    }
//    case Flinch:
//        printLine(tu(tr("%1 flinched!").arg(nick(spot))));
//        break;
//    case Recoil:
//    {
//        bool damage;
//        in >> damage;

//        if (damage)
//            printLine(tu(tr("%1 is hit with recoil!").arg(nick(spot))));
//        else
//            printLine(tu(tr("%1 had its energy drained!").arg(nick(spot))));
//        break;
//    }
//    case WeatherMessage: {
//        qint8 wstatus, weather;
//        in >> wstatus >> weather;
//        if (weather == NormalWeather)
//            break;

//        QColor c = Theme::TypeColor(TypeInfo::TypeForWeather(weather));
//        switch(wstatus) {
//        case EndWeather:
//            switch(weather) {
//            case Hail: printHtml(toColor(tr("The hail subsided!"),c)); break;
//            case SandStorm: printHtml(toColor(tr("The sandstorm subsided!"),c)); break;
//            case Sunny: printHtml(toColor(tr("The sunlight faded!"),c)); break;
//            case Rain: printHtml(toColor(tr("The rain stopped!"),c)); break;
//            } break;
//        case HurtWeather:
//            switch(weather) {
//            case Hail: printHtml(toColor(tr("%1 is buffeted by the hail!").arg(tu(nick(spot))),c)); break;
//            case SandStorm: printHtml(toColor(tr("%1 is buffeted by the sandstorm!").arg(tu(nick(spot))),c)); break;
//            } break;
//        case ContinueWeather:
//            switch(weather) {
//            case Hail: printHtml(toColor(tr("Hail continues to fall!"),c)); break;
//            case SandStorm: printHtml(toColor(tr("The sandstorm rages!"),c)); break;
//            case Sunny: printHtml(toColor(tr("The sunlight is strong!"),c)); break;
//            case Rain: printHtml(toColor(tr("Rain continues to fall!"),c)); break;
//            } break;
//        }
//    } break;
//    case StraightDamage :
//    {
//        qint16 damage;
//        in >> damage;

//        printLine(tu(tr("%1 lost %2% of its health!").arg(nick(spot)).arg(damage)));

//        break;
//    }
//    case AbilityMessage:
//    {
//        quint16 ab=0;
//        uchar part=0;
//        qint8 type(0), foe(0);
//        qint16 other(0);
//        in >> ab >> part >> type >> foe >> other;
//        QString mess = AbilityInfo::Message(ab,part);
//        mess.replace("%st", StatInfo::Stat(other));
//        mess.replace("%s", nick(spot));
//        //            mess.replace("%ts", data()->name(spot));
//        //            mess.replace("%tf", data()->name(!spot));
//        mess.replace("%t", TypeInfo::Name(type));
//        mess.replace("%f", nick(foe));
//        mess.replace("%m", MoveInfo::Name(other));
//        //            mess.replace("%d", QString::number(other));
//        mess.replace("%i", ItemInfo::Name(other));
//        mess.replace("%a", AbilityInfo::Name(other));
//        mess.replace("%p", PokemonInfo::Name(other));
//        if (type == Pokemon::Normal) {
//            printLine(escapeHtml(tu(mess)));
//        } else {
//            printHtml(toColor(escapeHtml(tu(mess)),Theme::TypeColor(type)));
//        }
//        break;
//    }
//    case Substitute:
//        in >> data()->sub[spot];
//        printLine(QString("%1 has a subtitute: %2").arg(nick(spot)).arg(data()->sub[spot]), true);
//        mydisplay->updatePoke(spot);
//        break;
//    case BattleEnd:
//    {
//        printLine("");
//        qint8 res;
//        in >> res;
//        battleEnded = true;
//        if (res == Tie) {
//            printHtml(toBoldColor(tr("Tie between %1 and %2!").arg(data()->name(data()->myself), data()->name(data()->opponent)), Qt::blue));
//        } else {
//            printHtml(toBoldColor(tr("%1 won the battle!").arg(data()->name(spot)), Qt::blue));
//        }
//        break;
//    }
//    case BlankMessage:
//        printLine("");
//        break;
//    case Clause:
//    {
//        printLine(ChallengeInfo::battleText(truespot));
//        break;
//    }
//    case Rated:
//    {
//        bool rated;
//        in >> rated;
//        printHtml(toBoldColor(tr("Rule: "), Qt::blue) + (rated? tr("Rated") : tr("Unrated")));

//        for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
//            if (conf().clauses & (1 << i)) {
//                printHtml(toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(i));
//            }
//        }

//        break;
//    }
//    case TierSection:
//    {
//        QString tier;
//        in >> tier;
//        printHtml(toBoldColor(tr("Tier: "), Qt::blue) + tier);
//        printHtml(toBoldColor(tr("Mode: "), Qt::blue) + ChallengeInfo::modeName(data()->mode));
//        break;
//    }
//    case DynamicInfo:
//    {
//        in >> data()->statChanges[spot];
//        mydisplay->updateToolTip(spot);
//        break;
//    }
//    case TempPokeChange:
//    {
//        quint8 type;
//        in >> type;
//        if (type == TempSprite) {
//            Pokemon::uniqueId old = data()->specialSprite[spot];
//            in >> data()->specialSprite[spot];
//            if (data()->specialSprite[spot] == -1) {
//                data()->lastSeenSpecialSprite[spot] = old;
//            } else if (data()->specialSprite[spot] == Pokemon::NoPoke) {
//                data()->specialSprite[spot] = data()->lastSeenSpecialSprite[spot];
//            }
//            mydisplay->updatePoke(spot);
//        } else if (type == DefiniteForme)
//        {
//            quint8 poke;
//            quint16 newform;
//            in >> poke >> newform;
//            data()->pokemons[spot][poke].num() = newform;
//            if (data()->isOut(spot, poke)) {
//                data()->poke(data()->slot(spot, poke)).num() = newform;
//            }
//        } else if (type == AestheticForme)
//        {
//            quint16 newforme;
//            in >> newforme;
//            data()->poke(spot).num().subnum = newforme;
//            mydisplay->updatePoke(spot);
//        }
//        break;
//    }
//    case ClockStart:
//    {
//        in >> data()->time[spot];
//        data()->startingTime[spot] = time(NULL);
//        data()->ticking[spot] = true;
//        break;
//    }
//    case ClockStop:
//    {
//        in >> data()->time[spot];
//        data()->ticking[spot] = false;
//        break;
//    }
//    case SpotShifts:
//    {
//        qint8 s1, s2;
//        bool silent;

//        in >> s1 >> s2 >> silent;

//        if (data()->poke(data()->slot(spot, s2)).status() == Pokemon::Koed) {
//            printLine(tr("%1 shifted spots to the middle!").arg(tu(nick(data()->slot(spot, s1)))), silent);
//        } else {
//            printLine(tr("%1 shifted spots with %2!").arg(tu(nick(data()->slot(spot, s1))), nick(data()->slot(spot, s2))), silent);
//        }

//        data()->switchOnSide(spot, s1, s2);

//        int pk1 = data()->slot(spot, s1);
//        int pk2 = data()->slot(spot, s2);
//        mydisplay->updatePoke(pk1);
//        mydisplay->updatePoke(pk2);

//        mydisplay->updatePoke(data()->player(spot), s1);
//        mydisplay->updatePoke(data()->player(spot), s2);

//        delay(500);
//        break;
//    }
//    default:
//        printLine("<i>" + tr("Unknown command received, are you up to date?") + "</i>");
//        break;
//    }
//}

//void BaseBattleWindow::addSpectator(bool come, int id)
//{
//    if (come) {
//        spectators.insert(id);
//    } else {
//        spectators.remove(id);
//    }
//    QString mess = come ? tr("%1 is watching the battle.") : tr("%1 stopped watching the battle.");
//    printHtml(toBoldColor(mess.arg(client()->data()->name(id)), Qt::green));
//}
