#include <memory>
#include "battleclientlog.h"
#include "battledata.h"
#include "teamdata.h"
#include "defaulttheme.h"

typedef ShallowBattlePoke* shallowpoke;
typedef BattleData<DataContainer> battledata;

BattleClientLog::BattleClientLog(battledata *dat, BattleDefaultTheme *theme, bool lognames) : mData(dat), mTheme(theme), mLogNames(lognames)
{
    hasLoggedTeams = false;
    blankMessage = false;

    bool spectator = !(data()->role(battledata::Player1) == BattleConfiguration::Player || data()->role(battledata::Player2) == BattleConfiguration::Player);
    pushHtml("<!DOCTYPE html>");
    pushHtml(QString("<!-- Pokemon Online battle%1 log (version 3.0) -->\n").arg(spectator ? " spectator": ""));
    pushHtml(QString("<head>\n\t<title>%1 vs %2</title>\n</head>").arg(data()->name(battledata::Player1), data()->name(battledata::Player2)));
    pushHtml("<body>");

    if (!spectator) {
        printHtml("BattleStart", toBoldColor(tr("Battle between %1 and %2 started!"), Qt::blue).arg(data()->name(battledata::Player1), data()->name(battledata::Player2)));
    } else {
        printHtml("BattleStart", toBoldColor(tr("Battle between %1 and %2 is underway!"), Qt::blue).arg(data()->name(battledata::Player1), data()->name(battledata::Player2)));
    }

    onBlankMessage();
}

void BattleClientLog::emitAll()
{
    foreach(QString s, getLog()) {
        emit lineToBePrinted(s);
    }
}

BattleDefaultTheme * BattleClientLog::theme()
{
    return mTheme;
}

void BattleClientLog::printLine(const QString &cl, const QString &str, bool silent)
{
    if (str == "" && blankMessage) {
        return;
    }

    if (str == "") {
        blankMessage = true;
    } else if (!silent) {
        blankMessage = false;
    }

    if (!silent) {
        pushHtml(QString("<span class=\"%1\">%2</span><br />\n").arg(cl, str));
        emit lineToBePrinted(log.back());
    } else {
        pushHtml(QString("<!-- <span class=\"%1\">%2</span> -->\n").arg(cl, str));
    }
}

void BattleClientLog::printSilent(const QString &str)
{
    pushHtml("<!--"+str+"-->\n");
}

QStringList BattleClientLog::getLog()
{
    return log;
}

void BattleClientLog::pushHtml(const QString &html)
{
    log.push_back(html);
}

void BattleClientLog::printHtml(const QString &cl, const QString &str)
{
    blankMessage = false;

    pushHtml(QString("<span class=\"%1\">%2</span><br />\n").arg(cl, str));
    emit lineToBePrinted(log.back());
}

QString BattleClientLog::nick(int spot)
{
    if (data()->role(spot) == BattleConfiguration::Player) {
        return rnick(spot);
    } else {
        if (data()->role(data()->opponent(spot)) == BattleConfiguration::Player) {
            return tr("the foe's %1").arg(rnick(spot));
        } else {
            return tr("%1's %2").arg(data()->name(spot), rnick(spot));
        }
    }
}

QString BattleClientLog::rnick(int spot)
{
    if (mLogNames) {
        return data()->poke(spot).nick();
    } else {
        return PokemonInfo::Name(data()->poke(spot).num());
    }
}

battledata * BattleClientLog::data()
{
    return mData;
}

void BattleClientLog::onKo(int spot)
{
    printHtml("Ko", "<b>" + escapeHtml(tu(tr("%1 fainted!").arg(nick(spot)))) + "</b>");
}

