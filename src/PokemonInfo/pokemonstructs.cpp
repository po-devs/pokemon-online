#include "pokemonstructs.h"
#include "pokemoninfo.h"

#ifdef CLIENT_SIDE
#include <QDomDocument>
#include <QDomNode>
#include <QtDebug>
#endif

PokeBaseStats::PokeBaseStats(quint8 base_hp, quint8 base_att, quint8 base_def, quint8 base_spd, quint8 base_spAtt, quint8 base_spDef)
{
    setBaseHp(base_hp);
    setBaseAttack(base_att);
    setBaseDefense(base_def);
    setBaseSpeed(base_spd);
    setBaseSpAttack(base_spAtt);
    setBaseSpDefense(base_spDef);
}

quint8 PokeBaseStats::baseStat(int stat) const
{
    return m_BaseStats[stat];
}

quint8 PokeBaseStats::baseHp() const
{
    return baseStat(Hp);
}

quint8 PokeBaseStats::baseAttack() const
{
    return baseStat(Attack);
}

quint8 PokeBaseStats::baseDefense() const
{
    return baseStat(Defense);
}

quint8 PokeBaseStats::baseSpeed() const
{
    return baseStat(Speed);
}

quint8 PokeBaseStats::baseSpAttack() const
{
    return baseStat(SpAttack);
}

quint8 PokeBaseStats::baseSpDefense() const
{
    return baseStat(SpDefense);
}

void PokeBaseStats::setBaseHp(quint8 hp)
{
    setBaseStat(Hp, hp);
}

void PokeBaseStats::setBaseAttack(quint8 att)
{
    setBaseStat(Attack, att);
}

void PokeBaseStats::setBaseDefense(quint8 def)
{
    setBaseStat(Defense, def);
}

void PokeBaseStats::setBaseSpeed(quint8 speed)
{
    setBaseStat(Speed, speed);
}

void PokeBaseStats::setBaseSpAttack(quint8 spAtt)
{
    setBaseStat(SpAttack, spAtt);
}

void PokeBaseStats::setBaseSpDefense(quint8 spDef)
{
    setBaseStat(SpDefense, spDef);
}

void PokeBaseStats::setBaseStat(int stat, quint8 base)
{
    m_BaseStats[stat] = base;
}

PokeGeneral::PokeGeneral()
{
    num() = 0;
    //default for non-bugged programs
    m_abilities.push_back(0);
    m_abilities.push_back(0);
    m_genderAvail = Pokemon::NeutralAvail;
    m_types[0] = Pokemon::Curse;
    m_types[1] = -1;
}

void PokeGeneral::loadBaseStats()
{
    setBaseStats(PokemonInfo::BaseStats(num()));
}

void PokeGeneral::loadMoves()
{
    m_moves = PokemonInfo::Moves(num());
}

void PokeGeneral::loadTypes()
{
}

void PokeGeneral::loadAbilities()
{
    m_abilities = PokemonInfo::Abilities(num());
}

void PokeGeneral::loadGenderAvail()
{
    m_genderAvail = PokemonInfo::Gender(num());
}

void PokeGeneral::load()
{
    loadBaseStats();
    loadMoves();
    loadTypes();
    loadAbilities();
    loadGenderAvail();
}

const QSet<int> &PokeGeneral::moves() const
{
    return m_moves;
}

const QList<int> &PokeGeneral::abilities() const
{
    return m_abilities;
}

int PokeGeneral::genderAvail() const
{
    return m_genderAvail;
}

void PokeGeneral::setBaseStats(const PokeBaseStats &stats)
{
    m_stats = stats;
}

const PokeBaseStats & PokeGeneral::baseStats() const
{
    return m_stats;
}

PokePersonal::PokePersonal()
{
    reset();
}

void PokePersonal::setMove(int moveNum, int moveSlot, bool check)
{
    if (moveNum == move(moveSlot))
        return;
    if (check) {
        if (moveNum != 0 && hasMove(moveNum))
            throw QObject::tr("%1 already has move %2.").arg(PokemonInfo::Name(num()), MoveInfo::Name(moveNum));
    }

    m_moves[moveSlot] = moveNum;
}

