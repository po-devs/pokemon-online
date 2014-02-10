#include "battleinput.h"
#include <memory>
#include <QPair>
#include <PokemonInfo/battlestructs.h>
#include "../Shared/battlecommands.h"
#include <Utilities/coreclasses.h>

namespace BC = BattleCommands;

template <class T> std::shared_ptr<T> mk() { return std::shared_ptr<T>(new T()); }

BattleInput::BattleInput(const BattleConfiguration *conf) {
    mCount = 0;
    delayCount = 0;
    this->conf = conf;
}

void BattleInput::receiveData(QByteArray inf)
{
    if (inf.isEmpty()) {
        if (paused()) {
            delayedCommands.push_back(inf);
            return;
        }

        /* An empty array means raw Command */
        if (commands.size() > 0) {
            AbstractCommand *command = *commands.begin();
            commands.pop_front();
            command->apply();
            delete command;
            return;
        }
    }

    if (paused() && inf[0] != char(BC::BattleChat) && inf[0] != char(BC::SpectatorChat) && inf[0] != char(BC::ClockStart)
            && inf[0] != char(BC::ClockStop)
            && inf[0] != char(BC::Spectating)) {
        delayedCommands.push_back(inf);
        return;
    }

    DataStream in (&inf, QIODevice::ReadOnly);

    uchar command;
    qint8 player;

    in >> command >> player;

    dealWithCommandInfo(in, command, player);
}

bool BattleInput::paused()
{
    return delayCount > 0;
}

void BattleInput::pause(int ticks)
{
    delayCount+= ticks;
    //qDebug() << "New delay (+): " << delayCount;
}

void BattleInput::unpause(int ticks)
{
    delayCount-=ticks;
    //qDebug() << "New delay (-): " << delayCount;
    if (delayCount < 0) {
        delayCount = 0;
    }

    /* As unpaused / paused can be in nested calls, class variable mCount is
      necessary */
    for ( ; mCount < delayedCommands.size() && !paused(); ) {
        receiveData(delayedCommands[mCount++]); //The ++ inside is necessary, not oustide
    }

    delayedCommands.erase(delayedCommands.begin(), delayedCommands.begin()+mCount);
    mCount = 0;
}