void BattleClientLog::onSendOut(int spot, int prevIndex, shallowpoke, bool silent)
{
    QString pokename = PokemonInfo::Name(data()->poke(spot).num());
    if (pokename != rnick(spot))
        printLine("SendOut", tr("%1 sent out %2! (%3)").arg(data()->name(spot), rnick(spot), pokename), silent);
    else
        printLine("SendOut", tr("%1 sent out %2!").arg(data()->name(spot), rnick(spot)), silent);

    printSilent(tr("%1's previous position in the team: %2.").arg(nick(spot)).arg(prevIndex));
    printSilent(tr("%1's new place on the field: %2.").arg(nick(spot)).arg(data()->slotNum(spot)));
    printSilent(tr("%1's life: %2%.").arg(nick(spot)).arg(data()->poke(spot).lifePercent()));
    printSilent(tr("%1's status: %2.").arg(nick(spot), StatInfo::Status(data()->poke(spot).status())));
    printSilent(tr("%1's level: %2.").arg(nick(spot)).arg(data()->poke(spot).level()));
    printSilent(tr("%1's shininess: %2.").arg(nick(spot)).arg(data()->poke(spot).shiny()));
    printSilent(tr("%1's gender: %2.").arg(nick(spot)).arg(GenderInfo::Name(data()->poke(spot).gender())));
}

void BattleClientLog::onSendBack(int spot, bool silent)
{
    printLine("SendBack", tr("%1 called %2 back!").arg(data()->name(data()->player(spot)), rnick(spot)), silent);
}

void BattleClientLog::onUseAttack(int spot, int attack, bool silent)
{
    if (!silent) {
        printHtml("UseAttack", tr("%1 used %2!").arg(escapeHtml(tu(nick(spot))), toBoldColor(MoveInfo::Name(attack), theme()->typeColor(MoveInfo::Type(attack, data()->gen())))));
    }
}

void BattleClientLog::onBeginTurn(int turn)
{
    printLine("Space", "");
    printHtml("BeginTurn", toBoldColor(tr("Start of turn %1").arg(turn), Qt::blue));
}

void BattleClientLog::onHpChange(int spot, int newHp)
{
    if (data()->isPlayer(spot)) {
        printSilent(tr("%1's new HP is %2/%3.").arg(nick(spot)).arg(newHp).arg(data()->poke(spot).totalLife()));
    } else {
        printSilent(tr("%1's new HP is %2%.").arg(nick(spot)).arg(newHp));
    }
}

void BattleClientLog::onHitCount(int, int count)
{
    printLine("Hit", tr("Hit %1 time(s)!").arg(count));
}

void BattleClientLog::onEffectiveness(int spot, int effectiveness)
{
    switch (effectiveness) {
    case 0:
        printLine("Effective", tr("It had no effect on %1!").arg(nick(spot)));
        break;
    case 1:
    case 2:
        printHtml("Effective", toColor(tr("It's not very effective..."), Qt::gray));
        break;
    case 8:
    case 16:
        printHtml("Effective", toColor(tr("It's super effective!"), Qt::blue));
    default:
        break;
    }
}

void BattleClientLog::onCriticalHit(int)
{
    printHtml("CriticalHit", toColor(tr("A critical hit!"), "#6b0000"));
}

void BattleClientLog::onMiss(int spot)
{
    printLine("Miss", tr("The attack of %1 missed!").arg(nick(spot)));
}

void BattleClientLog::onAvoid(int spot)
{
    printLine("Avoid", tr("%1 avoided the attack!").arg(tu(nick(spot))));
}

void BattleClientLog::onStatBoost(int spot, int stat, int boost, bool silent)
{
    printLine("StatChange", tu(tr("%1's %2 %3%4!").arg(nick(spot), StatInfo::Stat(stat, data()->gen().num), abs(boost) > 1 ? (abs(boost) > 2 ? tr("drastically ") : tr("sharply "))
                                                                                                        : "",
                                                       boost > 0 ? tr("rose") : tr("fell"))), silent);
}

