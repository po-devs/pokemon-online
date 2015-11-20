#include "damagecalc.h"
#include "ui_damagecalc.h"

#include <QMessageBox>

#define deb(msg) QMessageBox::information(0,"debug",msg)

DamageCalc::DamageCalc(int gen, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DamageCalc)
{
    ui->setupUi(this);

    whash.insert(Weather::NormalWeather, "Clear Skies");
    whash.insert(Weather::Sunny, "Sun");
    whash.insert(Weather::Rain, "Rain");
    whash.insert(Weather::SandStorm, "Sandstorm");
    whash.insert(Weather::Hail, "Hail");
    whash.insert(8, "Fog");
    whash.insert(Weather::StrongSun, "Strong Sun");
    whash.insert(Weather::StrongRain, "Strong Rain");
    whash.insert(Weather::StrongWinds, "Strong Winds");

    ui->mypokes->hide();
    ui->opokes->hide();

    // eventually get team info and allow user to click on pokebutton to quickly input their info

    gen = qMin(GenInfo::GenMax(), qMax(GenInfo::GenMin(), gen));

    for (int i = 0; i < GenInfo::NumberOfGens(); i++) {
        ui->gen->addItem(GenInfo::Gen(i + 1));
    }

    setupCalc(gen);
    ui->gen->setCurrentIndex(gen - 1);

    connect(ui->gen, SIGNAL(currentIndexChanged(int)), this, SLOT(updateGen()));
    connect(ui->move, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMoveInfo()));

    QList<QSpinBox *> mine;
    mine << ui->myhpev << ui->myhpiv << ui->myhppercent
            << ui->myatkev << ui->myatkiv << ui->myatkboost
            << ui->mydefev << ui->mydefiv << ui->mydefboost
            << ui->myspatkev << ui->myspatkiv << ui->myspatkboost
            << ui->myspdefev << ui->myspdefiv << ui->myspdefboost
            << ui->myspdev << ui->myspdiv << ui->myspdboost;

    foreach (QSpinBox *w, mine) {
        connect(w, SIGNAL(valueChanged(int)), this, SLOT(updateMyPokeStats()));
    }

    QList<QSpinBox *> opps;
    opps << ui->ohpev << ui->ohpiv << ui->ohppercent
            << ui->oatkev << ui->oatkiv << ui->oatkboost
            << ui->odefev << ui->odefiv << ui->odefboost
            << ui->ospatkev << ui->ospatkiv << ui->ospatkboost
            << ui->ospdefev << ui->ospdefiv << ui->ospdefboost
            << ui->ospdev << ui->ospdiv << ui->ospdboost;

    foreach (QSpinBox *w, opps) {
        connect(w, SIGNAL(valueChanged(int)), this, SLOT(updateOPokeStats()));
    }

    connect(ui->mypoke, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMyPoke()));
    connect(ui->opoke, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOPoke()));

    connect(ui->mynature, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMyPokeStats()));
    connect(ui->onature, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOPokeStats()));

    connect(ui->mylevel, SIGNAL(valueChanged(int)), this, SLOT(updateMyPokeStats()));
    connect(ui->olevel, SIGNAL(valueChanged(int)), this, SLOT(updateOPokeStats()));

    connect(ui->mygender, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMyPokePic()));
    connect(ui->ogender, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOPokePic()));

    connect(ui->calculate, SIGNAL(clicked()), this, SLOT(calculate()));
}

DamageCalc::~DamageCalc()
{
    delete ui;
}