int PokePersonal::addMove(int moveNum)
{
    for (int i = 0; i < 4; i++)
        if (move(i) == 0) {
            setMove(moveNum, i);
            return i;
        }
    throw QObject::tr("No free move available!");
}

bool PokePersonal::hasMove(int moveNum)
{
    for (int i = 0; i < 4; i++)
        if (move(i) == moveNum)
            return true;
    return false;
}

void PokePersonal::setDV(int stat, quint8 val)
{
    m_DVs[stat] = val;
}

void PokePersonal::setHpDV(quint8 val)
{
    setDV(Hp, val);
}

void PokePersonal::setAttackDV(quint8 val)
{
    setDV(Attack, val);
}

void PokePersonal::setDefenseDV(quint8 val)
{
    setDV(Defense, val);
}

void PokePersonal::setSpeedDV(quint8 val)
{
    setDV(Speed, val);
}

void PokePersonal::setSpAttackDV(quint8 val)
{
    setDV(SpAttack, val);
}

void PokePersonal::setSpDefenseDV(quint8 val)
{
    setDV(SpDefense, val);
}

void PokePersonal::controlEVs(int stat)
{
    int sum = EVSum();

    //if overflow we set it back to the limit
    if (sum > 510)
    {
        /* why do something so complicated? in case it's way over the limit and not simply because of stat ,
            we don't want something nasty induced by negative overflow */
        if (sum - 510 > m_EVs[stat])
            m_EVs[stat] = 0;
        else
            m_EVs[stat] -=  sum - 510;
    }
}

void PokePersonal::setEV(int stat, quint8 val)
{
    m_EVs[stat] = val;
    controlEVs(stat);
}

void PokePersonal::setHpEV(quint8 val)
{
    setEV(Hp, val);
}

void PokePersonal::setAttackEV(quint8 val)
{
    setEV(Attack, val);
}

void PokePersonal::setDefenseEV(quint8 val)
{
    setEV(Defense, val);
}

void PokePersonal::setSpeedEV(quint8 val)
{
    setEV(Speed, val);
}

void PokePersonal::setSpAttackEV(quint8 val)
{
    setEV(SpAttack, val);
}

void PokePersonal::setSpDefenseEV(quint8 val)
{
    setEV(SpDefense, val);
}

quint8 PokePersonal::DV(int stat) const
{
    return m_DVs[stat];
}

quint8 PokePersonal::hpDV() const
{
    return DV(Hp);
}

quint8 PokePersonal::attackDV() const
{
    return DV(Attack);
}

quint8 PokePersonal::defenseDV() const
{
    return DV(Defense);
}

quint8 PokePersonal::speedDV() const
{
    return DV(Speed);
}

quint8 PokePersonal::spAttackDV() const
{
    return DV(SpAttack);
}

quint8 PokePersonal::spDefenseDV() const
{
    return DV(SpDefense);
}

int PokePersonal::EVSum() const
{
    return hpEV() + attackEV() + defenseEV() + speedEV() + spAttackEV() + spDefenseEV();
}

quint8 PokePersonal::EV(int stat) const
{
    return m_EVs[stat];
}

quint8 PokePersonal::hpEV() const
{
    return EV(Hp);
}

quint8 PokePersonal::attackEV() const
{
    return EV(Attack);
}

quint8 PokePersonal::defenseEV() const
{
    return EV(Defense);
}

quint8 PokePersonal::speedEV() const
{
    return EV(Speed);
}

quint8 PokePersonal::spAttackEV() const
{
    return EV(SpAttack);
}

quint8 PokePersonal::spDefenseEV() const
{
    return EV(SpDefense);
}

int PokePersonal::move(int moveSlot) const
{
    return m_moves[moveSlot];
}

