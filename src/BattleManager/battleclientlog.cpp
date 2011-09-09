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

BattleDefaultTheme * BattleClientLog::theme()
{
    return mTheme;
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

void BattleClientLog::onKo(int spot)
{
    printHtml("<b>" + escapeHtml(tu(tr("%1 fainted!").arg(nick(spot)))) + "</b>");
}

void BattleClientLog::onSendOut(int spot, int prevIndex, shallowpoke, bool silent)
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

void BattleClientLog::onSendBack(int spot)
{
    printLine(tr("%1 called %2 back!").arg(data()->name(data()->player(spot)), rnick(spot)));
}

void BattleClientLog::onUseAttack(int spot, int attack)
{
    printHtml(tr("%1 used %2!").arg(escapeHtml(tu(nick(spot))), toBoldColor(MoveInfo::Name(attack), theme()->TypeColor(MoveInfo::Type(attack, data()->gen())))));
}

void BattleClientLog::onBeginTurn(int turn)
{
    printLine("");
    printHtml(toBoldColor(tr("Start of turn %1").arg(turn), Qt::blue));
}

void BattleClientLog::onHpChange(int spot, int newHp)
{
    printLine(tr("%1's new HP is %2%.").arg(nick(spot)).arg(newHp), true);
}

void BattleClientLog::onHitCount(int, int count)
{
    printLine(tr("Hit %1 times!").arg(count));
}

void BattleClientLog::onEffectiveness(int, int effectiveness)
{
    switch (effectiveness) {
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
}

void BattleClientLog::onCriticalHit(int)
{
    printHtml(toColor(tr("A critical hit!"), "#6b0000"));
}

void BattleClientLog::onMiss(int spot)
{
    printLine(tr("The attack of %1 missed!").arg(nick(spot)));
}

void BattleClientLog::onAvoid(int spot)
{
    printLine(tr("%1 avoided the attack!").arg(tu(nick(spot))));
}

void BattleClientLog::onStatBoost(int spot, int stat, int boost)
{
    printLine(tu(tr("%1's %2 %3%4!").arg(nick(spot), StatInfo::Stat(stat), abs(boost) > 1 ? tr("sharply ") : "", boost > 0 ? tr("rose") : tr("fell"))));
}

void BattleClientLog::onMajorStatusChange(int spot, int status, bool multipleTurns)
{
    static const QString statusChangeMessages[6] = {
        tr("%1 is paralyzed! It may be unable to move!"),
        tr("%1 fell asleep!"),
        tr("%1 was frozen solid!"),
        tr("%1 was burned!"),
        tr("%1 was poisoned!"),
        tr("%1 was badly poisoned!")
    };

    if (status > Pokemon::Fine && status <= Pokemon::Poisoned) {
        printHtml(toColor(tu(statusChangeMessages[status-1 + (status == Pokemon::Poisoned && multipleTurns)].arg(nick(spot))),
                          theme()->StatusColor(status)));
    } else if (status == Pokemon::Confused) {
        printHtml(toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(spot)))), theme()->TypeColor(Type::Ghost)));
    }
    printLine(tr("%1 had its status changed to: %2.").arg(nick(spot), StatInfo::Status(status)), true);
}

void BattleClientLog::onPokeballStatusChanged(int player, int poke, int status)
{
    printLine(tr("Pokemon number %1 of %2 had its status changed to: %3.").arg(poke).arg(data()->name(player), StatInfo::Status(status)), true);
}

void BattleClientLog::onStatusAlreadyThere(int spot, int status)
{
    printHtml(toColor(tr("%1 is already %2.").arg(tu(nick(spot)), StatInfo::Status(status)), theme()->StatusColor(status)));
}

void BattleClientLog::onStatusNotification(int spot, int status)
{
    switch (status) {
    case Pokemon::Confused:
        printHtml(toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(spot)))), theme()->TypeColor(Type::Ghost)));
        break;
    case Pokemon::Paralysed:
        printHtml(toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(spot)))), theme()->StatusColor(Pokemon::Paralysed)));
        break;
    case Pokemon::Asleep:
        printHtml(toColor(escapeHtml(tu(tr("%1 is fast asleep!").arg(nick(spot)))), theme()->StatusColor(Pokemon::Asleep)));
        break;
    case Pokemon::Frozen:
        printHtml(toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(spot)))), theme()->StatusColor(Pokemon::Frozen)));
    default:
        break;
    }
}

void BattleClientLog::onStatusDamage(int spot, int status)
{
    switch (status) {
    case Pokemon::Confused:
        printHtml(toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), theme()->TypeColor(Type::Ghost)));
        break;
    case Pokemon::Burnt:
        printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by its burn!").arg(nick(spot)))), theme()->StatusColor(Pokemon::Burnt)));
        break;
    case Pokemon::Poisoned:
        printHtml(toColor(escapeHtml(tu(tr("%1 is hurt by poison!").arg(nick(spot)))), theme()->StatusColor(Pokemon::Poisoned)));
    default:
        break;
    }
}

void BattleClientLog::onStatusOver(int spot, int status)
{
    switch (status) {
    case Pokemon::Confused:
        printHtml(toColor(escapeHtml(tu(tr("%1 snapped out its confusion!").arg(nick(spot)))), theme()->TypeColor(Type::Dark)));
        break;
    case Pokemon::Asleep:
        printHtml(toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(spot)))), theme()->TypeColor(Type::Dark)));
        break;
    case Pokemon::Frozen:
        printHtml(toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(spot)))), theme()->TypeColor(Type::Dark)));
    default:
        break;
    }
}

