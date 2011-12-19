#include "battleinput.h"
#include <memory>
#include <QPair>
#include "../PokemonInfo/battlestructs.h"

template <class T> std::shared_ptr<T> mk() { return std::shared_ptr<T>(new T()); }

BattleInput::BattleInput(const BattleConfiguration *conf) {
    mCount = 0;
    delayCount = 0;
    this->conf = conf;
}

void BattleInput::receiveData(QByteArray inf)
{
    if (delayed() && inf[0] != char(BattleChat) && inf[0] != char(SpectatorChat) && inf[0] != char(ClockStart) && inf[0] != char(ClockStop)
            && inf[0] != char(Spectating)) {
        delayedCommands.push_back(inf);
        return;
    }

    QDataStream in (&inf, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_6);

    uchar command;
    qint8 player;

    in >> command >> player;

    dealWithCommandInfo(in, command, player);
}

bool BattleInput::delayed()
{
    return delayCount > 0;
}

void BattleInput::pause()
{
    delayCount++;
    qDebug() << "New delay (+): " << delayCount;
}

void BattleInput::unpause()
{
    delayCount--;
    qDebug() << "New delay (-): " << delayCount;
    if (delayCount < 0) {
        delayCount = 0;
    }

    /* As unpaused / paused can be in nested calls, class variable mCount is
      necessary */
    for ( ; mCount < delayedCommands.size() && !delayed(); ) {
        receiveData(delayedCommands[mCount++]); //The ++ inside is necessary, not oustide
    }

    delayedCommands.erase(delayedCommands.begin(), delayedCommands.begin()+mCount);
    mCount = 0;
}