int PokePersonal::natureBoost(int stat) const
{
    return NatureInfo::Boost(nature(), stat);
}

void PokePersonal::reset()
{
    num() = 0;
    level() = 100;
    for (int i = 0; i < 4; i++)
        setMove(0,i);
    happiness() = 0;
    shiny() = false;
    gender() = Pokemon::Neutral;
    ability() = 0;
    nickname() = "";
    nature() = 0;
    item() = 0;

    for (int i = 0; i < 6; i ++) {
        setDV(i,31);
        setEV(i,0);
    }
}

#ifdef CLIENT_SIDE
PokeGraphics::PokeGraphics()
        : m_num(0), m_uptodate(false)
{
}

void PokeGraphics::setNum(int num)
{
    m_num = num;
    setUpToDate(false);
}

void PokeGraphics::setUpToDate(bool uptodate)
{
    m_uptodate = uptodate;
}

bool PokeGraphics::upToDate() const
{
    return m_uptodate;
}

void PokeGraphics::load(int gender, bool shiny)
{
    if (upToDate() && gender==m_storedgender && shiny == m_storedshininess)
        return;

    m_storedgender = gender;
    m_storedshininess = shiny;
    m_picture = PokemonInfo::Picture(num(), gender, shiny);
    setUpToDate(true);
}

void PokeGraphics::loadIcon(int index)
{
    m_icon = PokemonInfo::Icon(index);
}

QPixmap PokeGraphics::picture()
{
    return m_picture;
}

QPixmap PokeGraphics::picture(int gender, bool shiny)
{
    load(gender, shiny);
    return picture();
}

QIcon PokeGraphics::icon()
{
    return m_icon;
}

QIcon PokeGraphics::icon(int index)
{
    loadIcon(index);
    return icon();
}

int PokeGraphics::num() const
{
    return m_num;
}

PokeTeam::PokeTeam()
{
    setNum(0);
}

void PokeTeam::setNum(quint16 num)
{
    PokeGeneral::num() = num;
    PokePersonal::num() = num;
    PokeGraphics::setNum(num);
}

quint16 PokeTeam::num() const
{
    return PokeGeneral::num();
}

void PokeTeam::load()
{
    PokeGeneral::load();
    /*set the default gender & ability */
    if (genderAvail() == Pokemon::NeutralAvail)
    {
        gender() = Pokemon::Neutral;
    }
    else if (genderAvail() == Pokemon::FemaleAvail)
    {
        gender() = Pokemon::Female;
    }
    else
    {
        gender() = Pokemon::Male;
    }
    ability() = abilities()[0];
    nickname() = PokemonInfo::Name(num());
    PokeGraphics::load(gender(), false);
    PokeGraphics::loadIcon(num());
}

int PokeTeam::calc_stat(quint8 basestat, int level, quint8 ev, quint8 dv) const
{
    return ((2*basestat + dv+ ev/4)*level)/100 + 5;
}

QPixmap PokeTeam::picture()
{
    return PokeGraphics::picture(gender(), shiny());
}

QIcon PokeTeam::icon()
{
    return PokeGraphics::icon();
}

int PokeTeam::calc_stat_F(int stat) const
{
    return calc_stat(baseStats().baseStat(stat), level(), EV(stat), DV(stat)) * (10+natureBoost(stat))/10;
}

int PokeTeam::stat(int statno) const
{
    if (statno == Hp)
        return hp();
    else
        return calc_stat_F(statno);
}

int PokeTeam::hp() const
{
    /* Shedinja */
    if (num() == 292)
        return 1;
    else
        return calc_stat(baseStats().baseHp(), level(), hpEV(), hpDV()) + level() + 5;
}

int PokeTeam::attack() const
{
    return calc_stat_F(Attack);
}

int PokeTeam::defense() const
{
    return calc_stat_F(Defense);
}

int PokeTeam::speed() const
{
    return calc_stat_F(Speed);
}

int PokeTeam::spAttack() const
{
    return calc_stat_F(SpAttack);
}

