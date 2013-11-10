#include "pokemonstructs.h"
#include "pokemoninfo.h"
#include "movesetchecker.h"
#include "teamsaver.h"

#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QMessageBox>
#include <QFileDialog>

#include "../Utilities/coreclasses.h"

PokeBaseStats::PokeBaseStats(quint8 base_hp, quint8 base_att, quint8 base_def, quint8 base_spAtt, quint8 base_spDef, quint8 base_spd)
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
    num() = Pokemon::uniqueId();
    //default for non-bugged programs
    m_genderAvail = Pokemon::NeutralAvail;
    m_types[0] = Pokemon::Curse;
    m_types[1] = Pokemon::Curse;
}


void PokeGeneral::loadMoves()
{
    m_moves = PokemonInfo::Moves(num(), gen());
}

void PokeGeneral::loadTypes()
{
    m_types[0] = PokemonInfo::Type1(num(), gen());
    m_types[1] = PokemonInfo::Type2(num(), gen());
}

void PokeGeneral::loadAbilities()
{
    m_abilities = PokemonInfo::Abilities(num(), gen());
}

void PokeGeneral::loadGenderAvail()
{
    m_genderAvail = PokemonInfo::Gender(num());
}

void PokeGeneral::load()
{
    loadMoves();
    loadTypes();
    loadAbilities();
    loadGenderAvail();
}

const QSet<int> &PokeGeneral::moves() const
{
    return m_moves;
}

const AbilityGroup &PokeGeneral::abilities() const
{
    return m_abilities;
}

int PokeGeneral::genderAvail() const
{
    return m_genderAvail;
}

int PokeGeneral::type1() const
{
    return m_types[0];
}

int PokeGeneral::type2() const
{
    return m_types[1];
}

PokePersonal::PokePersonal()
{
    reset();
}

void PokePersonal::setMove(int moveNum, int moveSlot, bool check) throw(QString)
{
    if (moveNum == move(moveSlot))
        return;

    if (check && moveNum != 0) {
        m_moves[moveSlot] = Move::NoMove;

        if (hasMove(moveNum))
            throw QObject::tr("%1 already has move %2.").arg(nickname(), MoveInfo::Name(moveNum));
        else if (gen() > 1 && !PokemonInfo::Moves(num(), gen()).contains(moveNum))
            throw QObject::tr("%1 can't learn %2.").arg(nickname(), MoveInfo::Name(moveNum));
    }

    m_moves[moveSlot] = moveNum;

    if (check) {
        QSet<int> invalid_moves;
        QString error;
        if (!MoveSetChecker::isValid(num(), gen(), m_moves[0],m_moves[1],m_moves[2],m_moves[3],ability(),gender(),level(),false,&invalid_moves, &error)) {
            m_moves[moveSlot] = Move::NoMove;
            throw error;
        }
    }
}

void PokePersonal::runCheck()
{
    if (!PokemonInfo::Exists(num(), gen())) {
        reset();
        return;
    }

    if (!PokemonInfo::AFormesShown(num())) {
        num() = num().original();
    }

    if (gen() <= 2) {
        ability() = 0;
        nature() = 0;
    } else {
        AbilityGroup ab = PokemonInfo::Abilities(num(), gen());

        if (ability() == 0 || (ability() != ab.ab(2) && ability() != ab.ab(1)))
            ability() = ab.ab(0);
    }

    if (gen().num == 2)
        controlGender();

    if (!ItemInfo::Exists(item(), gen())) {
        item() = 0;
    }

    for (int i = 0; i < 6; i++)
        controlEVs(i);

    if (gen() <= 2) {
        for (int i = 0; i < 6; i++) {
            if (DV(i) >= 15)
                setDV(i, 15);
        }
    }

    if (gen().num == 2) {
        setEV(SpDefense, EV(SpAttack));
        setDV(SpDefense, DV(SpAttack));
    }

    int avail = PokemonInfo::Gender(num());

    if (avail != Pokemon::MaleAndFemaleAvail) {
        gender() = avail;
    } else if (gender() == Pokemon::Neutral){
        gender() = Pokemon::Male;
    }

    int minLevel = PokemonInfo::AbsoluteMinLevel(num(), gen());

    if (MoveSetChecker::enforceMinLevels && level() < minLevel) {
        level() = minLevel;
    }

    QSet<int> invalidMoves;

    MoveSetChecker::isValid(num(), gen(), move(0), move(1), move(2), move(3), ability(), gender(), level(), false, &invalidMoves);

    while (invalidMoves.size() > 0) {
        for (int i = 0; i < 4; i++) {
            if (invalidMoves.contains(move(i))) {
                setMove(0, i, false);
            }
        }
        invalidMoves.clear();

        MoveSetChecker::isValid(num(), gen(), move(0), move(1), move(2), move(3), ability(), gender(), level(), false, &invalidMoves);
    }

    if (num().pokenum == Pokemon::Keldeo) {
        if (move(0) == Move::SecretSword || move(1) == Move::SecretSword || move(2) == Move::SecretSword || move(3) == Move::SecretSword) {
            num() = Pokemon::Keldeo_R;
        } else {
            num() = num().original();
        }
    }
}

