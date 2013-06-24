#include "challenge.h"
#include "server.h"
#include "player.h"
#include "analyze.h"

Challenge::Challenge(Player *source, Player *dest, const ChallengeInfo &c, Server *s)
    :src(source), dest(dest), desc(c), server(s), cancelledFromServer(false)
{
    ChallengeInfo error = c;
    if (source->id() == dest->id()) {
        throw Exception();
    }

    if (!dest->getTiers().contains(c.desttier)) {
        error.dsc = ChallengeInfo::InvalidTier;
        source->sendChallengeStuff(error);
        throw Exception();
    }

    for (int i = 0; i < dest->teamCount(); i++) {
        if (dest->team(i).tier == c.desttier && dest->team(i).gen == source->team(c.team).gen) {
            goto tierCheckOk;
        }
    }

    error.dsc = ChallengeInfo::InvalidGen;
    source->sendChallengeStuff(error);
    throw Exception();

tierCheckOk:

    for (int i = 0; i < dest->teamCount(); i++) {
        if (dest->team(i).tier == c.desttier && !dest->team(i).invalid()) {
            goto validCheckOk;
        }
    }

    if (!(c.clauses & ChallengeInfo::ChallengeCup)) {
        error.dsc = ChallengeInfo::InvalidTeam;
        source->sendChallengeStuff(error);
        throw Exception();
    }

validCheckOk:

    if (!dest->okForChallenge(source->id())) {
        error.dsc = ChallengeInfo::Busy;
        source->sendChallengeStuff(error);
        throw Exception();
    }

    if (dest->isLocked()) {
        error.dsc = ChallengeInfo::Busy;
        source->sendChallengeStuff(error);
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

    //desc.rated = s->allowThroughChallenge && s->canHaveRatedBattle(source->id(), dest->id(), c.clauses & ChallengeInfo::ChallengeCup, false, false);

    desc.gen = source->team(c.team).gen;

    ChallengeInfo d = c;
    d.rated = false;
    d.opp = source->id();
    d.dsc = ChallengeInfo::Sent;
    d.team = 0;
    d.srctier = source->team(c.team).tier;
    d.gen = desc.gen;
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

QString Challenge::tier() const
{
    return description().desttier;
}

Pokemon::gen Challenge::gen() const
{
    return description().gen;
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

        /* Make sure the challenged one selected a team of the correct tier, correct gen */
        if (p->team(c.team).tier != desc.desttier || p->team(c.team).gen != desc.gen) {
            cancel(src);
            return;
        }

        src->removeChallenge(this);
        dest->removeChallenge(this);

        desc.rated = server->allowThroughChallenge && server->canHaveRatedBattle(src->id(), dest->id(), src->team(desc.team), dest->team(c.team), false, false);
        desc.srctier = p->team(c.team).tier;
        emit battleStarted(src->id(), dest->id(), desc, desc.team, c.team);

        delete this;
        return;
    }
    if (c.desc() == ChallengeInfo::Busy || c.desc() == ChallengeInfo::Cancelled || c.desc() == ChallengeInfo::Refused) {
        cancel(p, c.desc() == ChallengeInfo::Refused);
        return;
    }
}

void Challenge::cancel(Player *p, bool refused) {
    ChallengeInfo ret = desc;
    ret.opp = p->id();
    if (p == src) {
        ret.dsc = ChallengeInfo::Cancelled;
        dest->sendChallengeStuff(ret);
    } else {
        ret.dsc = refused ? ChallengeInfo::Refused : ChallengeInfo::Busy;
        src->sendChallengeStuff(ret);
    }
    src->removeChallenge(this);
    dest->removeChallenge(this);

    delete this;
}

void Challenge::onPlayerDisconnect(Player *p) {
    cancel(p);
}