int PokeTeam::spDefense() const
{
    return calc_stat_F(SpDefense);
}

Team::Team()
{
}

const PokeTeam & Team::poke(int index) const
{
    return m_pokes[index];
}

PokeTeam & Team::poke(int index)
{
    return m_pokes[index];
}

TrainerTeam::TrainerTeam()
{
}

QString TrainerTeam::trainerInfo() const
{
    return m_trainerInfo;
}

QString TrainerTeam::trainerLose() const
{
    return m_trainerLose;
}

QString TrainerTeam::trainerWin() const
{
    return m_trainerWin;
}

QString TrainerTeam::trainerNick() const
{
    return m_trainerNick;
}


void TrainerTeam::setTrainerInfo(const QString &newinfo)
{
    m_trainerInfo = newinfo;
}

void TrainerTeam::setTrainerWin(const QString &newwin)
{
    m_trainerWin = newwin;
}

void TrainerTeam::setTrainerLose(const QString &newlose)
{
    m_trainerLose = newlose;
}

void TrainerTeam::setTrainerNick(const QString &newnick)
{
    m_trainerNick = newnick;
}

const Team & TrainerTeam::team() const
{
    return m_team;
}

Team & TrainerTeam::team()
{
    return m_team;
}

#ifdef CLIENT_SIDE


bool TrainerTeam::saveToFile(const QString &path) const
{
    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(0, QObject::tr("Error while saving the team"),QObject::tr("Can't create file ")+file.fileName());
        return false;
    }
    /*QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_5);
    out << team;*/
    QDomDocument document;
    /*int line,col;
    QString msg;
    if(!document.setContent(&file,&msg,&line,&col))
    {
        QMessageBox::information(0,QObject::tr("Save Team"),QObject::tr("Erreur de parsage lors de l ouverture du fichier %1.\nErreur:%2 \nLigne:%3 Colonne:%4").arg(location).arg(msg).arg(line).arg(col));
        return false;
    }*/
    QDomElement Team = document.createElement("Team");
    document.appendChild(Team);
    QDomElement trainer = document.createElement("Trainer");
    Team.appendChild(trainer);
    QDomText trainerName = document.createTextNode(trainerNick());
    trainer.appendChild(trainerName);
    trainer.setAttribute("winMsg",trainerWin());
    trainer.setAttribute("loseMsg",trainerLose());
    trainer.setAttribute("infoMsg",trainerInfo());

    QDomElement poke[6];
    for(int cpt = 0;cpt<6;cpt++)
    {
        poke[cpt] = document.createElement("Pokemon");
        Team.appendChild(poke[cpt]);
        poke[cpt].setAttribute("Nickname", team().poke(cpt).nickname());
        poke[cpt].setAttribute("Num",team().poke(cpt).num());
        poke[cpt].setAttribute("Item",team().poke(cpt).item());
        poke[cpt].setAttribute("Ability",team().poke(cpt).ability());
        poke[cpt].setAttribute("Nature",team().poke(cpt).nature());
        poke[cpt].setAttribute("Gender",team().poke(cpt).gender());
        poke[cpt].setAttribute("Shiny",team().poke(cpt).shiny());
        poke[cpt].setAttribute("Happiness",team().poke(cpt).happiness());
        poke[cpt].setAttribute("Lvl",team().poke(cpt).level());
        int index;
        for(index = 0;index<4;index++)
        {
            QDomElement move = document.createElement("Move");
            poke[cpt].appendChild(move);
            QDomText name = document.createTextNode(QString("%1").arg(team().poke(cpt).move(index)));
            move.appendChild(name);
        }
        for(index = 0;index<6;index++)
        {
            QDomElement Dv = document.createElement("DV");
            poke[cpt].appendChild(Dv);
            QDomText Dvname = document.createTextNode(QString("%1").arg(team().poke(cpt).DV(index)));
            Dv.appendChild(Dvname);
        }
        for(index = 0;index<6;index++)
        {
            QDomElement Ev = document.createElement("EV");
            poke[cpt].appendChild(Ev);
            QDomText Evname = document.createTextNode(QString("%1").arg(team().poke(cpt).EV(index)));
            Ev.appendChild(Evname);
        }
    }

    QTextStream in(&file);
    document.save(in,4);
    return true;
}