int PokePersonal::addMove(int moveNum, bool check) throw(QString)
{
    for (int i = 0; i < 4; i++)
        if (move(i) == 0) {
            setMove(moveNum, i, check);
            return i;
        }
    throw QObject::tr("No free move available!");
}

void PokePersonal::removeMove(int movenum)
{
    for (int i = 0; i < 4; i++) {
        if (move(i) == movenum) {
            setMove(0, i);
        }
    }
}

bool PokePersonal::hasMove(int moveNum)
{
    for (int i = 0; i < 4; i++)
        if (move(i) == moveNum)
            return true;
    return false;
}

void PokePersonal::controlEVs(int stat)
{
    if (gen() <= 2)
        return;

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
    if (PokemonInfo::OriginalForme(num()) == Pokemon::Arceus && gen() < 5 && val > 100)
    {
        val = 100;
    }

    if (gen().num == 2 && (stat == SpAttack || stat == SpDefense)) {
        m_EVs[SpAttack] = val;
        m_EVs[SpDefense] = val;
    } else {
        m_EVs[stat] = val;
    }

    controlEVs(stat);
}

quint8 PokePersonal::DV(int stat) const
{
    return m_DVs[stat];
}

void PokePersonal::setDV(int stat, quint8 val)
{
    if (gen().num == 2 && (stat == SpAttack || stat == SpDefense)) {
        m_DVs[SpAttack] = val;
        m_DVs[SpDefense] = val;
    } else {
        m_DVs[stat] = val;
    }

    if (gen() <= 2) {
        controlHPDV();
        if (gen().num == 2) {
            controlShininess();
            if (stat == Attack)
                controlGender();
        }
    }
}

void PokePersonal::controlShininess()
{
    shiny() = DV(Defense) == 10 && DV(Speed) == 10 && DV(SpAttack) == 10 && DV(Attack) % 4 >= 2;
}

void PokePersonal::controlGender()
{
    if (PokemonInfo::Gender(num()) == Pokemon::MaleAndFemaleAvail) {
        int rate = PokemonInfo::GenderRate(num());

        if (DV(Attack) < (8-rate) * 2) {
            gender() = Pokemon::Female;
        } else {
            gender() = Pokemon::Male;
        }
    }
}

void PokePersonal::controlHPDV()
{
    m_DVs[Hp] = ((DV(Attack) & 1) << 3) + ((DV(Defense) & 1) << 2) + ((DV(Speed) & 1) << 1) + (DV(SpAttack) & 1);
}

int PokePersonal::EVSum() const
{
    return EV(Hp) + EV(Attack) + EV(Defense) + EV(Speed) + EV(SpAttack) + EV(SpDefense);
}

quint8 PokePersonal::EV(int stat) const
{
    return m_EVs[stat];
}

quint16 PokePersonal::move(int moveSlot) const
{
    return m_moves[moveSlot];
}

int PokePersonal::natureBoost(int stat) const
{
    return NatureInfo::Boost(nature(), stat);
}

void PokePersonal::reset()
{
    num() = Pokemon::NoPoke;
    level() = 100;
    for (int i = 0; i < 4; i++)
        m_moves[i] = 0;
    happiness() = 0;
    shiny() = false;
    gender() = Pokemon::Neutral;
    ability() = 0;
    nickname() = "";
    nature() = 0;
    item() = 0;

    for (int i = 0; i < 6; i ++) {
        if (gen() >= 3) {
            m_EVs[i] = 0;
            m_DVs[i] = 31;
        }
        else {
            m_EVs[i] = 255;
            m_DVs[i] = 15;
        }
    }
}