void DamageCalc::setupCalc(int gen)
{
    Pokemon::gen pgen = Pokemon::gen(gen, GenInfo::NumberOfSubgens(gen) - 1);
    m_currentGen = pgen;

    QString move = ui->move->currentText();
    QString weather = ui->weather->currentText();

    QString mypoke = ui->mypoke->currentText();
    QString myability = ui->myability->currentText();
    QString myitem = ui->myitem->currentText();
    QString mynature = ui->mynature->currentText();
    QString mystatus = ui->mystatus->currentText();
    QString mygender = ui->mygender->currentText();

    QString opoke = ui->opoke->currentText();
    QString oability = ui->oability->currentText();
    QString oitem = ui->oitem->currentText();
    QString onature = ui->onature->currentText();
    QString ostatus = ui->ostatus->currentText();
    QString ogender = ui->ogender->currentText();

    QStringList moveNames = MoveInfo::Names(pgen);
    moveNames.sort();

    ui->move->clear();
    ui->move->addItems(moveNames);

    ui->movetype->clear();
    ui->movetype->addItems(TypeInfo::Names(pgen));

    ui->movecategory->clear();
    ui->movecategory->addItems(CategoryInfo::Names());

    ui->weather->clear();

    ui->weather->addItem("Clear Skies", QVariant(Weather::NormalWeather));

    if (gen >= 2) {
        ui->weather->addItem(whash.value(Weather::Sunny));
        ui->weather->addItem(whash.value(Weather::Rain));
        ui->weather->addItem(whash.value(Weather::SandStorm));
    }
    if (gen >= 3) {
        ui->weather->addItem(whash.value(Weather::Hail));
    }
    if (gen == 4) {
        ui->weather->addItem(whash.value(8));
    }
    if (gen >= 6) {
        ui->weather->addItem(whash.value(Weather::StrongSun));
        ui->weather->addItem(whash.value(Weather::StrongRain));
        ui->weather->addItem(whash.value(Weather::StrongWinds));
    }

    ui->mypoke->clear();
    ui->myability->clear();
    ui->myitem->clear();
    ui->mynature->clear();
    ui->mystatus->clear();
    ui->mygender->clear();

    ui->opoke->clear();
    ui->oability->clear();
    ui->oitem->clear();
    ui->onature->clear();
    ui->ostatus->clear();
    ui->ogender->clear();

    QStringList pokeNames = PokemonInfo::Names(pgen, true);

    ui->mypoke->addItems(pokeNames);
    ui->opoke->addItems(pokeNames);

    QStringList abilityNames = AbilityInfo::Names(pgen);
    abilityNames.sort();

    ui->myability->addItems(abilityNames);
    ui->oability->addItems(abilityNames);

    ui->myitem->addItems(ItemInfo::SortedUsefulNames(pgen));
    ui->oitem->addItems(ItemInfo::SortedUsefulNames(pgen));

    ui->mynature->addItems(NatureInfo::Names(pgen));
    ui->onature->addItems(NatureInfo::Names(pgen));

    ui->mystatus->addItems(StatInfo::StatusNames());
    ui->ostatus->addItems(StatInfo::StatusNames());

    int max = (gen >= 3 ? 31 : 15);

    ui->myhpiv->setMaximum(max);
    ui->myatkiv->setMaximum(max);
    ui->mydefiv->setMaximum(max);
    ui->myspatkiv->setMaximum(max);
    ui->myspdefiv->setMaximum(max);
    ui->myspdiv->setMaximum(max);

    ui->ohpiv->setMaximum(max);
    ui->oatkiv->setMaximum(max);
    ui->odefiv->setMaximum(max);
    ui->ospatkiv->setMaximum(max);
    ui->ospdefiv->setMaximum(max);
    ui->ospdiv->setMaximum(max);

    ui->myhpiv->setValue(max);
    ui->myatkiv->setValue(max);
    ui->mydefiv->setValue(max);
    ui->myspatkiv->setValue(max);
    ui->myspdefiv->setValue(max);
    ui->myspdiv->setValue(max);

    ui->ohpiv->setValue(max);
    ui->oatkiv->setValue(max);
    ui->odefiv->setValue(max);
    ui->ospatkiv->setValue(max);
    ui->ospdefiv->setValue(max);
    ui->ospdiv->setValue(max);

    setText(ui->move, move);

    setText(ui->mypoke, mypoke);
    setText(ui->myability, myability);
    setText(ui->myitem, myitem);
    setText(ui->mynature, mynature);
    setText(ui->mystatus, mystatus);
    setText(ui->mygender, mygender);

    setText(ui->opoke, opoke);
    setText(ui->oability, oability);
    setText(ui->oitem, oitem);
    setText(ui->onature, onature);
    setText(ui->ostatus, ostatus);
    setText(ui->ogender, ogender);

    setText(ui->weather, weather);

    ui->weather->setVisible(gen > 1);
    ui->weatherLabel->setVisible(gen > 1);

    ui->myitemlabel->setVisible(gen > 1);
    ui->myitem->setVisible(gen > 1);
    ui->myabilitylabel->setVisible(gen > 2);
    ui->myability->setVisible(gen > 2);
    ui->mynaturelabel->setVisible(gen > 2);
    ui->mynature->setVisible(gen > 2);
    ui->mygenderlabel->setVisible(gen > 2);
    ui->mygender->setVisible(gen > 2);

    ui->oitemlabel->setVisible(gen > 1);
    ui->oitem->setVisible(gen > 1);
    ui->oabilitylabel->setVisible(gen > 2);
    ui->oability->setVisible(gen > 2);
    ui->onaturelabel->setVisible(gen > 2);
    ui->onature->setVisible(gen > 2);
    ui->ogenderlabel->setVisible(gen > 2);
    ui->ogender->setVisible(gen > 2);

    ui->watersport->setVisible(gen > 3);
    ui->mudsport->setVisible(gen > 3);
    ui->fusionused->setVisible(gen > 4);
    ui->mefirst->setVisible(gen > 3);
    ui->charged->setVisible(gen > 2);
    ui->helpinghand->setVisible(gen > 2);
    ui->myflowergift->setVisible(gen > 3);
    ui->oflowergift->setVisible(gen > 3);
    ui->friendguard->setVisible(gen > 4);
    ui ->wonderroom->setVisible(gen > 4);
    ui->allyfainted->setVisible(gen > 4);

    updateMyPoke();
    updateOPoke();
    updateMoveInfo();
}