void BattleClientLog::onMajorStatusChange(int spot, int status, bool multipleTurns, bool silent)
{
    static const QString statusChangeMessages[6] = {
        tr("%1 is paralyzed! It may be unable to move!"),
        tr("%1 fell asleep!"),
        tr("%1 was frozen solid!"),
        tr("%1 was burned!"),
        tr("%1 was poisoned!"),
        tr("%1 was badly poisoned!")
    };

    if (!silent) {
        if (status > Pokemon::Fine && status <= Pokemon::Poisoned) {
            printHtml("StatusChange", toColor(tu(statusChangeMessages[status-1 + (status == Pokemon::Poisoned && multipleTurns)].arg(nick(spot))),
                                              theme()->statusColor(status)));
        } else if (status == Pokemon::Confused) {
            printHtml("StatusChange", toColor(escapeHtml(tu(tr("%1 became confused!").arg(nick(spot)))), theme()->typeColor(Type::Ghost)));
        }
    }
    printSilent(tr("%1 had its status changed to: %2.").arg(nick(spot), StatInfo::Status(status)));
}

void BattleClientLog::onPokeballStatusChanged(int player, int poke, int status)
{
    printSilent(tr("Pokemon number %1 of %2 had its status changed to: %3.").arg(poke).arg(data()->name(player), StatInfo::Status(status)));
}

void BattleClientLog::onStatusAlreadyThere(int spot, int status)
{
    printHtml("AlreadyStatusMessage", toColor(tr("%1 is already %2.").arg(tu(nick(spot)), StatInfo::Status(status)), theme()->statusColor(status)));
}

void BattleClientLog::onStatusNotification(int spot, int status)
{
    switch (status) {
    case Pokemon::Confused:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 is confused!").arg(nick(spot)))), theme()->typeColor(Type::Ghost)));
        break;
    case Pokemon::Paralysed:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 is paralyzed! It can't move!").arg(nick(spot)))), theme()->statusColor(Pokemon::Paralysed)));
        break;
    case Pokemon::Asleep:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 is fast asleep.").arg(nick(spot)))), theme()->statusColor(Pokemon::Asleep)));
        break;
    case Pokemon::Frozen:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 is frozen solid!").arg(nick(spot)))), theme()->statusColor(Pokemon::Frozen)));
    default:
        break;
    }
}

void BattleClientLog::onStatusDamage(int spot, int status)
{
    switch (status) {
    case Pokemon::Confused:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("It hurt itself in its confusion!"))), theme()->typeColor(Type::Ghost)));
        break;
    case Pokemon::Burnt:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 was hurt by its burn!").arg(nick(spot)))), theme()->statusColor(Pokemon::Burnt)));
        break;
    case Pokemon::Poisoned:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 was hurt by poison!").arg(nick(spot)))), theme()->statusColor(Pokemon::Poisoned)));
    default:
        break;
    }
}

void BattleClientLog::onStatusOver(int spot, int status)
{
    switch (status) {
    case Pokemon::Confused:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 snapped out its confusion.").arg(nick(spot)))), theme()->typeColor(Type::Dark)));
        break;
    case Pokemon::Asleep:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 woke up!").arg(nick(spot)))), theme()->typeColor(Type::Dark)));
        break;
    case Pokemon::Frozen:
        printHtml("StatusMessage", toColor(escapeHtml(tu(tr("%1 thawed out!").arg(nick(spot)))), theme()->typeColor(Type::Dark)));
    default:
        break;
    }
}

void BattleClientLog::onAttackFailing(int, bool silent)
{
    printLine("Failed", tr("But it failed!"), silent);
}

void BattleClientLog::onPlayerMessage(int spot, const QString &message)
{
    //can be 0 for winning/losing message
    if (message.length() == 0)
        return;
    printHtml("PlayerChat", QString("<span style='color:") + (spot?"#5811b1":"green") + "'><b>" + escapeHtml(data()->name(spot)) + ": </b></span>" + escapeHtml(removeTrollCharacters(message)));
}

void BattleClientLog::onSpectatorJoin(int id, const QString &name)
{
    spectators.insert(id, name);

    printHtml("Spectating", toColor(tr("%1 is watching the battle.").arg(spectators.value(id)), Qt::green));
}