PokeGraphics::PokeGraphics()
    : m_num(Pokemon::NoPoke), m_uptodate(false)
{
}

void PokeGraphics::setNum(Pokemon::uniqueId num)
{
    m_num = num;
    setUpToDate(false);
}

void PokeGraphics::setGen(Pokemon::gen gen)
{
    m_gen = gen;
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
    m_picture = PokemonInfo::Picture(num(), gen(), gender, shiny, false);

    setUpToDate(true);
}

void PokeGraphics::loadIcon(const Pokemon::uniqueId &pokeid)
{
    m_icon = PokemonInfo::Icon(pokeid);
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

QIcon PokeGraphics::icon(const Pokemon::uniqueId &pokeid)
{
    loadIcon(pokeid);
    return icon();
}

Pokemon::uniqueId PokeGraphics::num() const
{
    return m_num;
}

Pokemon::gen PokeGraphics::gen() const
{
    return m_gen;
}

PokeTeam::PokeTeam()
{
    setNum(Pokemon::uniqueId(Pokemon::NoPoke));
    setGen(Pokemon::gen(GenInfo::GenMax(), GenInfo::NumberOfSubgens(GenInfo::GenMax())-1));
}

void PokeTeam::setNum(Pokemon::uniqueId num)
{
    PokeGeneral::num() = num;
    PokePersonal::num() = num;
    PokeGraphics::setNum(num);
}

void PokeTeam::setGen(Pokemon::gen gen)
{
    PokeGeneral::gen() = gen;
    PokePersonal::gen() = gen;
    PokeGraphics::setGen(gen);
}

void PokeTeam::runCheck()
{
    Pokemon::uniqueId num = this->num();

    PokePersonal::runCheck();

    /* If the pokemon is reset to 0, we also make PokeGeneral and PokeGraphics reset */
    if (num != PokePersonal::num()) {
        setNum(PokePersonal::num());

        load();
    }
}

Pokemon::uniqueId PokeTeam::num() const
{
    return PokePersonal::num();
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
    else if (genderAvail() == Pokemon::MaleAvail)
    {
        gender() = Pokemon::Male;
    } else
    {
        if (gender() == Pokemon::Neutral) {
            /* Gen 2 has to do with IVs, so since we set max Att IV by default, it's male */
            gender() = gen() <= 2 ? Pokemon::Male : (true_rand() % 2 ? Pokemon::Male : Pokemon::Female);
        }
    }

    if (ability() == 0 || !abilities().contains(ability())) {
        ability() = abilities().ab(0);
    }

    PokeGraphics::load(gender(), false);
    PokeGraphics::loadIcon(num());
}

void PokeTeam::loadQuietly()
{
    PokeGeneral::load();
    /*set the default gender & ability */
    runCheck();
    PokeGraphics::load(gender(), false);
    PokeGraphics::loadIcon(num());
}

QPixmap PokeTeam::picture()
{
    return PokeGraphics::picture(gender(), shiny());
}

QIcon PokeTeam::icon()
{
    return PokeGraphics::icon();
}

int PokeTeam::stat(int statno) const
{
    return PokemonInfo::FullStat(num(), gen(), nature(), statno, level(),DV(statno),EV(statno));
}

Team::Team()
{
}

void Team::setGen(Pokemon::gen gen)
{
    if (this->gen() == gen)
        return;

    m_gen = gen;

    for (int i = 0; i < 6; i++) {
        poke(i).setGen(gen);
    }
}

QDomElement & PokeTeam::toXml(QDomElement &el) const
{
    const_cast<PokeTeam*>(this)->sanityCheck();

    QDomDocument doc;

    el.setAttribute("Nickname", nickname());
    el.setAttribute("Num", num().pokenum);
    el.setAttribute("Item", item());
    el.setAttribute("Ability", ability());
    el.setAttribute("Nature", nature());
    el.setAttribute("Gender", gender());
    el.setAttribute("Shiny", shiny());
    el.setAttribute("Happiness", happiness());
    el.setAttribute("Forme", num().subnum);
    el.setAttribute("Lvl", level());
    el.setAttribute("Gen", gen().num);
    el.setAttribute("SubGen", gen().subnum);

    for(int i = 0; i < 4; i++)
    {
        QDomElement move = doc.createElement("Move");
        el.appendChild(move);

        QDomText name = doc.createTextNode(QString("%1").arg(this->move(i)));
        move.appendChild(name);
    }
    for(int i = 0; i < 6; i++)
    {
        QDomElement Dv = doc.createElement("DV");
        el.appendChild(Dv);

        QDomText Dvname = doc.createTextNode(QString("%1").arg(DV(i)));
        Dv.appendChild(Dvname);
    }
    for(int i = 0; i < 6; i++)
    {
        QDomElement Ev = doc.createElement("EV");
        el.appendChild(Ev);

        QDomText Evname = doc.createTextNode(QString("%1").arg(EV(i)));
        Ev.appendChild(Evname);
    }

    return el;
}

void PokeTeam::sanityCheck()
{
    if (nickname().length() > 20) {
        nickname().resize(20);
    }
}

void Team::toXml(QDomDocument &document) const
{
    const_cast<Team*>(this)->sanityCheck();

    QDomElement Team = document.createElement("Team");
    Team.setAttribute("gen", gen().num);
    Team.setAttribute("subgen", gen().subnum);
    Team.setAttribute("defaultTier", defaultTier());
    Team.setAttribute("version", 1);
    document.appendChild(Team);

    for(int i = 0; i < 6; i++)
    {
        QDomElement pokemon = document.createElement("Pokemon");
        Team.appendChild(poke(i).toXml(pokemon));
    }
}

QString Team::toXml() const
{
    QDomDocument document;

    toXml(document);

    return document.toString();
}

bool Team::saveToFile(const QString &path) const
{
    m_path = path;

    QFile file(path);
    if(!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(0, QObject::tr("Error while saving the team"),QObject::tr("Can't create file ")+file.fileName());
        return false;
    }

    QDomDocument document;

    toXml(document);

    QTextStream in(&file);
    document.save(in,4);
    return true;
}

void saveTTeamDialog(const Team &team, QObject *receiver, const char *slot)
{
    QSettings s;
    QString defaultPath = s.value("Teams/Folder").toString();
    QFileDialog *f = new QFileDialog(NULL, QObject::tr("Saving the Team"),defaultPath, QObject::tr("Team(*.tp)"));
    //f->setWindowFlags(Qt::Window); //maybe the reason for crashes for some people
    f->setAttribute(Qt::WA_DeleteOnClose);
    f->setAcceptMode(QFileDialog::AcceptSave);
#if defined(Q_OS_MAC)
    f->setOption(QFileDialog::DontUseNativeDialog);
#endif
    f->show();

    TeamSaver *t = new TeamSaver(const_cast<Team*>(&team));
    t->setParent(f);

    QObject::connect(f, SIGNAL(fileSelected(QString)), t, SLOT(fileNameReceived(QString)));
    if (receiver)
        QObject::connect(f, SIGNAL(fileSelected(QString)), receiver, slot);
}

void loadTTeamDialog(Team &team, QObject *receiver, const char *slot)
{
    QSettings s;
    QString defaultPath = s.value("Teams/Folder").toString();
    QFileDialog *f = new QFileDialog(NULL, QObject::tr("Loading the Team"),defaultPath);
    //f->setWindowFlags(Qt::Window); //maybe the reason for crashes
    f->setAttribute(Qt::WA_DeleteOnClose);
    f->setAcceptMode(QFileDialog::AcceptOpen);
    f->setFileMode(QFileDialog::ExistingFile);
    f->show();

    TeamSaver *t = new TeamSaver(&team);
    t->setParent(f);

    QObject::connect(f, SIGNAL(fileSelected(QString)), t, SLOT(fileNameReceivedL(QString)));
    if (receiver)
        QObject::connect(f, SIGNAL(fileSelected(QString)), receiver, slot);
}

void PokeTeam::loadFromXml(const QDomElement &poke, int version)
{
    if (poke.hasAttribute("Gen")) {
        setGen(Pokemon::gen(poke.attribute("Gen", QString::number(GenInfo::GenMax())).toInt(),
                            poke.attribute("SubGen", QString::number(GenInfo::NumberOfSubgens(poke.attribute("Gen", QString::number(GenInfo::GenMax())).toInt())-1)).toInt()));
    }

    reset();

    /* Code to import old teams which had different formes registered as different pokemon numbers */
    int num = poke.attribute("Num").toInt();
    int forme = poke.attribute("Forme").toInt();

    if (gen().num == 4 && num > 493 && forme == 0 && !PokemonInfo::Exists(Pokemon::uniqueId(num, 0), 4)) {
        //Old way
        int indexes[] = {
            479,479,479,479,479,386,386,386,413,413,492,487
        };
        int formes[] = {
            1,2,3,4,5,1,2,3,1,2,1,1
        };

        int i = num - 494;

        setNum(Pokemon::uniqueId(indexes[i], formes[i]));
    } else {
        setNum(Pokemon::uniqueId(num, forme));
    }

    bool outdated = gen() < 5 && version < 1;

    load();
    nickname() = poke.attribute("Nickname");
    item() = poke.attribute("Item").toInt();
    ability() = poke.attribute("Ability").toInt();
    if (outdated) {
        ability() = AbilityInfo::ConvertFromOldAbility(ability());
    }
    nature() = poke.attribute("Nature").toInt();
    gender() = poke.attribute("Gender").toInt();
    shiny() = QVariant(poke.attribute("Shiny")).toBool();
    happiness() = poke.attribute("Happiness").toInt();
    level() = poke.attribute("Lvl").toInt();

    int cptMove=0;

    QDomElement moveElement = poke.firstChildElement("Move");
    while(!moveElement.isNull())
    {
        int movenum = moveElement.text().toInt();
        if (outdated) {
            movenum = MoveInfo::ConvertFromOldMove(movenum);
        }
        setMove(movenum,cptMove,false);
        cptMove++;
        moveElement = moveElement.nextSiblingElement("Move");
    }
    int cptDV=0;
    QDomElement DVElement = poke.firstChildElement("DV");
    while(!DVElement.isNull())
    {
        setDV(outdated ? NatureInfo::ConvertToStat(cptDV) : cptDV,DVElement.text().toInt());
        cptDV++;
        DVElement = DVElement.nextSiblingElement("DV");
    }
    int cptEV=0;
    QDomElement EVElement = poke.firstChildElement("EV");
    while(!EVElement.isNull())
    {
        setEV(outdated ? NatureInfo::ConvertToStat(cptEV) : cptEV,EVElement.text().toInt());
        cptEV++;
        EVElement = EVElement.nextSiblingElement("EV");
    }
}

Pokemon::gen PokeTeam::gen() const
{
    return PokePersonal::gen();
}

bool Team::loadFromFile(const QString &path)
{
    m_path = path;

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
    int version = team.attribute("version", "0").toInt();

    if (version > 1) {
        QMessageBox::information(0,QObject::tr("Load Team"),QObject::tr("Error while loading the team, the client is outdated."));
        return false;
    }

    int gen = team.attribute("gen", QString::number(GenInfo::GenMax())).toInt();
    if (gen < GEN_MIN || gen > GenInfo::GenMax())
        gen = GenInfo::GenMax();
    setGen(Pokemon::gen(team.attribute("gen", QString::number(GenInfo::GenMax())).toInt(),
                        team.attribute("subgen", QString::number(GenInfo::NumberOfSubgens(team.attribute("gen", QString::number(GenInfo::GenMax())).toInt())-1)).toInt()));
    defaultTier() = team.attribute("defaultTier");

    QDomElement poke = team.firstChildElement("Pokemon");
    int cpt = 0;
    while(!poke.isNull())
    {
        this->poke(cpt).loadFromXml(poke, version);

        cpt++;
        poke = poke.nextSiblingElement("Pokemon");
    }
    return true;
}

/******** Really ugly *********/
bool Team::importFromTxt(const QString &file1)
{
    QString file = file1;
    file.replace("---", "");
    file.replace("\r\n", "\n"); // for windows

    QStringList pokes = file.split("\n\n", QString::SkipEmptyParts);

    for (int i = 0; i < pokes.size() && i < 6; i++) {
        QStringList pokeDetail = pokes[i].split("\n", QString::SkipEmptyParts);

        if (pokeDetail.size() < 5) {
            continue;
        }

        QStringList first = pokeDetail[0].split('@');
        PokeTeam &p = this->poke(i);
        p.reset();

        Pokemon::uniqueId pokenum;
        QString nickname;
        int gender = 0;

        if (first[0].indexOf("(M)") > -1) {
            gender = Pokemon::Male;
            first[0].replace("(M)", "");
        } else if (first[0].indexOf("(F)") > -1) {
            gender = Pokemon::Female;
            first[0].replace("(F)", "");
        } else {
            gender = Pokemon::Neutral;
        }

        first[0] = first[0].trimmed();

        QString pokestring;
        if (first[0].contains('(')) {
            pokestring = first[0].section('(',1,1).section(')',0,0);
            nickname = first[0].mid(0, first[0].indexOf('(')).trimmed();
        } else {
            pokestring = first[0];
            nickname = first[0];
        }

        static QStringList bef = QStringList() << "Porygonz" << "Deoxys-f" << "Deoxys-e" << "Deoxys-l";
        static QStringList aft = QStringList() << "Porygon-Z" << "Deoxys-A" << "Deoxys-S" << "Deoxys-D";

        if (bef.contains(pokestring)) {
            pokestring = aft[bef.indexOf(pokestring)];
        }

        // alternate formes, from Shoddy :s
        if (pokestring.indexOf('-') != -1 && pokestring.indexOf('-') <= pokestring.length() - 2) {
            pokestring[pokestring.indexOf('-')+1] = pokestring[pokestring.indexOf('-')+1].toUpper();
        }

        pokenum = PokemonInfo::Number(pokestring);

        int item = 0;

        QString itemString;
        if (first.size() > 1) {
            if (first[1].contains("**")) {
                itemString = first[1].section("**",0,0).trimmed();
                nickname = first[1].section("**",1,1).trimmed();
            } else {
                itemString = first[1].trimmed();
            }
        }

        //Shoddy fix
        if (itemString == "Platinum Orb")
            itemString = "Griseous Orb";

        item = ItemInfo::Number(itemString);

        p.setNum(pokenum);
        p.load();
        p.gender() = gender;
        p.nickname() = nickname;
        p.item() = item;

        int movecount = 0;

        for (int i = 0; i < pokeDetail.count(); i++) {
            QString s = pokeDetail[i].trimmed();
            if (s.contains(":")) {
                QString key = s.section(':', 0, 0).trimmed();
                QString val = s.section(':', 1).trimmed();

                if (key == "Trait" || key == "Ability") {
                    int abnum = AbilityInfo::Number(val);
                    if (abnum != 0) {
                        p.ability() = abnum;
                    }
                } else if (key == "EVs" || key == "IVs") {
                    QStringList evList = pokeDetail[2].split(": ")[1].split("/");

                    foreach(QString ev, evList) {
                        QStringList ev2 = ev.trimmed().split(' ');
                        if (ev2.length() < 2)
                            break;
                        int evnum = ev2[0].toInt();
                        int stat = 0;

                        if (ev2[1] == "SDef" || ev2[1] == "SpDef"|| ev2[1] == "SpD")
                            stat = SpDefense;
                        else if (ev2[1] == "SAtk" || ev2[1] == "SpAtk" || ev2[1] == "SpA")
                            stat = SpAttack;
                        else if (ev2[1] == "Spd" || ev2[1] == "Spe")
                            stat = Speed;
                        else if (ev2[1] == "Def")
                            stat = Defense;
                        else if (ev2[1] == "Atk")
                            stat = Attack;
                        else
                            stat = Hp;

                        if (key == "EVs") {
                            p.setEV(stat, unsigned(evnum)%255);
                        } else {
                            p.setDV(stat, unsigned(evnum)%32);
                        }
                    }
                }
            } else if (s.contains(" Nature ")) {
                p.nature() = NatureInfo::Number(s.section(' ', 0, 0));
            } else if (s.startsWith("-")) {
                QString move = s.section('-',1).trimmed();

                if (move.contains('[')) {
                    int type = TypeInfo::Number(move.section('[',1,1).section(']',0,0));
                    if (type != 0) {
                        move = move.section('[',0,0).trimmed();

                        if (p.gen() >= 3) {
                            QStringList dvs = HiddenPowerInfo::PossibilitiesForType(type, p.gen())[0];
                            for(int i =0;i < dvs.size(); i++) {
                                p.setDV(i, dvs[i].toInt());
                            }
                        } else {
                            QPair<quint8,quint8> dvs = HiddenPowerInfo::AttDefDVsForGen2(type);

                            p.setDV(Attack, dvs.first);
                            p.setDV(Defense, dvs.second);
                        }
                    }
                }
                int moveNum = MoveInfo::Number(move);
                p.setMove(moveNum,movecount++,false);

                if (moveNum == Move::Return) {
                    p.happiness() = 255;
                }
            }
        }

        /* Removes invalid move combinations */
        p.runCheck();
    }
    return true;
/*
    Cheetos (Starmie) @ Life Orb
    Ability: Natural Cure
    EVs: 4 HP/252 Spd/252 SAtk
    Timid nature (+Spd, -Atk)
    - Surf
    - Ice Beam
    - Rapid Spin
    - Hidden Power [Ice]

    Venusaur (M) @ (No Item) ** Venyy
    Trait: Overgrow
    EVs: 85 HP / 85 Atk / 85 Def / 85 Spd / 85 SAtk / 85 SDef
    Hardy Nature (Neutral)
    - Bullet Seed

    Dugtrio (M) @ (No Item)
    Trait: Sand Veil
    EVs: 85 HP / 85 Atk / 85 Def / 85 Spd / 85 SAtk / 85 SDef
    Hardy Nature (Neutral)
    - Ancientpower

    Smeargle (M) @ (No Item)
    Trait: Own Tempo
    EVs: 85 HP / 85 Atk / 85 Def / 85 Spd / 85 SAtk / 85 SDef
    Hardy Nature (Neutral)
    - Acid Armor

    Lucario (M) @ (No Item)
    Trait: Steadfast
    EVs: 85 HP / 85 Atk / 85 Def / 85 Spd / 85 SAtk / 85 SDef
    Hardy Nature (Neutral)
    - Blaze Kick

    Zapdos @ (No Item)
    Trait: Pressure
    EVs: 85 HP / 85 Atk / 85 Def / 85 Spd / 85 SAtk / 85 SDef
    Hardy Nature (Neutral)
    - Charge

    Machamp (M) @ (No Item)
    Trait: Guts
    EVs: 85 HP / 85 Atk / 85 Def / 85 Spd / 85 SAtk / 85 SDef
    Hardy Nature (Neutral)
    - Bulk Up
*/
}

/******** Less ugly *********/
QString Team::exportToTxt() const
{
    QString ret = "";
    for (int i = 0; i < 6; i++) {
        if (this->poke(i).num() == Pokemon::NoPoke)
            continue;

        const PokeTeam &p = this->poke(i);

        ret += p.nickname();

        if (p.nickname() != PokemonInfo::Name(p.num())) {
            ret += " (" + PokemonInfo::Name(p.num()) + ")";
        }

        if (p.gender() != Pokemon::Neutral) {
            ret += QString(" (") + (p.gender() == Pokemon::Male ? "M" : "F") + ")";
        }

        ret += " @ " + ItemInfo::Name(p.item()) + "\n";

        if (p.gen() >= 3) {
            ret += "Trait: " + AbilityInfo::Name(p.ability()) + "\n";

            ret += "EVs: ";

            QString stats[] = {"HP", "Atk", "Def", "SAtk", "SDef", "Spd"};

            bool started = false;
            for (int i = 0; i < 6; i++) {
                if (p.EV(i) != 0) {
                    if (started) {
                        ret += " / ";
                    }
                    started = true;

                    ret += QString ("%1 %2").arg(p.EV(i)).arg(stats[i]);
                }
            }

            ret += "\n";

            ret += NatureInfo::Name(p.nature()) + " Nature";

            int up = NatureInfo::StatBoosted(p.nature());

            if (up != 0) {
                int down = NatureInfo::StatHindered(p.nature());
                ret += " (+" + stats[up] + ", -" + stats[down] + ")";
            }
            ret += "\n";
        }

        for (int i = 0; i < 4; i++) {
            if (p.move(i) != 0) {
                ret += "- " + MoveInfo::Name(p.move(i)) ;
                if (p.move(i) == Move::HiddenPower) {
                    ret += " [" + TypeInfo::Name(HiddenPowerInfo::Type(p.gen().num, p.DV(0), p.DV(1), p.DV(2), p.DV(3), p.DV(4), p.DV(5))) + "]";
                }
                ret += "\n";
            }
        }

        ret += "\n";
    }
    return ret.trimmed();
}

QString Team::name() const
{
    QFileInfo info(path());

    return QUrl::fromPercentEncoding(info.baseName().toUtf8());
}

void Team::setName(const QString &name)
{
    m_path = folder() + "/" + QUrl::toPercentEncoding(name) + ".tp";
}

void Team::setFolder(const QString &folder)
{
    m_path = folder + "/" + QUrl::toPercentEncoding(name()) + ".tp";
}

void Team::sanityCheck()
{
    if (defaultTier().length() > 100) {
        defaultTier().resize(100);
    }
}

QString Team::folder() const
{
    QFileInfo info(path());

    return info.absolutePath();
}

DataStream & operator << (DataStream & out, const Team & team)
{
    VersionControl v;
    Flags network(!team.defaultTier().isEmpty());

    v.stream << network;

    if (team.defaultTier().length() > 0) {
        v.stream << team.defaultTier();
    }

    v.stream << team.gen();

    for (int i = 0; i < 6; i++) {
        v.stream << team.poke(i);
    }

    out << v;

    return out;
}


DataStream & operator << (DataStream & out, const PokePersonal & p)
{
    VersionControl v;
    Flags network;

    typedef PokePersonal pp;

    network.setFlag(pp::hasNickname, !p.nickname().isEmpty());
    network.setFlag(pp::hasHappiness, p.happiness() != 0);

    for (int i = Hp; i <= Speed; i++) {
        if (p.DV(i) != 31) {
            network.setFlag(pp::hasIVs, true);
            break;
        }
    }

    v.stream << network;
    v.stream << p.num();
    v.stream << p.level();

    Flags data;
    data.setFlag(pp::isShiny, p.shiny());

    v.stream << data;

    if (p.nickname().length() > 0) {
        v.stream << p.nickname();
    }

    if (p.gen() > 1) {
        v.stream << p.item();
        if (p.gen() > 2) {
            v.stream << p.ability();
            v.stream << p.nature();
        }
        v.stream << p.gender();
        if (p.gen() >= 2 && p.happiness() != 0) {
            v.stream << p.happiness();
        }
    }

    int i;
    for(i=0;i<4;i++)
    {
        v.stream << p.move(i);
    }
    for(i=0;i<6;i++)
    {
        v.stream << p.EV(i);
    }

    if (network[pp::hasIVs]) {
        for(i=0;i<6;i++)
        {
            v.stream << p.DV(i);
        }
    }

    out << v;

    return out;
}

DataStream & operator >> (DataStream & in, PokePersonal & p)
{
    typedef PokePersonal pp;

    VersionControl v;
    in >> v;

    if (v.versionNumber != 0) {
        return in;
    }

    Flags network, data;

    v.stream >> network;
    if (network[pp::hasGen]) {
        v.stream >> p.gen();
    }
    v.stream >> p.num() >> p.level() >> data;

    p.shiny() = data[pp::isShiny];

    if (network[pp::hasNickname]) {
        v.stream >> p.nickname();
    }

    if (network[pp::hasPokeball]) {
        quint16 ball;
        v.stream >> ball;
    }

    if (p.gen() > 1) {
        v.stream >> p.item();
        if (p.gen() > 2) {
            v.stream >> p.ability();
            v.stream >> p.nature();
        }
        v.stream >> p.gender();
        if (p.gen() >= 2 && network[pp::hasHappiness]) {
            v.stream >> p.happiness();
        }
    }

    for (int i = 0; i < 4; i++) {
        if (network[pp::hasPPups]) {
            quint8 ppup;
            v.stream >> ppup;
        }
        quint16 moveNum;
        v.stream >> moveNum;
        p.setMove(moveNum,i);
    }

    for(int i=0;i<6;i++)
    {
        quint8 EV;
        v.stream >> EV;
        p.setEV(i,EV);
    }

    if (network[pp::hasIVs]) {
        for(int i=0;i<6;i++)
        {
            quint8 IV;
            v.stream >> IV;
            p.setDV(i,IV);
        }
    }

    return in;
}