void DamageCalc::updateGen()
{
    int gen = ui->gen->currentIndex() + 1;
    setupCalc(gen);
}

void DamageCalc::updateMyPokeStats()
{
    int n = NatureInfo::Number(ui->mynature->currentText());

    int hp = PokemonInfo::FullStat(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, n, 0, ui->mylevel->value(), ui->myhpiv->value(), ui->myhpev->value());
    int atk = PokemonInfo::FullStat(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, n, 1, ui->mylevel->value(), ui->myatkiv->value(), ui->myatkev->value());
    int def = PokemonInfo::FullStat(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, n, 2, ui->mylevel->value(), ui->mydefiv->value(), ui->mydefev->value());
    int spatk = PokemonInfo::FullStat(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, n, 3, ui->mylevel->value(), ui->myspatkiv->value(), ui->myspatkev->value());
    int spdef = PokemonInfo::FullStat(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, n, 4, ui->mylevel->value(), ui->myspdefiv->value(), ui->myspdefev->value());
    int spd = PokemonInfo::FullStat(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, n, 5, ui->mylevel->value(), ui->myspdiv->value(), ui->myspdev->value());

    hp = hp * ui->myhppercent->value()/100;
    atk = PokemonInfo::BoostedStat(atk, ui->myatkboost->value());
    def = PokemonInfo::BoostedStat(def, ui->mydefboost->value());
    spatk = PokemonInfo::BoostedStat(spatk, ui->myspatkboost->value());
    spdef = PokemonInfo::BoostedStat(spdef, ui->myspdefboost->value());
    spd = PokemonInfo::BoostedStat(spd, ui->myspdboost->value());

    ui->myhpstat->setText(QString::number(hp));
    ui->myatkstat->setText(QString::number(atk));
    ui->mydefstat->setText(QString::number(def));
    ui->myspatkstat->setText(QString::number(spatk));
    ui->myspdefstat->setText(QString::number(spdef));
    ui->myspdstat->setText(QString::number(spd));
}

void DamageCalc::updateOPokeStats()
{
    int n = NatureInfo::Number(ui->onature->currentText());

    int hp = PokemonInfo::FullStat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, n, 0, ui->olevel->value(), ui->ohpiv->value(), ui->ohpev->value());
    int atk = PokemonInfo::FullStat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, n, 1, ui->olevel->value(), ui->oatkiv->value(), ui->oatkev->value());
    int def = PokemonInfo::FullStat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, n, 2, ui->olevel->value(), ui->odefiv->value(), ui->odefev->value());
    int spatk = PokemonInfo::FullStat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, n, 3, ui->olevel->value(), ui->ospatkiv->value(), ui->ospatkev->value());
    int spdef = PokemonInfo::FullStat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, n, 4, ui->olevel->value(), ui->ospdefiv->value(), ui->ospdefev->value());
    int spd = PokemonInfo::FullStat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, n, 5, ui->olevel->value(), ui->ospdiv->value(), ui->ospdev->value());

    hp = hp * ui->ohppercent->value()/100;
    atk = PokemonInfo::BoostedStat(atk, ui->oatkboost->value());
    def = PokemonInfo::BoostedStat(def, ui->odefboost->value());
    spatk = PokemonInfo::BoostedStat(spatk, ui->ospatkboost->value());
    spdef = PokemonInfo::BoostedStat(spdef, ui->ospdefboost->value());
    spd = PokemonInfo::BoostedStat(spd, ui->ospdboost->value());

    ui->ohpstat->setText(QString::number(hp));
    ui->oatkstat->setText(QString::number(atk));
    ui->odefstat->setText(QString::number(def));
    ui->ospatkstat->setText(QString::number(spatk));
    ui->ospdefstat->setText(QString::number(spdef));
    ui->ospdstat->setText(QString::number(spd));
}

void DamageCalc::updateMoveInfo()
{
    int move = MoveInfo::Number(ui->move->currentText());
    int bp = MoveInfo::Power(move, m_currentGen);
    int type = MoveInfo::Type(move, m_currentGen);
    int category = MoveInfo::Category(move, m_currentGen);

    ui->movepower->setValue(bp);
    ui->movetype->setCurrentIndex(type);
    ui->movecategory->setCurrentIndex(category);
}

void DamageCalc::updateMyPokePic()
{
    QPixmap p = PokemonInfo::Picture(PokemonInfo::Number(ui->mypoke->currentText()), m_currentGen, GenderInfo::Number(ui->mygender->currentText()));
    //ui->mypokepic->setMinimumSize(qMax(64, p.width()), qMax(64, p.height()));
    ui->mypokepic->setPixmap(p);
}

void DamageCalc::updateOPokePic()
{
    QPixmap p = PokemonInfo::Picture(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, GenderInfo::Number(ui->ogender->currentText()));
    //ui->opokepic->setMinimumSize(qMax(64, p.width()), qMax(64, p.height()));
    ui->opokepic->setPixmap(p);
}