void BattleInput::dealWithCommandInfo(QDataStream &in, uchar command, int spot)
{
    switch (command)
    {
    case SendOut:
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
    case SendBack:
    {
        bool silent;
        in >> silent;
        output<BattleEnum::SendBack>(spot, silent);
        break;
    }
    case UseAttack:
    {
        qint16 attack;
        in >> attack;
        output<BattleEnum::UseAttack>(spot, attack);
        break;
    }
    case BeginTurn:
    {
        int turn;
        in >> turn;
        output<BattleEnum::Turn>(turn);
        break;
    }
    case ChangeHp:
    {
        quint16 newHp;
        in >> newHp;
        output<BattleEnum::NewHp>(spot, newHp);
        break;
    }
    case Ko:
        output<BattleEnum::Ko>(spot);
        break;
    case Hit:
    {
        quint8 number;
        in >> number;
        output<BattleEnum::Hits>(spot, number);
        break;
    }
    case Effective:
    {
        quint8 eff;
        in >> eff;

        output<BattleEnum::Effectiveness>(spot, eff);
        break;
    }
    case CriticalHit:
        output<BattleEnum::CriticalHit>(spot);
        break;
    case Miss:
    {
        output<BattleEnum::Miss>(spot);
        break;
    }
    case Avoid:
    {
        output<BattleEnum::Avoid>(spot);
        break;
    }
    case StatChange:
    {
        qint8 stat, boost;
        bool silent;
        in >> stat >> boost >> silent;
        output<BattleEnum::StatChange>(spot, stat, boost, silent);
        break;
    }
    case StatusChange:
    {
        qint8 status;
        in >> status;
        bool multipleTurns;
        in >> multipleTurns;

        output<BattleEnum::ClassicStatusChange>(spot, status, multipleTurns);
        break;
    }
    case AbsStatusChange:
    {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        output<BattleEnum::AbsoluteStatusChange>(spot,poke,status);
        break;
    }
    case AlreadyStatusMessage:
    {
        quint8 status;
        in >> status;
        output<BattleEnum::AlreadyStatusMessage>(spot,status);
        break;
    }
    case StatusMessage:
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
    case Failed:
    {
        output<BattleEnum::Fail>(spot);
        break;
    }
    case BattleChat:
    case EndMessage:
    {
        auto message = mk<QString>();
        in >> *message;
        output<BattleEnum::PlayerMessage>(spot, &message);
        break;
    }
    case Spectating:
    {
        bool come;
        qint32 id;
        in >> come >> id;

        if (come) {
            auto name = mk<QString>();
            in >> *name;
            output<BattleEnum::SpectatorEnter>(id, &name);
        } else {
            output<BattleEnum::SpectatorLeave>(id);
        }
        break;
    }
    case SpectatorChat:
    {
        qint32 id;
        auto message = mk<QString>();
        in >> id >> *message;
        output<BattleEnum::SpectatorMessage>(id, &message);
        break;
    }
    case MoveMessage:
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
    case NoOpponent:
        output<BattleEnum::NoTargetMessage>(spot);
        break;
    case ItemMessage:
    {
        quint16 item=0;
        uchar part=0;
        qint8 foe = 0;
        qint16 other=0;
        qint16 berry = 0;
        in >> item >> part >> foe >> berry >> other;
        output<BattleEnum::ItemMessage>(spot, item, part, foe, berry, other);
        break;
    }
    case Flinch:
    {
        output<BattleEnum::Flinch>(spot);
        break;
    }
    case Recoil:
    {
        bool damage;
        in >> damage;

        if (damage)
            output<BattleEnum::Recoil>(spot);
        else
            output<BattleEnum::Drained>(spot);
        break;
    }
    case WeatherMessage: {
        qint8 wstatus, weather;
        in >> wstatus >> weather;
        if (weather == NormalWeather)
            break;

        switch(wstatus) {
        case EndWeather:
            output<BattleEnum::EndWeather>(weather);
            break;
        case HurtWeather:
            output<BattleEnum::WeatherDamage>(spot, weather);
            break;
        case ContinueWeather:
            output<BattleEnum::WeatherMessage>(weather);
            break;
        }
    } break;
    case StraightDamage :
    {
        qint16 damage;
        in >> damage;

        output<BattleEnum::Damaged>(spot, damage);
        break;
    }
    case AbilityMessage:
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
    case Substitute:
    {
        bool sub;
        in >> sub;
        output<BattleEnum::SubstituteStatus>(spot, sub);
        break;
    }
    case BattleEnd:
    {
        qint8 res;
        in >> res;
        output<BattleEnum::BattleEnd>(res, spot);
        break;
    }
    case BlankMessage: {
        output<BattleEnum::BlankMessage>();
        break;
    }
    case Clause:
    {
        output<BattleEnum::ClauseMessage>(spot);
        break;
    }
    case Rated:
    {
        bool rated;
        in >> rated;

        output<BattleEnum::RatedInfo>(rated);
        break;
    }
    case TierSection:
    {
        auto tier = mk<QString>();
        in >> *tier;
        output<BattleEnum::TierInfo>(&tier);
        break;
    }
    case DynamicInfo:
    {
        BattleDynamicInfo info;
        in >> info;
        output<BattleEnum::StatBoostsAndField>(spot, info);
        break;
    }
    case TempPokeChange:
    {
        quint8 type;
        in >> type;
        if (type == TempMove || type == DefMove) {
            qint8 slot;
            quint16 move;
            in >> slot >> move;

            output<BattleEnum::MoveChange>(spot, slot, move, type==DefMove);
        } else if (type == TempPP) {
            qint8 slot;
            qint8 PP;

            in >> slot >> PP;
            output<BattleEnum::TempPPChange>(spot, slot, PP);
        } else if (type == TempSprite) {
            Pokemon::uniqueId tempsprite;
            in >> tempsprite;

            if (tempsprite == -1) {
                output<BattleEnum::PokemonVanish>(spot);
            } else if (tempsprite == Pokemon::NoPoke) {
                output<BattleEnum::PokemonReappear>(spot);
            } else {
                output<BattleEnum::SpriteChange>(spot, tempsprite.toPokeRef());
            }
        } else if (type == DefiniteForme)
        {
            quint8 poke;
            Pokemon::uniqueId newforme;
            in >> poke >> newforme;
            output<BattleEnum::DefiniteFormeChange>(spot, poke, newforme.toPokeRef());
        } else if (type == AestheticForme)
        {
            quint16 newforme;
            in >> newforme;
            output<BattleEnum::CosmeticFormeChange>(spot, newforme);
        }
        break;
    }
    case ClockStart:
    {
        quint16 time;
        in >> time;
        output<BattleEnum::ClockStart>(spot, time);
        break;
    }
    case ClockStop:
    {
        quint16 time;
        in >> time;
        output<BattleEnum::ClockStop>(spot, time);
        break;
    }
    case SpotShifts:
    {
        qint8 s1, s2;
        bool silent;

        in >> s1 >> s2 >> silent;

        output<BattleEnum::ShiftSpots>(spot, s1, s2, silent);
        break;
    }
    case ChangePP:
    {
        quint8 move, pp;
        in >> move >> pp;

        output<BattleEnum::PPChange>(spot, move, pp);
        break;
    }
    case OfferChoice:
    {
        auto c = mk<BattleChoices>();
        in >> *c;

        output<BattleEnum::OfferChoice>(spot, &c);
        break;
    }
    case MakeYourChoice:
    {
        output<BattleEnum::ChoiceSelection>(spot);
        break;
    }
    case CancelMove:
    {
        output<BattleEnum::ChoiceCanceled>(spot);
        break;
    }
    case DynamicStats:
    {
        auto stats = mk<BattleStats>();
        in >> *stats;

        output<BattleEnum::DynamicStats>(spot, &stats);
        break;
    }
    case PointEstimate:
    {
        qint8 first, second;
        in >> first >> second;

        output<BattleEnum::Variation>(spot, first, second);
        break;
    }
    case RearrangeTeam:
    {
        auto t = mk<ShallowShownTeam>();
        in >> *t;

        output<BattleEnum::RearrangeTeam>(spot, &t);
    }
    default:
        /* TODO: UNKNOWN COMMAND */
        break;
    }
}