void BattleClientLog::onSpectatorLeave(int id)
{
    printHtml("Spectating", toColor(tr("%1 stopped watching the battle.").arg(spectators.value(id)), Qt::green));
    spectators.remove(id);
}

void BattleClientLog::onSpectatorChat(int id, const QString &message)
{
    printHtml("SpectatorChat", toColor(spectators.value(id), Qt::blue) + ": " + escapeHtml(removeTrollCharacters(message)));
}

void BattleClientLog::onMoveMessage(int spot, int move, int part, int type, int foe, int other, const QString &q)
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
    printHtml("MoveMessage", toColor(escapeHtml(tu(mess)), theme()->typeColor(type)));
}

void BattleClientLog::onNoTarget(int)
{
    printLine("NoTarget", tr("But there was no target..."));
}

void BattleClientLog::onItemMessage(int spot, int item, int part, int foe, int berry, int other)
{
    /* Item like Potion used on a pokemon we haven't seen */
    if (data()->poke(foe).num() == Pokemon::NoPoke || data()->poke(spot).num() == Pokemon::NoPoke) {
        return;
    }
    QString mess = ItemInfo::Message(item, part);
    mess.replace("%st", StatInfo::Stat(other, data()->gen()));
    mess.replace("%s", nick(spot));
    mess.replace("%f", nick(foe));
    mess.replace("%i", ItemInfo::Name(berry));
    mess.replace("%m", MoveInfo::Name(other));
    /* Balloon gets a really special treatment */
    if (item == 35)
        printHtml("ItemMessage", QString("<b>%1</b>").arg(escapeHtml(tu(mess))));
    else
        printLine("ItemMessage", tu(mess));
}

void BattleClientLog::onFlinch(int spot)
{
    printLine("Flinch", tu(tr("%1 flinched and couldn't move!").arg(nick(spot))));
}

void BattleClientLog::onRecoil(int spot)
{
    printLine("Recoil", tu(tr("%1 is damaged by recoil!").arg(nick(spot))));
}

void BattleClientLog::onDrained(int spot)
{
    printLine("Drain", tu(tr("%1 had its energy drained!").arg(nick(spot))));
}

void BattleClientLog::onStartWeather(int spot, int weather, bool ability)
{
    QColor c = theme()->typeColor(TypeInfo::TypeForWeather(weather));

    static const QString weatherAbilityMessage[4] = {
        tr("%1's Snow Warning whipped up a hailstorm!"),
        tr("%1's Drizzle made it rain!"),
        tr("%1's Sand Stream whipped up a sandstorm!"),
        tr("%1's Drought intensified the sun's rays!")
    };

    static const QString weatherRegularMessage[4] = {
        tr("It started to hail!"),
        tr("It started to rain!"),
        tr("A sandstorm kicked up!"),
        tr("The sunlight turned harsh!")
    };

    if (ability) {
        printLine("Weather", toColor(tu(weatherAbilityMessage[weather-1]).arg(nick(spot)), c));
    } else {
        printLine("Weather", toColor(tu(weatherRegularMessage[weather-1]), c));
    }
}

void BattleClientLog::onContinueWeather(int weather)
{
    QColor c = theme()->typeColor(TypeInfo::TypeForWeather(weather));

    switch(weather) {
    case Weather::Hail: printHtml("Weather", toColor(tr("The hail crashes down."),c)); break;
    case Weather::SandStorm: printHtml("Weather", toColor(tr("The sandstorm rages."),c)); break;
    case Weather::Sunny: printHtml("Weather", toColor(tr("The sunlight is strong."),c)); break;
    case Weather::Rain: printHtml("Weather", toColor(tr("Rain continues to fall."),c)); break;
    }
}