void BattleClientLog::onAttackFailing(int)
{
    printLine(tr("But if failted!"));
}

void BattleClientLog::onPlayerMessage(int spot, QString message)
{
    printHtml(QString("<span style='color:") + (spot?"#5811b1":"green") + "'><b>" + escapeHtml(data()->name(spot)) + ": </b></span>" + escapeHtml(message));
}

void BattleClientLog::onSpectatorJoin(int id, QString name)
{
    spectators.insert(id, name);

    printHtml(toBoldColor(tr("%1 is watching the battle.").arg(spectators.value(id)), Qt::green));
}

void BattleClientLog::onSpectatorLeave(int id)
{
    printHtml(toBoldColor(tr("%1 stopped watching the battle.").arg(spectators.value(id)), Qt::green));
    spectators.remove(id);
}

void BattleClientLog::onSpectatorChat(int id, QString message)
{
    printHtml(toColor(spectators.value(id), Qt::blue) + ": " + escapeHtml(message));
}

void BattleClientLog::onMoveMessage(int spot, int move, int part, int type, int foe, int other, QString q)
{
    QString mess = MoveInfo::MoveMessage(move,part);
    mess.replace("%s", nick(spot));
    mess.replace("%ts", data()->name(data()->player(spot)));
    mess.replace("%tf", data()->name(data()->opponent(data()->player(spot))));
    mess.replace("%t", TypeInfo::Name(type));
    mess.replace("%f", nick(foe));
    mess.replace("%m", MoveInfo::Name(other));
    mess.replace("%d", QString::number(other));
    mess.replace("%q", q);
    mess.replace("%i", ItemInfo::Name(other));
    mess.replace("%a", AbilityInfo::Name(other));
    mess.replace("%p", PokemonInfo::Name(other));
    printHtml(toColor(escapeHtml(tu(mess)), theme()->TypeColor(type)));
}

void BattleClientLog::onNoTarget(int)
{
    printLine(tr("But there was no target..."));
}

void BattleClientLog::onItemMessage(int spot, int item, int part, int foe, int berry, int other)
{
    QString mess = ItemInfo::Message(item, part);
    mess.replace("%st", StatInfo::Stat(other));
    mess.replace("%s", nick(spot));
    mess.replace("%f", nick(foe));
    mess.replace("%i", ItemInfo::Name(berry));
    mess.replace("%m", MoveInfo::Name(other));
    /* Balloon gets a really special treatment */
    if (item == 35)
        printHtml(QString("<b>%1</b>").arg(escapeHtml(tu(mess))));
    else
        printLine(tu(mess));
}

void BattleClientLog::onFlinch(int spot)
{
    printLine(tu(tr("%1 flinched!").arg(nick(spot))));
}

void BattleClientLog::onRecoil(int spot)
{
    printLine(tu(tr("%1 is hit with recoil!").arg(nick(spot))));
}

void BattleClientLog::onDrained(int spot)
{
    printLine(tu(tr("%1 had its energy drained!").arg(nick(spot))));
}

void BattleClientLog::onStartWeather(int spot, int weather, bool ability)
{
    QColor c = theme()->TypeColor(TypeInfo::TypeForWeather(weather));

    static const QString weatherAbilityMessage[4] = {
        tr("%s's Snow Warning whipped up a hailstorm!"),
        tr("%s's Drizzle made it rain!"),
        tr("%s's Sand Stream whipped up a sandstorm!"),
        tr("%s's Drought intensified the sun's rays!")
    };

    static const QString weatherRegularMessage[4] = {
        tr("A hailstorm brewed!"),
        tr("It started to rain!"),
        tr("A sandstorm brewed!"),
        tr("The sunlight turned harsh!")
    };

    if (ability) {
        printLine(toColor(tu(weatherAbilityMessage[weather-1]).arg(nick(spot)), c));
    } else {
        printLine(toColor(tu(weatherRegularMessage[weather-1]), c));
    }
}

void BattleClientLog::onContinueWeather(int weather)
{
    QColor c = theme()->TypeColor(TypeInfo::TypeForWeather(weather));

    switch(weather) {
    case Weather::Hail: printHtml(toColor(tr("Hail continues to fall!"),c)); break;
    case Weather::SandStorm: printHtml(toColor(tr("The sandstorm rages!"),c)); break;
    case Weather::Sunny: printHtml(toColor(tr("The sunlight is strong!"),c)); break;
    case Weather::Rain: printHtml(toColor(tr("Rain continues to fall!"),c)); break;
    }
}

void BattleClientLog::onEndWeather(int weather)
{
    QColor c = theme()->TypeColor(TypeInfo::TypeForWeather(weather));

    switch(weather) {
    case Weather::Hail: printHtml(toColor(tr("The hail subsided!"),c)); break;
    case Weather::SandStorm: printHtml(toColor(tr("The sandstorm subsided!"),c)); break;
    case Weather::Sunny: printHtml(toColor(tr("The sunlight faded!"),c)); break;
    case Weather::Rain: printHtml(toColor(tr("The rain stopped!"),c)); break;
    }
}

void BattleClientLog::onHurtWeather(int spot, int weather)
{
    QColor c = theme()->TypeColor(TypeInfo::TypeForWeather(weather));

    switch(weather) {
    case Weather::Hail: printHtml(toColor(tr("%1 is buffeted by the hail!").arg(tu(nick(spot))),c)); break;
    case Weather::SandStorm: printHtml(toColor(tr("%1 is buffeted by the sandstorm!").arg(tu(nick(spot))),c)); break;
    }
}

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