void BattleInput::dealWithCommandInfo(DataStream &in, uchar command, int spot)
{
    switch (command)
    {
    case BC::SendOut:
    {
        bool silent;
        quint8 prevIndex;
        auto poke = mk<ShallowBattlePoke>();
        in >> silent;
        in >> prevIndex;
        in >> *poke;
        output<BattleEnum::SendOut>(spot, prevIndex, &poke, silent);
        break;
    }
    case BC::SendBack:
    {
        bool silent;
        in >> silent;
        output<BattleEnum::SendBack>(spot, silent);
        break;
    }
    case BC::UseAttack:
    {
        qint16 attack;
        bool silent;
        in >> attack >> silent;
        output<BattleEnum::UseAttack>(spot, attack, silent);
        break;
    }
    case BC::BeginTurn:
    {
        int turn;
        in >> turn;
        output<BattleEnum::Turn>(turn);
        break;
    }
    case BC::ChangeHp:
    {
        quint16 newHp;
        in >> newHp;
        output<BattleEnum::NewHp>(spot, newHp);
        break;
    }
    case BC::Ko:
        output<BattleEnum::Ko>(spot);
        break;
    case BC::Hit:
    {
        quint8 number;
        in >> number;
        output<BattleEnum::Hits>(spot, number);
        break;
    }
    case BC::Effective:
    {
        quint8 eff;
        in >> eff;

        output<BattleEnum::Effectiveness>(spot, eff);
        break;
    }
    case BC::CriticalHit:
        output<BattleEnum::CriticalHit>(spot);
        break;
    case BC::Miss:
    {
        output<BattleEnum::Miss>(spot);
        break;
    }
    case BC::Avoid:
    {
        output<BattleEnum::Avoid>(spot);
        break;
    }
    case BC::StatChange:
    {
        qint8 stat, boost;
        bool silent;
        in >> stat >> boost >> silent;
        output<BattleEnum::StatChange>(spot, stat, boost, silent);
        break;
    }
    case BC::CappedStat:
    {
        qint8 stat;
        bool maxi;
        in >> stat >> maxi;
        output<BattleEnum::CappedStat>(spot, stat, maxi);
        break;
    }
    case BC::StatusChange:
    {
        qint8 status;
        in >> status;
        bool multipleTurns, silent;
        in >> multipleTurns >> silent;

        output<BattleEnum::ClassicStatusChange>(spot, status, multipleTurns, silent);
        break;
    }
    case BC::AbsStatusChange:
    {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        output<BattleEnum::AbsoluteStatusChange>(spot,poke,status);
        break;
    }
    case BC::AlreadyStatusMessage:
    {
        quint8 status;
        in >> status;
        output<BattleEnum::AlreadyStatusMessage>(spot,status);
        break;
    }
    case BC::StatusMessage:
    {
        qint8 status;
        in >> status;
        static const QPair<BattleEnum, int> corr[]= {
            QPair<BattleEnum, int>(BattleEnum::StatusFeel, Pokemon::Confused),
            QPair<BattleEnum, int>(BattleEnum::StatusHurt, Pokemon::Confused),
            QPair<BattleEnum, int>(BattleEnum::StatusFree, Pokemon::Confused),
            QPair<BattleEnum, int>(BattleEnum::StatusFeel, Pokemon::Paralysed),
            QPair<BattleEnum, int>(BattleEnum::StatusFeel, Pokemon::Frozen),
            QPair<BattleEnum, int>(BattleEnum::StatusFree, Pokemon::Frozen),
            QPair<BattleEnum, int>(BattleEnum::StatusFeel, Pokemon::Asleep),
            QPair<BattleEnum, int>(BattleEnum::StatusFree, Pokemon::Asleep),
            QPair<BattleEnum, int>(BattleEnum::StatusHurt, Pokemon::Burnt),
            QPair<BattleEnum, int>(BattleEnum::StatusHurt, Pokemon::Poisoned)
        };
        static const int num = sizeof(corr)/sizeof(*corr);

        if (status >= 0 && status < num) {
            switch (corr[status].first) {
            case BattleEnum::StatusFeel:
                output<BattleEnum::StatusFeel>(spot, corr[status].second);
                break;
            case BattleEnum::StatusFree:
                output<BattleEnum::StatusFree>(spot, corr[status].second);
                break;
            case BattleEnum::StatusHurt:
                output<BattleEnum::StatusHurt>(spot, corr[status].second);
            default:
                break;
            }
        }
        break;
    }
    case BC::Failed:
    {
        bool silent;
        in >> silent;
        output<BattleEnum::Fail>(spot, silent);
        break;
    }
    case BC::BattleChat:
    case BC::EndMessage:
    {
        auto message = mk<QString>();
        in >> *message;
        output<BattleEnum::PlayerMessage>(spot, &message);
        break;
    }
    case BC::Spectating:
    {
        bool come;
        qint32 id;
        in >> come >> id;

        if (conf && conf->isInBattle(id)) {
            if (come) {
                output<BattleEnum::Reconnect>(id);
            } else {
                output<BattleEnum::Disconnect>(id);
            }
        } else {
            if (come) {
                auto name = mk<QString>();
                in >> *name;
                output<BattleEnum::SpectatorEnter>(id, &name);
            } else {
                output<BattleEnum::SpectatorLeave>(id);
            }
        }
        break;
    }
    case BC::SpectatorChat:
    {
        qint32 id;
        auto message = mk<QString>();
        in >> id >> *message;
        output<BattleEnum::SpectatorMessage>(id, &message);
        break;
    }
    case BC::MoveMessage:
    {
        quint16 move=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        auto q = mk<QString>();
        in >> move >> part >> type >> foe >> other >> *q;
        if (move == 57) {
            output<BattleEnum::StartWeather>(spot, part+1, false); //False for non-ability weather
        } else {
            output<BattleEnum::MoveMessage>(spot, move, part, type, foe, other, &q);
        }
        break;
    }
    case BC::NoOpponent:
        output<BattleEnum::NoTargetMessage>(spot);
        break;
    case BC::ItemMessage:
    {
        quint16 item=0;
        uchar part=0;
        qint8 foe = 0;
        qint32 other=0;
        qint16 berry = 0;
        in >> item >> part >> foe >> berry >> other;
        output<BattleEnum::ItemMessage>(spot, item, part, foe, berry, other);
        break;
    }
    case BC::Flinch:
    {
        output<BattleEnum::Flinch>(spot);
        break;
    }
    case BC::Recoil:
    {
        bool damage;
        in >> damage;

        if (damage)
            output<BattleEnum::Recoil>(spot);
        else
            output<BattleEnum::Drained>(spot);
        break;
    }
    case BC::WeatherMessage: {
        qint8 wstatus, weather;
        in >> wstatus >> weather;
        if (weather == BC::NormalWeather)
            break;

        switch(wstatus) {
        case BC::EndWeather:
            output<BattleEnum::EndWeather>(weather);
            break;
        case BC::HurtWeather:
            output<BattleEnum::WeatherDamage>(spot, weather);
            break;
        case BC::ContinueWeather:
            output<BattleEnum::WeatherMessage>(weather);
            break;
        }
    } break;
    case BC::StraightDamage:
    {
        qint16 damage;
        in >> damage;

        output<BattleEnum::Damaged>(spot, damage);
        break;
    }
    case BC::AbilityMessage:
    {
        quint16 ab=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        in >> ab >> part >> type >> foe >> other;

        if (ab == 14) {
            /* Weather message */
            output<BattleEnum::StartWeather>(spot, part+1, true); //true is for ability-weather
        } else {
            output<BattleEnum::AbilityMessage>(spot, ab, part, type, foe, other);
        }
        break;
    }
    case BC::Substitute:
    {
        bool sub;
        in >> sub;
        output<BattleEnum::SubstituteStatus>(spot, sub);
        break;
    }
    case BC::BattleEnd:
    {
        qint8 res;
        in >> res;
        output<BattleEnum::BattleEnd>(res, spot);
        break;
    }
    case BC::BlankMessage: {
        output<BattleEnum::BlankMessage>();
        break;
    }
    case BC::Clause:
    {
        output<BattleEnum::ClauseMessage>(spot);
        break;
    }
    case BC::Rated:
    {
        bool rated;
        in >> rated;

        output<BattleEnum::RatedInfo>(rated);
        break;
    }
    case BC::TierSection:
    {
        auto tier = mk<QString>();
        in >> *tier;
        output<BattleEnum::TierInfo>(&tier);
        break;
    }
    case BC::DynamicInfo:
    {
        BattleDynamicInfo info;
        in >> info;
        output<BattleEnum::StatBoostsAndField>(spot, info);
        break;
    }
    case BC::ChangeTempPoke:
    {
        quint8 type;
        in >> type;
        if (type == BC::TempMove || type == BC::DefMove) {
            qint8 slot;
            quint16 move;
            in >> slot >> move;

            output<BattleEnum::MoveChange>(spot, slot, move, type==BC::DefMove);
        } else if (type == BC::TempPP) {
            qint8 slot;
            qint8 PP;

            in >> slot >> PP;
            output<BattleEnum::TempPPChange>(spot, slot, PP);
        } else if (type == BC::TempSprite) {
            Pokemon::uniqueId tempsprite;
            in >> tempsprite;

            if (tempsprite == -1) {
                output<BattleEnum::PokemonVanish>(spot);
            } else if (tempsprite == Pokemon::NoPoke) {
                output<BattleEnum::PokemonReappear>(spot);
            } else {
                output<BattleEnum::SpriteChange>(spot, tempsprite.toPokeRef());
            }
        } else if (type == BC::DefiniteForme)
        {
            quint8 poke;
            Pokemon::uniqueId newforme;
            in >> poke >> newforme;
            output<BattleEnum::DefiniteFormeChange>(spot, poke, newforme.toPokeRef());
        } else if (type == BC::AestheticForme)
        {
            quint16 newforme;
            in >> newforme;
            output<BattleEnum::CosmeticFormeChange>(spot, newforme);
        }
        break;
    }
    case BC::ClockStart:
    {
        quint16 time;
        in >> time;
        output<BattleEnum::ClockStart>(spot, time);
        break;
    }
    case BC::ClockStop:
    {
        quint16 time;
        in >> time;
        output<BattleEnum::ClockStop>(spot, time);
        break;
    }
    case BC::SpotShifting:
    {
        qint8 s1, s2;
        bool silent;

        in >> s1 >> s2 >> silent;

        output<BattleEnum::ShiftSpots>(spot, s1, s2, silent);
        break;
    }
    case BC::ChangePP:
    {
        quint8 move, pp;
        in >> move >> pp;

        output<BattleEnum::PPChange>(spot, move, pp);
        break;
    }
    case BC::OfferChoice:
    {
        auto c = mk<BattleChoices>();
        in >> *c;

        output<BattleEnum::OfferChoice>(spot, &c);
        break;
    }
    case BC::StartChoices:
    {
        output<BattleEnum::ChoiceSelection>(spot);
        break;
    }
    case BC::CancelMove:
    {
        output<BattleEnum::ChoiceCancelled>(spot);
        break;
    }
    case BC::DynamicStats:
    {
        auto stats = mk<BattleStats>();
        in >> *stats;

        output<BattleEnum::DynamicStats>(spot, &stats);
        break;
    }
    case BC::PointEstimate:
    {
        qint8 first, second;
        in >> first >> second;

        output<BattleEnum::Variation>(spot, first, second);
        break;
    }
    case BC::RearrangeTeam:
    {
        auto t = mk<ShallowShownTeam>();
        in >> *t;

        output<BattleEnum::RearrangeTeam>(spot, &t);
        break;
    }
    case BC::ChoiceMade:
    {
        BattleChoice choice;
        in >> choice;

        if (choice.attackingChoice()) {
            output<BattleEnum::ChooseAttack>(choice.slot(), choice.attackSlot(), choice.target());
        } else if (choice.switchChoice()) {
            output<BattleEnum::ChooseSwitch>(choice.slot(), choice.pokeSlot());
        } else if (choice.cancelled()) {
            output<BattleEnum::ChooseCancel>(choice.slot());
        } else if (choice.drawChoice()) {
            output<BattleEnum::ChooseDraw>(choice.slot());
        } else if (choice.rearrangeChoice()) {
            auto c = mk<RearrangeChoice>();

            memcpy((*c).pokeIndexes, choice.choice.rearrange.pokeIndexes, sizeof((*c).pokeIndexes));
            output<BattleEnum::ChooseRearrangeTeam>(choice.slot(), &c);
        } else if (choice.moveToCenterChoice()) {
            output<BattleEnum::ChooseShiftToCenter>(choice.slot());
        }
        break;
    }
    case BC::UseItem:
    {
        quint16 item;
        in >> item;
        output<BattleEnum::UseItem>(spot, item);
        break;
    }
    case BC::ItemCountChange:
    {
        quint16 item, count;
        in >> item >> count;
        output<BattleEnum::ItemCountChange>(spot, item, count);
        break;
    }
    default:
        /* TODO: UNKNOWN COMMAND */
        break;
    }
}