void BattleClientLog::onEndWeather(int weather)
{
    QColor c = theme()->typeColor(TypeInfo::TypeForWeather(weather));

    switch(weather) {
    case Weather::Hail: printHtml("Weather", toColor(tr("The hail stopped."),c)); break;
    case Weather::SandStorm: printHtml("Weather", toColor(tr("The sandstorm subsided."),c)); break;
    case Weather::Sunny: printHtml("Weather", toColor(tr("The sunlight faded."),c)); break;
    case Weather::Rain: printHtml("Weather", toColor(tr("The rain stopped."),c)); break;
    }
}

void BattleClientLog::onHurtWeather(int spot, int weather)
{
    QColor c = theme()->typeColor(TypeInfo::TypeForWeather(weather));

    switch(weather) {
    case Weather::Hail: printHtml("Weather", toColor(tr("%1 is buffeted by the hail!").arg(tu(nick(spot))),c)); break;
    case Weather::SandStorm: printHtml("Weather", toColor(tr("%1 is buffeted by the sandstorm!").arg(tu(nick(spot))),c)); break;
    }
}

void BattleClientLog::onDamageDone(int spot, int damage)
{
    if (data()->role(spot) == BattleConfiguration::Spectator) {
        printLine("StraightDamage", tu(tr("%1 lost %2% of its health!").arg(nick(spot)).arg(damage)));
    } else {
        printLine("StraightDamage", tu(tr("%1 lost %2 HP! (%3% of its health)").arg(nick(spot)).arg(damage).arg(damage*100/data()->poke(spot).totalLife())));
    }
}

void BattleClientLog::onAbilityMessage(int spot, int ab, int part, int type, int foe, int other)
{
    QString mess = AbilityInfo::Message(ab,part);
    mess.replace("%st", StatInfo::Stat(other, data()->gen()));
    mess.replace("%s", nick(spot));
    //            mess.replace("%ts", data()->name(spot));
    mess.replace("%tf", data()->name(!spot));
    mess.replace("%t", TypeInfo::Name(type));
    mess.replace("%f", nick(foe));
    mess.replace("%m", MoveInfo::Name(other));
    //            mess.replace("%d", QString::number(other));
    mess.replace("%i", ItemInfo::Name(other));
    mess.replace("%a", AbilityInfo::Name(other));
    mess.replace("%p", PokemonInfo::Name(other));
    if (type == Pokemon::Normal) {
        printLine("AbilityMessage", escapeHtml(tu(mess)));
    } else {
        printHtml("AbilityMessage", toColor(escapeHtml(tu(mess)),theme()->typeColor(type)));
    }
}

void BattleClientLog::onSubstituteStatus(int spot, bool substitute)
{
    printSilent(QString("%1 has a subtitute: %2").arg(nick(spot)).arg(substitute));
}

void BattleClientLog::onBattleEnd(int res, int winner)
{
    if (res == Tie) {
        printHtml("BattleEnd", toBoldColor(tr("Tie between %1 and %2!").arg(data()->name(battledata::Player1), data()->name(battledata::Player2)), Qt::blue));
    } else if (res == Forfeit) {
        printHtml("BattleEnd", toBoldColor(tr("%1 forfeited against %2!").arg(data()->name(data()->opponent(winner)), data()->name(winner)), Qt::blue));
    } else {
        printHtml("BattleEnd", toBoldColor(tr("%1 won the battle!").arg(data()->name(winner)), Qt::blue));
    }
}

void BattleClientLog::onBlankMessage()
{
    printLine("Space", "");
}

void BattleClientLog::onClauseActivated(int clause)
{
    printLine("ClauseMessage", ChallengeInfo::battleText(clause));
}

void BattleClientLog::onRatedNotification(bool rated)
{
    /* For now, this message can still be sent even with new conf (for backward compatibility), we ignore it then */
    if (data()->oldConf()) {
        printHtml("Rated", toBoldColor(tr("Rule: "), Qt::blue) + (rated? tr("Rated") : tr("Unrated")));

        for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
            if (data()->clauses() & (1 << i)) {
                printHtml("Clause", toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(i));
            }
        }
    }
}

void BattleClientLog::setTheme(BattleDefaultTheme *theme)
{
    mTheme = theme;
}