void DamageCalc::updateMyPokeInfo()
{
    ui->mygender->clear();

    int avail = PokemonInfo::Gender(PokemonInfo::Number(ui->mypoke->currentText()));

    if (avail == Pokemon::MaleAvail) {
        ui->mygender->addItem(tr("Male"));
    } else if (avail == Pokemon::FemaleAvail) {
        ui->mygender->addItem(tr("Female"));
    } else if (avail == Pokemon::NeutralAvail) {
        ui->mygender->addItem(tr("Neutral"));
    } else { // male and female
        ui->mygender->addItem(tr("Male"));
        ui->mygender->addItem(tr("Female"));
    }

}

void DamageCalc::updateOPokeInfo()
{
    ui->ogender->clear();

    int avail = PokemonInfo::Gender(PokemonInfo::Number(ui->opoke->currentText()));

    if (avail == Pokemon::MaleAvail) {
        ui->ogender->addItem("Male");
    } else if (avail == Pokemon::FemaleAvail) {
        ui->ogender->addItem("Female");
    } else if (avail == Pokemon::NeutralAvail) {
        ui->ogender->addItem("Neutral");
    } else { // male and female
        ui->ogender->addItem("Male");
        ui->ogender->addItem("Female");
    }
}

void DamageCalc::updateMyPoke()
{
    updateMyPokePic();
    updateMyPokeInfo();
    updateMyPokeStats();
}

void DamageCalc::updateOPoke()
{
    updateOPokePic();
    updateOPokeInfo();
    updateOPokeStats();
}

