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

void BattleClientLog::onStatBoost(int spot, int stat, int boost, bool silent)
{
    printLine(tu(tr("%1's %2 %3%4!").arg(nick(spot), StatInfo::Stat(stat), abs(boost) > 1 ? tr("sharply ") : "",
                                         boost > 0 ? tr("rose") : tr("fell"))), silent);
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

void BattleClientLog::onDamageDone(int spot, int damage)
{
    printLine(tu(tr("%1 lost %2% of its health!").arg(nick(spot)).arg(damage)));
}

void BattleClientLog::onAbilityMessage(int spot, int ab, int part, int type, int foe, int other)
{
    QString mess = AbilityInfo::Message(ab,part);
    mess.replace("%st", StatInfo::Stat(other));
    mess.replace("%s", nick(spot));
    //            mess.replace("%ts", data()->name(spot));
    //            mess.replace("%tf", data()->name(!spot));
    mess.replace("%t", TypeInfo::Name(type));
    mess.replace("%f", nick(foe));
    mess.replace("%m", MoveInfo::Name(other));
    //            mess.replace("%d", QString::number(other));
    mess.replace("%i", ItemInfo::Name(other));
    mess.replace("%a", AbilityInfo::Name(other));
    mess.replace("%p", PokemonInfo::Name(other));
    if (type == Pokemon::Normal) {
        printLine(escapeHtml(tu(mess)));
    } else {
        printHtml(toColor(escapeHtml(tu(mess)),theme()->TypeColor(type)));
    }
}

void BattleClientLog::onSubstituteStatus(int spot, bool substitute)
{
    printLine(QString("%1 has a subtitute: %2").arg(nick(spot)).arg(substitute), true);
}

void BattleClientLog::onBattleEnd(int res, int winner)
{
    if (res == Tie) {
        printHtml(toBoldColor(tr("Tie between %1 and %2!").arg(data()->name(BattleData::Player1), data()->name(BattleData::Player2)), Qt::blue));
    } else {
        printHtml(toBoldColor(tr("%1 won the battle!").arg(data()->name(winner)), Qt::blue));
    }
}

void BattleClientLog::onBlankMessage()
{
    printLine("");
}

void BattleClientLog::onClauseActivated(int clause)
{
    printLine(ChallengeInfo::battleText(clause));
}

void BattleClientLog::onRatedNotification(bool rated)
{
    printHtml(toBoldColor(tr("Rule: "), Qt::blue) + (rated? tr("Rated") : tr("Unrated")));

//        for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
//            if (conf().clauses & (1 << i)) {
//                printHtml(toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(i));
//            }
//        }
}

void BattleClientLog::onTierNotification(QString tier)
{
    printHtml(toBoldColor(tr("Tier: "), Qt::blue) + tier);
//        printHtml(toBoldColor(tr("Mode: "), Qt::blue) + ChallengeInfo::modeName(data()->mode));
}

void BattleClientLog::onShiftSpots(int player, int spot1, int spot2, bool silent)
{
    if (data()->poke(data()->spot(player, spot1)).status() == Pokemon::Koed) {
        printLine(tr("%1 shifted spots to the middle!").arg(tu(nick(data()->spot(player, spot2)))), silent);
    } else {
        printLine(tr("%1 shifted spots with %2!").arg(tu(nick(data()->spot(player, spot2))), nick(data()->spot(player, spot1))), silent);
    }
}