void BattleClientLog::onTierNotification(const QString &tier)
{
    printHtml("Tier", toBoldColor(tr("Tier: "), Qt::blue) + tier);
    printHtml("Mode", toBoldColor(tr("Mode: "), Qt::blue) + ChallengeInfo::modeName(data()->mode()));

    if (!data()->oldConf()) {
        printHtml("Rated", toBoldColor(tr("Rule: "), Qt::blue) + (data()->rated()? tr("Rated") : tr("Unrated")));

        for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
            if (data()->clauses() & (1 << i)) {
                printHtml("Clause", toBoldColor(tr("Rule: "), Qt::blue) + ChallengeInfo::clause(i));
            }
        }
    }
}

void BattleClientLog::onShiftSpots(int player, int spot1, int spot2, bool silent)
{
    if (data()->poke(data()->spot(player, spot1)).status() == Pokemon::Koed) {
        printLine("ShiftSpots", tr("%1 moved to the center!").arg(tu(nick(data()->spot(player, spot2)))), silent);
    } else {
        printLine("ShiftSpots", tr("%1 shifted spots with %2!").arg(tu(nick(data()->spot(player, spot2))), nick(data()->spot(player, spot1))), silent);
    }
}

void BattleClientLog::onVariation(int, int bonus, int malus)
{
    printHtml("Variation", tr("%1+%2, %3").arg(toBoldColor(tr("Variation: "), Qt::blue)).arg(bonus).arg(malus));
}

void BattleClientLog::onRearrangeTeam(int, const ShallowShownTeam &team)
{
    if (hasLoggedTeams) {
        return;
    }
    hasLoggedTeams = true;

    int mp = data()->role(battledata::Player1) == BattleConfiguration::Player ? battledata::Player1 : battledata::Player2;

    QStringList mynames, oppnames;

    for (int i = 0; i < 6; i++) {
        Pokemon::uniqueId id = data()->team(mp).poke(i)->num();

        if (id != Pokemon::NoPoke) {
            mynames.push_back(PokemonInfo::Name(id));
        }
    }
    for (int i = 0; i < 6; i++) {
        Pokemon::uniqueId id = team.poke(i).num;

        if (id != Pokemon::NoPoke) {
            oppnames.push_back(PokemonInfo::Name(id));
        }
    }

    printHtml("Teams", toBoldColor(tr("Your team: "), Qt::blue) + mynames.join(" / "));
    printHtml("Teams", toBoldColor(tr("Opponent's team: "), Qt::blue) + oppnames.join(" / "));
    onBlankMessage();
}

void BattleClientLog::onPrintHtml(const QString &data)
{
    printHtml("ServerMessage", data);
}

void BattleClientLog::onReconnect(int player)
{
    int spot = data()->spotFromId(player);

    printHtml("Reconnect", toBoldColor(tr("%1 logged back in and is ready to resume the battle!").arg(data()->name(spot)), Qt::blue));
}

void BattleClientLog::selfDisconnection()
{
    pushHtml(QString("<br><i>Disconnected from Server!</i>"));
    emit lineToBePrinted(log.back());
}

void BattleClientLog::onDisconnect(int player)
{
    int spot = data()->spotFromId(player);

    if (data()->isPlayer(data()->opponent(player)) && !(data()->clauses() & ChallengeInfo::NoTimeOut)) {
        printHtml("Disconnect", toBoldColor(tr("%1 got disconnected! You can wait for their time to run out if you want the win.").arg(data()->name(spot)), Qt::blue));
    } else {
        printHtml("Disconnect", toBoldColor(tr("%1 got disconnected!").arg(data()->name(spot)), Qt::blue));
    }
}

void BattleClientLog::onUseItem(int spot, int item)
{
    printHtml("UseItem", tr("%1 used %2!").arg(escapeHtml(tu(data()->name(spot))), QString("<b>%1</b>").arg(ItemInfo::Name(item))));
}