void DamageCalc::calculate()
{
    // INFO // http://www.smogon.com/bw/articles/bw_complete_damage_formula

    int move = MoveInfo::Number(ui->move->currentText());
    int bp = ui->movepower->value();
    int type = TypeInfo::Number(ui->movetype->currentText());
    int category = ui->movecategory->currentIndex();
    bool crit = ui->crit->isChecked();
    int gen = m_currentGen.num;

    int weather = whash.key(ui->weather->currentText());

    if (category == Move::Other) {
        showResult(0, 0);
        return;
    }

    Pokemon::uniqueId mypoke = PokemonInfo::Number(ui->mypoke->currentText());
    Pokemon::uniqueId opoke = PokemonInfo::Number(ui->opoke->currentText());

    int mylevel = ui->mylevel->value();
    int olevel = ui->olevel->value();
    int mynature = ui->mynature->currentIndex();
    int onature = ui->onature->currentIndex();
    int myitem = ItemInfo::Number(ui->myitem->currentText());
    int oitem = ItemInfo::Number(ui->oitem->currentText());
    int myability = AbilityInfo::Number(ui->myability->currentText());
    int oability = AbilityInfo::Number(ui->oability->currentText());
    int mytype1 = PokemonInfo::Type1(mypoke, m_currentGen);
    int mytype2 = PokemonInfo::Type2(mypoke, m_currentGen);
    int otype1 = PokemonInfo::Type1(opoke, m_currentGen);
    int otype2 = PokemonInfo::Type2(opoke, m_currentGen);
    int mystatus = StatInfo::StatusNumber(ui->mystatus->currentText());
    int ostatus = StatInfo::StatusNumber(ui->ostatus->currentText());
    int mygender = GenderInfo::Number(ui->mygender->currentText());
    int ogender = GenderInfo::Number(ui->ogender->currentText());
    int mymaxhp = PokemonInfo::Stat(mypoke, m_currentGen, 0, mylevel, ui->myhpiv->value(), ui->myhpev->value());
    int omaxhp = PokemonInfo::Stat(opoke, m_currentGen, 0, olevel, ui->ohpiv->value(), ui->ohpev->value());

    QList<int> mystats;
    QList<int> ostats;
    QList<int> myboosts;
    QList<int> oboosts;

    QStringList statNames = { "hp", "atk", "def", "spatk", "spdef", "spd" };

    for (int i = 0; i < 6; i++) {
        QSpinBox *myev = ui->myinfo->findChild<QSpinBox *>("my" + statNames[i] + "ev");
        QSpinBox *myiv = ui->myinfo->findChild<QSpinBox *>("my" + statNames[i] + "iv");

        int mystat = PokemonInfo::FullStat(mypoke, m_currentGen, mynature, i, mylevel, myiv->value(), myev->value());

        if (i > 0) {
            QSpinBox *myboost = ui->myinfo->findChild<QSpinBox *>("my" + statNames[i] + "boost");
            myboosts.append(myboost->value());
        } else if (i == 0) {
            mystat = mystat * ui->myhppercent->value() / 100;
            oboosts.append(0);
        }

        mystats.append(mystat);

        QSpinBox *oev = ui->oinfo->findChild<QSpinBox *>("o" + statNames[i] + "ev");
        QSpinBox *oiv = ui->oinfo->findChild<QSpinBox *>("o" + statNames[i] + "iv");

        int ostat = PokemonInfo::FullStat(opoke, m_currentGen, onature, i, olevel, oiv->value(), oev->value());

        if (i > 0) {
            QSpinBox *oboost = ui->oinfo->findChild<QSpinBox *>("o" + statNames[i] + "boost");
            oboosts.append(oboost->value());
        } else if (i == 0) {
            ostat = ostat * ui->ohppercent->value() / 100;
            oboosts.append(0);
        }

        ostats.append(ostat);
    }

    mystats[5] = PokemonInfo::BoostedStat(mystats[5], myboosts[5]);
    ostats[5] = PokemonInfo::BoostedStat(ostats[5], oboosts[5]);

    // CALC //
    // 0    0x0000
    // 0.33 0x0548
    // 0.5  0x0800
    // 0.75 0x0C00
    // 1    0x1000
    // 1.2  0x1333
    // 1.3  0x14CD
    // 1.33 0x1555
    // 1.5  0x1800
    // 2    0x2000

    // -2. ATTACK STATS //
    int amod = 0x1000;
    // -2A. FOUL PLAY //
    if (move == Move::FoulPlay) {
        mystats[1] = ostats[1];
    }
    // -2B. BOOSTS //
    bool unaware = (oability == Ability::Unaware && myability != Ability::MoldBreaker);
    if (!unaware) {
        if (myboosts[1] < 0 && crit) {
            myboosts[1] = 0;
        }
        if (myboosts[3] < 0 && crit) {
            myboosts[3] = 0;
        }

        mystats[1] = PokemonInfo::BoostedStat(mystats[1], myboosts[1]);
        mystats[3] = PokemonInfo::BoostedStat(mystats[3], myboosts[3]);
    }

    // -2C. THICK FAT //
    if ((type == Type::Ice || type == Type::Fire) && oability == Ability::ThickFat) {
        amod = chainmod(amod, 0x800);
    }
    // -2D. TORRENT, ETC //
    if (mystats[0] < (mymaxhp / 3) && ((myability == Ability::Torrent && type == Type::Water) || (myability == Ability::Blaze && type == Type::Fire)
                                       || (myability == Ability::Overgrow && type == Type::Grass) || (myability == Ability::Swarm && type == Type::Bug))) {
        amod = chainmod(amod, 0x1800);
    }
    // -2E. GUTS //
    if (myability == Ability::Guts && category == Move::Physical && mystatus != Pokemon::Fine) {
        amod = chainmod(amod, 0x1800);
    }
    // -2F. PLUS/MINUS //
    if (myability == Ability::Plus || myability == Ability::Minus) {
        amod = chainmod(amod, 0x1800);
    }
    // -2G. DEFEATIST //
    if (myability == Ability::Defeatist && mystats[0] < (mymaxhp / 2)) {
        amod = chainmod(amod, 0x800);
    }
    // -2H. LARGE POWER //
    if ((myability == Ability::PurePower || myability == Ability::HugePower) && category == Move::Physical) {
        amod = chainmod(amod, 0x2000);
    }
    // -2I. SOLAR POWER //
    if (myability == Ability::SolarPower && weather == Weather::Sunny && category == Move::Special) {
        amod = chainmod(amod, 0x1800);
    }
    // -2J. HUSTLE // NOTE: APPLIES DIRECTLY TO STAT
    if (myability == Ability::Hustle && category == Move::Physical) {
       mystats[1] = applymod(mystats[1], 0x1800);
    }
    // -2K. FLASH FIRE //
    if (myability == Ability::FlashFire && type == Type::Fire) {
        amod = chainmod(amod, 0x1800);
    }
    // -2L. SLOW START //
    if (myability == Ability::SlowStart) {
        amod = chainmod(amod, 0x800);
    }
    // -2M. FLOWER GIFT //
    if (ui->myflowergift->isChecked() && weather == Weather::Sunny && category == Move::Physical) {
        amod = chainmod(amod, 0x1800);
    }
    // -2N. THICK CLUB //
    if ((mypoke == Pokemon::Cubone || mypoke == Pokemon::Marowak) && myitem == Item::ThickClub && category == Move::Physical) {
        amod = chainmod(amod, 0x2000);
    }
    // -2O. DEEPSEATOOTH //
    if (mypoke == Pokemon::Clamperl && myitem == Item::DeepSeaTooth && category == Move::Special) {
        amod = chainmod(amod, 0x2000);
    }
    // -2P. LIGHT BALL //
    if (mypoke == Pokemon::Pikachu && myitem == Item::LightBall) {
        amod = chainmod(amod, 0x2000);
    }
    // -2Q. SOUL DEW //
    if ((mypoke == Pokemon::Latios || mypoke == Pokemon::Latias) && myitem == Item::SoulDew) {
        amod = chainmod(amod, 0x1800);
    }
    // -2R. CHOICE ITEMS //
    if ((myitem == Item::ChoiceBand && category == Move::Physical) || (myitem == Item::ChoiceSpecs && category == Move::Special)) {
        amod = chainmod(amod, 0x1800);
    }

    int attack = (category == Move::Physical ? mystats[1] : mystats[3]);
    attack = applymod(amod, attack);

    // -1. DEFENSE STAT //
    int dmod = 0x1000;
    // -1A. BOOSTS //
    unaware = (myability == Ability::Unaware);
    if (!unaware && move != Move::ChipAway) {
        if (oboosts[2] > 0 && crit) {
            oboosts[2] = 0;
        }
        if (oboosts[4] > 0 && crit) {
            oboosts[4] = 0;
        }

        ostats[2] = PokemonInfo::BoostedStat(ostats[2], oboosts[2]);
        ostats[4] = PokemonInfo::BoostedStat(ostats[4], oboosts[4]);
    }

    bool wonderroom = ui->wonderroom->isChecked();
    int defense = (category == Move::Special ? ((move == Move::Psyshock || move == Move::Psystrike || move == Move::SecretSword)
                                                ? (wonderroom ? ostats[4] : ostats[2]) : (wonderroom ? ostats[2] : ostats[4])) : (wonderroom ? ostats[4] : ostats[2]));
    bool usingspecial = (category == Move::Special || move == Move::Psyshock || move == Move::Psystrike || move == Move::SecretSword);

    // -1B. SANDSTORM //
    if (gen >= 4 && usingspecial && weather == Weather::SandStorm && (otype1 == Type::Rock || otype2 == Type::Rock)) {
        defense = applymod(defense, 0x1800);
    }
    // -1C. MARVEL SCALE //
    if (oability == Ability::MarvelScale && ostatus != Pokemon::Fine) {
        dmod = chainmod(dmod, 0x1800);
    }
    // -1D. FLOWER GIFT //
    if (ui->oflowergift->isChecked() && weather == Weather::Sunny && usingspecial) {
        dmod = chainmod(dmod, 0x1800);
    }
    // -1E. DEEPSEASCALE //
    if (opoke == Pokemon::Clamperl && oitem == Item::DeepSeaScale && usingspecial) {
        dmod = chainmod(dmod, 0x1800);
    }
    // -1F. METAL POWDER //
    if (opoke == Pokemon::Ditto && oitem == Item::MetalPowder && !usingspecial) {
        dmod = chainmod(dmod, 0x2000);
    }
    // -1G. EVIOLITE //
    if (PokemonInfo::HasEvolutions(opoke.pokenum) && oitem == Item::Eviolite) {
        dmod = chainmod(dmod, 0x1800);
    }
    // -1H. SOUL DEW //
    if ((opoke == Pokemon::Latios || opoke == Pokemon::Latias) && oitem == Item::SoulDew && usingspecial) {
        dmod = chainmod(dmod, 0x1800);
    }

    defense = applymod(defense, dmod);

    // 0. BASE //
    int bmod = 0x1000;

    // 0A. TECHNICIAN //
    if (myability == Ability::Technician && bp <= 60) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0B. FLARE BOOST //
    if (myability == Ability::FlareBoost && category == Move::Special) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0C. ANALYTIC // 0x14CD
    // 0D. RECKLESS //
    if (myability == Ability::Reckless && (MoveInfo::Recoil(move, m_currentGen) > 0 || move == Move::JumpKick || move == Move::HiJumpKick)) {
        bmod = chainmod(bmod, 0x1333);
    }
    // 0E. IRON FIST //
    if (myability == Ability::IronFist && MoveInfo::Flags(move, m_currentGen) & Move::PunchFlag) {
        bmod = chainmod(bmod, 0x1333);
    }
    // 0F. TOXIC BOOST //
    if (myability == Ability::ToxicBoost && category == Move::Physical) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0G. RIVALRY //
    if (myability == Ability::Rivalry) {
        int rmod = 0x1000;

        if (mygender == Pokemon::Neutral || ogender == Pokemon::Neutral) {
            rmod = 0x1000;
        } else if (mygender == ogender) {
            rmod = 0x1400;
        } else {
            rmod = 0xC00;
        }

        bmod = chainmod(bmod, rmod);
    }
    // 0H. SAND FORCE //
    if (myability == Ability::SandForce && (type == Type::Rock || type == Type::Ground || type == Type::Steel) && weather == Weather::SandStorm) {
        bmod = chainmod(bmod, 0x14CD);
    }
    // 0I. HEATPROOF //
    if (oability == Ability::Heatproof && type == Type::Fire) {
        bmod = chainmod(bmod, 0x800);
    }
    // 0J. DRY SKIN //
    if (oability == Ability::DrySkin) {
        if (type == Type::Fire) {
            bmod = chainmod(bmod, 0x1400);
        } else if (type == Type::Water) {
            bmod = chainmod(bmod, 0x0);
        }
    }
    // 0K. SHEER FORCE //
    /* if (cl != Move::OffensiveStatChangingMove && cl != Move::OffensiveStatusInducingMove && tmove(b,s).flinchRate == 0
            && (cl != Move::OffensiveSelfStatChangingMove || ((signed char)(tmove(b,s).boostOfStat >> 16)) < 0)) */
    int classification = MoveInfo::Classification(move, m_currentGen);
    if (myability == Ability::SheerForce
            && (classification == Move::OffensiveStatChangingMove || classification == Move::OffensiveStatusInducingMove || MoveInfo::FlinchRate(move, m_currentGen) > 0
                || classification == Move::OffensiveSelfStatChangingMove || ((signed char)MoveInfo::BoostOfStat(move, m_currentGen) << 16) >= 0)) {
        bmod = chainmod(bmod, 0x14CD);
    }
    // 0L. TYPE-BOOSTING ITEMS // 0x1333
    if (ItemInfo::PlateType(myitem) == type) {
        bmod = chainmod(bmod, 0x1333);
    }
    // 0M. MUSCLE BAND, ETC //
    if ((myitem == Item::MuscleBand && category == Move::Physical) || (myitem == Item::WiseGlasses && category == Move::Special)) {
        bmod = chainmod(bmod, 0x1199);
    }
    // 0N. GEN 4 ORBS //
    if ((mypoke == Pokemon::Palkia && myitem == Item::LustrousOrb && (type == Type::Water || type == Type::Dragon))
            || (mypoke == Pokemon::Dialga && myitem == Item::AdamantOrb && (type == Type::Steel || type == Type::Dragon))
            || (mypoke == Pokemon::Giratina_O && myitem == Item::GriseousOrb && (type == Type::Ghost || type == Type::Dragon))) {
        bmod = chainmod(bmod, 0x1333);
    }
    // 0O. GEMS // ARE TRULY OUTRAGEOUS
    if (false) {
        if (gen < 6) {
            bmod = chainmod(bmod, 0x1800);
        } else {
            bmod = chainmod(bmod, 0x14CD);
        }
    }
    // 0P. FACADE //
    if (move == Move::Facade && (mystatus == Pokemon::Burnt || mystatus == Pokemon::Paralysed || mystatus == Pokemon::Poisoned)) {
        bmod = chainmod(bmod, 0x2000);
    }
    // 0Q. BRINE //
    if (move == Move::Brine && ostats[0] <= (omaxhp / 2)) {
        bmod = chainmod(bmod, 0x2000);
    }
    // 0R. VENOSHOCK //
    if (move == Move::Venoshock && ostatus == Pokemon::Poisoned) {
        bmod = chainmod(bmod, 0x2000);
    }
    // 0S. RETALIATE //
    if (move == Move::Retaliate && ui->allyfainted->isChecked()) {
        bmod = chainmod(bmod, 0x2000);
    }
    // 0T. FUSION MOVES //
    if ((move == Move::FusionBolt || move == Move::FusionFlare) && ui->fusionused->isChecked()) {
        bmod = chainmod(bmod, 0x2000);
    }
    // 0U. ME FIRST //
    if (ui->mefirst->isChecked()) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0V. SOLARBEAR // i dont want to fix this solarbear is cool
    if (move == Move::SolarBeam && (weather != Weather::Sunny && weather != Weather::NormalWeather)) {
        bmod = chainmod(bmod, 0x800);
    }
    // 0W. CHARGE //
    if (type == Type::Electric && ui->charged->isChecked()) {
        bmod = chainmod(bmod, 0x2000);
    }
    // 0X. HELPING HAND //
    if (ui->helpinghand->isChecked()) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0Y. SPORTS //
    if ((ui->watersport->isChecked() && type == Type::Fire) || (ui->mudsport->isChecked() && type == Type::Electric)) {
        bmod = chainmod(bmod, 0x548);
    }
    // 0Z. MEGA LAUNCHER //
    if (myability == Ability::MegaLauncher && MoveInfo::Flags(move, m_currentGen) & Move::LaunchFlag) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0AA. STRONG JAW //
    if (myability == Ability::StrongJaw && MoveInfo::Flags(move, m_currentGen) & Move::BiteFlag) {
        bmod = chainmod(bmod, 0x1800);
    }
    // 0AB. TOUGH CLAWS //
    if (myability == Ability::ToughClaws && MoveInfo::Flags(move, m_currentGen) & Move::ContactFlag) {
        bmod = chainmod(bmod, 0x1555);
    }

    bp = applymod(bp, bmod);
    int base = ((((2 * mylevel) / 5 + 2) * bp * attack) / defense) / 50 + 2;

    // 1. MULTITARGET //
    if (ui->multitarget->isChecked()) {
        base = applymod(base, 0xC00); // 0.75
    }

    // 2. WEATHER - (1.5, 0.5) //
    if ((type == Type::Fire && weather == Weather::Sunny) || (type == Type::Water && weather == Weather::Rain)) {
        base = applymod(base, 0x1800);
    } else if ((type == Type::Fire && weather == Weather::Rain) || (type == Type::Water && weather == Weather::Sunny)) {
        base = applymod(base, 0x800);
    }

    // 3. CRIT //
    if (crit) {
        if (gen < 6) {
            base = applymod(base, 0x2000);
        } else {
            base = applymod(base, 0x1800);
        }
    }

    // 4. SPLIT (RANDOM) //
    int dmin = base * 85 / 100;
    int dmax = base;

    // 5. STAB //
    int stabmod = 0x1000;

    if (mytype1 == type || mytype2 == type || myability == Ability::Protean) {
        if (myability == Ability::Adaptability) {
            stabmod = 0x2000;
        } else {
            stabmod = 0x1800;
        }
    }

    dmin = applymod(dmin, stabmod);
    dmax = applymod(dmax, stabmod);

    // 6. TYPE EFFECTIVENESS //
    int eff = TypeInfo::Eff(type, otype1, m_currentGen) * TypeInfo::Eff(type, otype2, m_currentGen); // each is doubled to retain int, so we divide by 2^2, or 4

    dmin = dmin * eff / 4;
    dmax = dmax * eff / 4;

    // 7. BURN //
    if (category == Move::Physical && mystatus == Pokemon::Burnt && myability != Ability::Guts) {
        dmin /= 2;
        dmax /= 2;
    }

    // 8. FINAL MOD //
    int fmod = 0x1000;
    // 8A. REFLECT/LIGHT SCREEN //
    if (ui->reflect->isChecked() && myability != Ability::Infiltrator && !crit) {
        fmod = chainmod(fmod, (ui->singlebattle->isChecked() ? 0x800 : 0xA8F));
    }
    // 8B. MULTISCALE //
    if (oability == Ability::MultiScale && ostats[0] == omaxhp) {
        fmod = chainmod(fmod, 0x800);
    }
    // 8C. TINTED LENS //
    if (eff < 4 && myability == Ability::TintedLens) {
        fmod = chainmod(fmod, 0x2000);
    }
    // 8D. FRIEND GUARD //
    if (ui->friendguard->isChecked()) {
        fmod = chainmod(fmod, 0xC00);
    }
    // 8E. SNIPER //
    if (crit && myability == Ability::Sniper) {
        fmod = chainmod(fmod, 0x1800);
    }
    // 8F. SOLID ROCK/FILTER //
    if (crit && (oability == Ability::SolidRock || oability == Ability::Filter)) {
        fmod = chainmod(fmod, 0xC00);
    }
    // 8G. METRONOME // 0x1000+n*0x333 if n≤4 and 0x2000 otherwise
    // 8H. EXPERT BELT //
    if (eff > 4 && myitem == Item::ExpertBelt) {
        fmod = chainmod(fmod, 0x1333);
    }
    // 8I. LIFE ORB //
    if (myitem == Item::LifeOrb) {
        fmod = chainmod(fmod, 0x14CC);
    }
    // 8J. DAMAGE-LOWERING BERRY // 0x800
    // 8K. STOMP ON MINIMIZE // 0x2000
    // 8L. EARTHQUAKE ON DIG // 0x2000
    // 8M. SURF ON DIVE // 0x2000
    // 8N. STEAMROLLER ON MINIMIZE // 0x2000
    if (ui->surfondive->isChecked()) {
        fmod = chainmod(fmod, 0x2000);
    }
    // 8O. FUR COAT //
    if (oability == Ability::FurCoat && category == Move::Physical) {
        fmod = chainmod(fmod, 0x0800);
    }

    dmin = applymod(dmin, fmod);
    dmax = applymod(dmax, fmod);

    showResult(dmin, dmax);
}

int DamageCalc::applymod(int d, int m)
{
    int a = d * m;
    float b = (float)a / 0x1000;
    return qRound(b);
}

int DamageCalc::chainmod(int m1, int m2)
{
    return ((m1 * m2) + 0x800) >> 12;
}

void DamageCalc::showResult(int min, int max)
{
    int omaxhp = PokemonInfo::Stat(PokemonInfo::Number(ui->opoke->currentText()), m_currentGen, 0, ui->olevel->value(), ui->ohpiv->value(), ui->ohpev->value());

    int pmin = min * 100 / omaxhp;
    int pmax = max * 100 / omaxhp;

    ui->result1->setValue(100 - std::min(pmin, 100));
    ui->result2->setValue(100 - std::min(pmax, 100));

    ui->resultText->setText("<b><u>Result:</u></b><br />"
                            + QString::number(min) + " - " + QString::number(max) + "<br />(" + QString::number(pmin) + "% - " + QString::number(pmax) + "%)");
}

void DamageCalc::setText(QComboBox *c, const QString &t)
{
    c->setCurrentIndex(qMax(0, c->findText(t)));
}