bool saveTTeamDialog(const TrainerTeam &team, const QString &defaultPath, QString *chosenPath)
{
    QString location = QFileDialog::getSaveFileName(0,QObject::tr("Saving the Team"),defaultPath, QObject::tr("Team(*.tp)"));
    if(location.isEmpty())
    {
        //Maybe the user hit "Cancel"
        return false;
    }
    if (chosenPath) {
        *chosenPath = location;
    }

    return team.saveToFile(location);
}

bool loadTTeamDialog(TrainerTeam &team, const QString &defaultPath, QString *chosenPath)
{
    QString location = QFileDialog::getOpenFileName(0,QObject::tr("Loading the Team"),defaultPath, QObject::tr("Team(*.tp)"));
    if(location.isEmpty())
    {
        //Maybe the user hit "Cancel"
        return false;
    }
    if (chosenPath) {
        *chosenPath = location;
    }
    return team.loadFromFile(location);
}


bool TrainerTeam::loadFromFile(const QString &path)
{
    //ancienne facon de charger
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        return false;
    }
    QDomDocument document;
    QString msg;
    int line,col;
    if(!document.setContent(&file,&msg,&line,&col))
    {
        QMessageBox::information(0,QObject::tr("Load Team"),QObject::tr("Error while loading the team."));
        return false;
    }
    QDomElement team = document.firstChildElement("Team");
    if(team.isNull())
    {
        QMessageBox::information(0,QObject::tr("Load Team"),QObject::tr("Error while loading the team."));
        return false;
    }
    QDomElement trainer = team.firstChildElement("Trainer");
    if(trainer.isNull())
    {
        QMessageBox::information(0,QObject::tr("Load Team"),QObject::tr("Error while loading the team."));
        return false;
    }
    this->setTrainerNick(trainer.text());
    this->setTrainerInfo(trainer.attribute("infoMsg",QString()));
    this->setTrainerLose(trainer.attribute("loseMsg",QString()));
    this->setTrainerWin(trainer.attribute("winMsg",QString()));
    QDomElement poke = team.firstChildElement("Pokemon");
    int cpt = 0;
    while(!poke.isNull())
    {
        this->team().poke(cpt).setNum(poke.attribute("Num",0).toInt(0,10));
        this->team().poke(cpt).load();
        this->team().poke(cpt).nickname() = poke.attribute("Nickname");
        this->team().poke(cpt).item() = poke.attribute("Item",0).toInt(0,10);
        this->team().poke(cpt).ability() = poke.attribute("Ability",0).toInt(0,10);
        this->team().poke(cpt).nature() = poke.attribute("Nature",0).toInt(0,10);
        this->team().poke(cpt).gender() = poke.attribute("Gender",0).toInt(0,10);
        this->team().poke(cpt).shiny() = QVariant(poke.attribute("Shiny",false)).toBool();
        this->team().poke(cpt).happiness() = poke.attribute("Happiness",0).toInt(0,10);
        this->team().poke(cpt).level() = poke.attribute("Lvl",0).toInt(0,10);
        int cptMove=0;

        QDomElement moveElement = poke.firstChildElement("Move");
        while(!moveElement.isNull())
        {
            //QMessageBox::information(0,QObject::tr(""),QString("pokemon %1 moveElement %2:%3").arg(cpt).arg(cptMove).arg(moveElement.text()));
            this->team().poke(cpt).setMove(moveElement.text().toInt(0,10),cptMove,false);
            cptMove++;
            moveElement = moveElement.nextSiblingElement("Move");
        }
        int cptDV=0;
        QDomElement DVElement = poke.firstChildElement("DV");
        while(!DVElement.isNull())
        {
            this->team().poke(cpt).setDV(cptDV,DVElement.text().toInt(0,10));
            cptDV++;
            DVElement = DVElement.nextSiblingElement("DV");
        }
        int cptEV=0;
        QDomElement EVElement = poke.firstChildElement("EV");
        while(!EVElement.isNull())
        {
            this->team().poke(cpt).setEV(cptEV,EVElement.text().toInt(0,10));
            cptEV++;
            EVElement = EVElement.nextSiblingElement("EV");
        }

        cpt++;
        poke = poke.nextSiblingElement("Pokemon");
    }
    return true;
}
#endif

