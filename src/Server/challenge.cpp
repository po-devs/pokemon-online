#include "challenge.h"
#include "server.h"
#include "player.h"
#include "analyze.h"

Challenge::Challenge(Player *source, Player *dest, const ChallengeInfo &c, Server *s)
    :src(source), dest(dest), desc(c), cancelledFromServer(false)
{
    if (source->id() == dest->id()) {
        throw Exception();
    }
    if (dest->gen() != source->gen()) {
        source->sendChallengeStuff(ChallengeInfo(ChallengeInfo::InvalidGen, dest->id()));
        throw Exception();
    }
    if (dest->team().invalid() && !(c.clauses & ChallengeInfo::ChallengeCup)) {
        source->sendChallengeStuff(ChallengeInfo(ChallengeInfo::InvalidTeam, dest->id()));
        throw Exception();
    }
    if (!dest->okForChallenge(source->id())) {
        source->sendChallengeStuff(ChallengeInfo(ChallengeInfo::Busy, dest->id()));
        throw Exception();
    }

    if (dest->isLocked()) {
        source->sendChallengeStuff(ChallengeInfo(ChallengeInfo::Busy, dest->id()));
        throw Exception();
    }

    s->beforeChallengeIssued(source->id(), dest->id(), this);

    if (cancelledFromServer)
        throw Exception();

    if (!source->isInSameChannel(dest)) {
        source->relay().sendPlayer(dest->bundle());
        dest->relay().sendPlayer(source->bundle());
    }

    source->addChallenge(this, false);
    dest->addChallenge(this, true);

    ChallengeInfo d = c;
    /* Now only rated battles are through Find Battle */
    desc.rated = s->allowThroughChallenge && s->canHaveRatedBattle(source->id(), dest->id(), c.clauses & ChallengeInfo::ChallengeCup, false, false);
    d.opp = source->id();
    d.dsc = ChallengeInfo::Sent;
    dest->sendChallengeStuff(d);

    s->afterChallengeIssued(source->id(), dest->id(), this);
}

void Challenge::cancelFromServer()
{
    cancelledFromServer = true;
}

int Challenge::challenged() const
{
    return dest->id();
}

int Challenge::challenger() const
{
    return src->id();
}

void Challenge::manageStuff(Player *p, const ChallengeInfo &c)
{
    if (c.desc() == ChallengeInfo::Accepted) {
        if (p != dest) {
            cancel(src);
            return;
        }
        if (!src->okForBattle()) {
            cancel(src);
            return;
        }

        src->removeChallenge(this);
        dest->removeChallenge(this);

        emit battleStarted(src->id(), dest->id(), desc);

        delete this;
        return;
    }
    if (c.desc() == ChallengeInfo::Busy || c.desc() == ChallengeInfo::Cancelled || c.desc() == ChallengeInfo::Refused) {
        cancel(p, c.desc() == ChallengeInfo::Refused);
        return;
    }
}

void Challenge::cancel(Player *p, bool refused) {
    if (p == src) {
        dest->sendChallengeStuff(ChallengeInfo(ChallengeInfo::Cancelled, src->id()));
    } else {
        src->sendChallengeStuff(ChallengeInfo(refused ? ChallengeInfo::Refused : ChallengeInfo::Busy, dest->id()));
    }
    src->removeChallenge(this);
    dest->removeChallenge(this);

    delete this;
}

void Challenge::onPlayerDisconnect(Player *p) {
    cancel(p);
}