QDataStream & operator << (QDataStream & out, const Team & team)
{
    for(int index = 0;index<6;index++)
    {
        const PokeTeam & poke = team.poke(index);
        out << poke;
    }
    return out;
}


QDataStream &operator << (QDataStream &out, const TrainerTeam& trainerTeam)
{
    out << trainerTeam.trainerNick();
    out << trainerTeam.trainerInfo();
    out << trainerTeam.trainerLose();
    out << trainerTeam.trainerWin();
    out << trainerTeam.team();

    return out;
}


QDataStream &operator >> (QDataStream &in, TrainerTeam& trainerTeam)
{
    QString nick, info, lose, win;

    in >> nick;
    in >> info;
    in >> lose;
    in >> win;
    in >> trainerTeam.team();

    trainerTeam.setTrainerNick(nick);
    trainerTeam.setTrainerInfo(info);
    trainerTeam.setTrainerWin(win);
    trainerTeam.setTrainerLose(lose);

    return in;
}

QDataStream & operator >> (QDataStream & in, Team & team)
{
    for(int i=0;i<6;i++)
    {
        in >> team.poke(i);
    }

    return in;
}


QDataStream & operator >> (QDataStream & in, PokeTeam & poke)
{
    quint16 num;
    in >> num;

    poke.setNum(num);

    poke.load();

    in >> poke.nickname() >> poke.item() >> poke.ability() >> poke.nature() >> poke.gender() >> poke.shiny() >> poke.happiness() >> poke.level();

    for(int i=0;i<4;i++)
    {
        int moveNum;
        in >> moveNum;
        poke.setMove(moveNum,i);
    }
    for(int i=0;i<6;i++)
    {
        quint8 DV;
        in >> DV;
        poke.setDV(i,DV);
    }
    for(int i=0;i<6;i++)
    {
        quint8 EV;
        in >> EV;
        poke.setEV(i,EV);
    }
    return in;
}

#endif


QDataStream & operator << (QDataStream & out, const PokePersonal & Pokemon)
{
    out << Pokemon.num();
    out << Pokemon.nickname();
    out << Pokemon.item();
    out << Pokemon.ability();
    out << Pokemon.nature();
    out << Pokemon.gender();
    out << Pokemon.shiny();
    out << Pokemon.happiness();
    out << Pokemon.level();
    int i;
    for(i=0;i<4;i++)
    {
        out << Pokemon.move(i);
    }
    for(i=0;i<6;i++)
    {
        out << Pokemon.DV(i);
    }
    for(i=0;i<6;i++)
    {
        out << Pokemon.EV(i);
    }
    return out;
}

QDataStream & operator >> (QDataStream & in, PokePersonal & poke)
{
    in >> poke.num() >> poke.nickname() >> poke.item() >> poke.ability() >> poke.nature() >> poke.gender() >> poke.shiny() >> poke.happiness() >> poke.level();

    for(int i=0;i<4;i++)
    {
        int moveNum;
        in >> moveNum;
        poke.setMove(moveNum,i);
    }
    for(int i=0;i<6;i++)
    {
        quint8 DV;
        in >> DV;
        poke.setDV(i,DV);
    }
    for(int i=0;i<6;i++)
    {
        quint8 EV;
        in >> EV;
        poke.setEV(i,EV);
    }
    return in;
}
