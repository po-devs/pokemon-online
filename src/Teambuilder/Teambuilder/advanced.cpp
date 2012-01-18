#include "advanced.h"
#include "teambuilder_old.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "../PokemonInfo/movesetchecker.h"
#include "theme.h"

TB_Advanced::TB_Advanced(PokeTeam *_poke)
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    m_poke = _poke;

    QVBoxLayout *baselayout = new QVBoxLayout(this);

    QHBoxLayout *uplayout = new QHBoxLayout();
    baselayout->addLayout(uplayout);

    QVBoxLayout *firstColumn = new QVBoxLayout();
    QVBoxLayout *secondColumn = new QVBoxLayout();

    uplayout->addLayout(firstColumn,1);
    uplayout->addLayout(secondColumn,0);

    if (gen() >= 2) {
        QGroupBox *hiddenpower = new QGroupBox(tr("&Hidden Power"));
        firstColumn->addWidget(hiddenpower);
        QGridLayout *hidpower = new QGridLayout(hiddenpower);

        QLabel * l_type = new QLabel(tr("&Type:"));
        l_type->setObjectName("SmallText");
        hpchoice = new QComboBox();
        l_type->setBuddy(hpchoice);
        hidpower->addWidget(l_type, 0, 0);
        hidpower->addWidget(hpchoice, 0, 1);
        QLabel *l_power;
        hidpower->addWidget(l_power=new QLabel(tr("Power:")), 1,0);
        l_power->setObjectName("SmallText");
        hpower = new QLabel();
        hpower->setObjectName("SmallText");
        hidpower->addWidget(hpower,1,1);

        for (int i = 1; i < TypeInfo::NumberOfTypes() - 1; i++)
        {
            hpchoice->addItem(TypeInfo::Name(i));
        }
        connect(hpchoice, SIGNAL(activated(int)), SLOT(changeHiddenPower(int)));
    }

    QGroupBox *dvs = new QGroupBox(tr("&DVs"));
    firstColumn->addWidget(dvs);
    QGridLayout *dvlayout = new QGridLayout(dvs);
    QStringList stats_l;
    stats_l << tr("HP:") << tr("Att:") << tr("Def:") << (gen() == 1 ? tr("Special:", "Stat") : tr("Sp.Att:")) << tr("Sp.Def:") << tr("Speed:");

    for (int i = 0; i < 6; i++)
    {
        QLabel * l = new QLabel(stats_l[i]);
        l->setObjectName("BigText");
        QSpinBox * s = new QSpinBox();
        dvchoice[i] = s;
        l->setBuddy(s);
        dvlayout->addWidget(l, i, 0);
        dvlayout->addWidget(s, i, 1);

        dvchoice[i]->setRange(0, gen() <= 2 ? 15 : 31);
        dvchoice[i]->setAccelerated(true);
        connect(dvchoice[i], SIGNAL(valueChanged(int)), SLOT(changeDV(int)));

        if (gen() <= 2 && i == Hp) {
            dvchoice[i]->setDisabled(true);
        }

        dvlayout->addWidget((stats[i]=new QLabel()), i, 2);
        stats[i]->setObjectName("BigText");

        if (gen() == 1 && i == SpDefense) {
            l->hide();
            stats[i]->hide();
            s->hide();
        }
    }

    if (gen() >= 2) {
        QHBoxLayout *happLayout = new QHBoxLayout();
        firstColumn->addLayout(happLayout);
        QLabel *happLabel = new QLabel(tr("Happiness"));
        happLabel->setObjectName("BigText");
        happLayout->addWidget(happLabel);
        happLayout->addWidget(happiness = new QSpinBox());
        happiness->setRange(0, 255);
        happiness->setFixedWidth(50);
        happiness->setValue(poke()->happiness());
        connect(happiness, SIGNAL(valueChanged(int)), SLOT(changeHappiness(int)));
    }

    secondColumn->addWidget(pokeImage=new QLabel(),0,Qt::AlignHCenter);
    pokeImage->setObjectName("PokemonPicture");
    updatePokeImage();

    QHBoxLayout *levellayout = new QHBoxLayout();
    secondColumn->addLayout(levellayout);
    QLabel * l_lvl = new QLabel(tr("&Level"));
    l_lvl->setObjectName("BigText");
    levellayout->addWidget(l_lvl);
    levellayout->addWidget(level = new QSpinBox());
    l_lvl->setBuddy(level);
    level->setRange(1,100);
    level->setValue(poke()->level());
    level->setAccelerated(true);
    connect(level, SIGNAL(valueChanged(int)), SLOT(changeLevel(int)));

    if (gen() >= 3) {
        QGroupBox *gender = new QGroupBox(tr("&Gender"));
        QVBoxLayout *genderLayout = new QVBoxLayout(gender);
        secondColumn->addWidget(gender);

        if (poke()->genderAvail() == Pokemon::MaleAndFemaleAvail)
        {
            genderLayout->addWidget(gender1 = new QRadioButton(tr("Male")));
            genderLayout->addWidget(gender2 = new QRadioButton(tr("Female")));
            connect(gender1, SIGNAL(toggled(bool)), SLOT(changeGender(bool)));
            updateGender();
        } else {
            genderLayout->addWidget(gender1 = new QRadioButton( poke()->gender() == Pokemon::Neutral ? tr("Neutral") : (poke()->gender() == Pokemon::Female ? tr("Female") : tr("Male"))));
            gender1->setChecked(true);
            gender1->setEnabled(false);
        }

        QGroupBox *abilityB = new QGroupBox(tr("&Ability"));
        secondColumn->addWidget(abilityB);
        QVBoxLayout *abilityLayout = new QVBoxLayout(abilityB);

        abilityLayout->addWidget(ability[0]=new QRadioButton(AbilityInfo::Name(poke()->abilities().ab(0))));
        ability[0]->setToolTip(AbilityInfo::Desc(poke()->abilities().ab(0)));

        if ( (poke()->abilities().ab(1) == poke()->abilities().ab(2)) && (poke()->abilities().ab(1) == 0) ) {
            ability[0]->setChecked(true);
            ability[0]->setEnabled(false);
        } else {
            for (int i = 1; i < 3; i++) {
                if (poke()->abilities().ab(i) != 0 && poke()->abilities().ab(i) != poke()->abilities().ab(0)) {
                    abilityLayout->addWidget(ability[i]=new QRadioButton(AbilityInfo::Name(poke()->abilities().ab(i))));
                    ability[i]->setToolTip(AbilityInfo::Desc(poke()->abilities().ab(i)));
                    connect(ability[i], SIGNAL(toggled(bool)), SLOT(changeAbility(bool)));
                } else {
                    ability[i] = 0;
                }
            }
            updateAbility();
        }
    }

    if (gen() >= 2) {
        secondColumn->addWidget(shiny = new QCheckBox(tr("&Shiny")));
        if (poke()->shiny()) {
            shiny->setChecked(true);
        }
        if (gen() == 2) { // This because changeShininess is called also when an IV is changed, so you wouldn't be able to change IVs when it's not shiny
            connect(shiny, SIGNAL(toggled(bool)), SLOT(changeShininess2(bool)));
        } else {
            connect(shiny, SIGNAL(toggled(bool)), SLOT(changeShininess(bool)));
        }

        QPushButton *bForms = new QPushButton(tr("Alternate Formes"));
        QMenu *m= new QMenu(bForms);

        if (PokemonInfo::HasFormes(poke()->num()) && PokemonInfo::AFormesShown(poke()->num())) {
            QList<Pokemon::uniqueId> formes = PokemonInfo::Formes(poke()->num(), gen());

            foreach(Pokemon::uniqueId forme, formes) {
                QAction *ac = m->addAction(PokemonInfo::Name(forme),this, SLOT(changeForme()));
                ac->setCheckable(true);
                if (forme == poke()->num()) {
                    ac->setChecked(true);
                }
                ac->setProperty("pokemonid", forme.toPokeRef());
            }

            bForms->setMenu(m);
        } else {
            bForms->setDisabled(true);
        }

        baselayout->addWidget(bForms);

        stats_l.clear();
        stats_l << tr("HP") << tr("Att") << tr("Def") << tr("Sp Att") << tr("Sp Def") << tr("Speed");

        hpanddvchoice = new QCompactTable(0, 6);

        baselayout->addWidget(hpanddvchoice,Qt::AlignCenter);

        hpanddvchoice->horizontalHeader()->setStretchLastSection(true);
        hpanddvchoice->setHorizontalHeaderLabels(stats_l);

        for (int i = 0; i < 6; i++) {
            hpanddvchoice->horizontalHeader()->resizeSection(i, 44);
        }

        connect(hpanddvchoice, SIGNAL(cellActivated(int,int)), SLOT(changeDVsAccordingToHP(int)));
    }

    updateDVs();
    updateHiddenPower();
    updateStats();
}

void TB_Advanced::changeForme()
{
    QAction *ac = (QAction*) sender();
    if (ac->property("pokemonid").toUInt() == poke()->num().toPokeRef()) {
        ac->setChecked(true);
        return;
    }
    else {
        emit pokeFormeChanged(Pokemon::uniqueId(ac->property("pokemonid").toInt()));
    }
}

void TB_Advanced::changeAbility(bool)
{
    for (int i = 0; i < 3; i++) {
        if (ability[i] != NULL && ability[i]->isChecked()) {
            poke()->ability() = poke()->abilities().ab(i);
            emit abilityChanged();
            break;
        }
    }
}

void TB_Advanced::changeShininess(bool shine)
{
    if (shine != shiny->isChecked()) {
        shiny->setChecked(shine);
    }
    poke()->shiny() = shine;
    updatePokeImage();
    emit imageChanged();
}

void TB_Advanced::changeShininess2(bool shine)
{
    if (shine == poke()->shiny()) {
        return;
    }

    uchar stats[6];

    memset(stats,  shine ? 10: 15, 6);
    stats[Attack] = 15;

    for (unsigned i = Attack; i < 6; i++) {
        poke()->setDV(i, stats[i]);
    }
    updateDVs();

    changeShininess(poke()->shiny());
    updateHiddenPower();
}

void TB_Advanced::changeGender(bool gend1)
{
    poke()->gender() = gend1 ? Pokemon::Male : Pokemon::Female;
    updatePokeImage();
    emit genderChanged();
}

void TB_Advanced::changeHappiness(int x)
{
    poke()->happiness() = x;
}

void TB_Advanced::updatePokeImage()
{
    pokeImage->setFixedSize(poke()->picture().size());
    pokeImage->setPixmap(poke()->picture());
}

void TB_Advanced::updateAbility()
{
    for (int i = 0; i < 3; i++) {
        if (poke()->ability() == poke()->abilities().ab(i) && ability[i]) {
            ability[i]->setChecked(true);
        }
    }
}

void TB_Advanced::updateGender()
{
    if (poke()->genderAvail()== Pokemon::MaleAndFemaleAvail)
    {
        if (poke()->gender() == Pokemon::Male)
        {
            gender1->setChecked(true);
        } else {
            gender2->setChecked(true);
        }
    }
}

void TB_Advanced::changeDV(int stat, int newval)
{
    if (poke()->DV(stat) != newval)
    {
        poke()->setDV(stat, newval);

        /* Making Sp Atk and Sp Def coordinated in gen 2 */
        if (gen() == 2 && (stat == SpDefense || stat == SpAttack)) {
            updateDV(SpDefense);
            updateDV(SpAttack);
        } else {
            updateDV(stat);
        }

        if (gen() <= 2) {
            updateDV(Hp);
            if (gen() == 2) {
                changeShininess(poke()->shiny());
                changeGender(poke()->gender());
            }
        }

        updateHiddenPower();
    }
}

int TB_Advanced::gen() const
{
    return m_poke->gen().num;
}

int TB_Advanced::stat(QObject *dvchoiceptr)
{
    for (int i = 0; i < 6; i++) {
        if (dvchoice[i] == dvchoiceptr)
            return i;
    }

    throw tr("Fatal error in TB_Advanced::stat(QObject *) : the pointer provided does not correspond to any dvchoice");
}

void TB_Advanced::changeDV(int newval)
{
    changeDV(stat(sender()), newval);
}

void TB_Advanced::updateDVs()
{
    for (int i = 0; i < 6; i++)
    {
        updateDV(i);
    }
}

void TB_Advanced::updateDV(int stat)
{
    dvchoice[stat]->setValue(poke()->DV(stat));
    updateStat(stat);
}

void TB_Advanced::changeLevel(int level)
{
    if (level == poke()->level()) {
        return;
    }
    if (MoveSetChecker::enforceMinLevels &&
            PokemonInfo::AbsoluteMinLevel(poke()->num(), gen()) > level) {
        level = PokemonInfo::AbsoluteMinLevel(poke()->num(), gen());
        poke()->level() = level;
        this->level->setValue(level);
    } else {
        poke()->level() = level;
    }
    updateStats();
    emit levelChanged();
}

void TB_Advanced::updateHiddenPower()
{
    if (gen() <= 1)
        return;

    hpower->setText(QString::number(calculateHiddenPowerPower()));

    int type = calculateHiddenPowerType();

    hpchoice->setCurrentIndex(type - 1);
    updateHpAndDvChoice();
}

void TB_Advanced::updateHpAndDvChoice()
{
    if (gen() <= 2)
        return;

    int type = currentHiddenPower();

    QList<QStringList> hpAndDvVals = HiddenPowerInfo::PossibilitiesForType(type);

    hpanddvchoice->setRowCount(0);
    hpanddvchoice->setRowCount(hpAndDvVals.size());

    for (int i = 0; i < hpAndDvVals.size(); i++)
    {
        for (int j = 0; j < 6; j++)
        {
            QLabel *l = new QLabel(hpAndDvVals[i][j]);
            l->setAlignment(Qt::AlignHCenter);
            hpanddvchoice->setCellWidget(i, j, l);
        }
    }

    emit statChanged();
}

int TB_Advanced::currentHiddenPower() const
{
    return hpchoice->currentIndex()+1;
}

const PokeTeam * TB_Advanced::poke() const
{
    return m_poke;
}

int TB_Advanced::calculateHiddenPowerPower() const
{
    return HiddenPowerInfo::Power(poke()->gen(), poke()->DV(0), poke()->DV(1), poke()->DV(2), poke()->DV(3), poke()->DV(4), poke()->DV(5));
}

int TB_Advanced::calculateHiddenPowerType() const
{
    return HiddenPowerInfo::Type(poke()->gen(), poke()->DV(0), poke()->DV(1), poke()->DV(2), poke()->DV(3), poke()->DV(4), poke()->DV(5));
}

void TB_Advanced::changeHiddenPower(int newtype)
{
    newtype += 1;
    if (newtype == calculateHiddenPowerType())
        return;

    if (gen() >= 3) {
        updateHpAndDvChoice();
        //We pick the first possible set of DVs (defaulted as the 'best' possible one)
        changeDVsAccordingToHP(0);
    } else {
        QPair<quint8,quint8> dvs = HiddenPowerInfo::AttDefDVsForGen2(newtype);

        changeDV(Attack, dvs.first);
        changeDV(Defense, dvs.second);
    }
}

void TB_Advanced::changeDVsAccordingToHP(int row)
{
    /* Now we have to change the dvs... */
    for (int i = 0; i < 6; i++) {
        /* If the item at row0 and column i (i=current stat) is a string representing an odd number,
       then odd is true */

        int numeric_value = (dynamic_cast<QLabel*>(hpanddvchoice->cellWidget(row, i)))->text().toInt();
        bool odd =  numeric_value % 2;

        if (odd == poke()->DV(i)%2) {
            /* if that stat has the same parity, no need to change it, as the type is only based
           on parity */
        } else {
            /* changing the parity of the stat */
            if (odd == false)
                poke()->setDV(i, poke()->DV(i)-1);
            else
                poke()->setDV(i, poke()->DV(i)+1);

            updateDV(i);
        }
    }
}

void TB_Advanced::updateStats()
{
    for (int i = 0; i < 6; i++)
        updateStat(i);
    emit statChanged();
}

void TB_Advanced::updateStat(int stat)
{
    if (poke()->natureBoost(stat) != 0) {
        QColor color;

        switch (poke()->natureBoost(stat)) {
        case -1: color = Theme::Color("Teambuilder/statHindered"); break;
        case 1: color = Theme::Color("Teambuilder/statRaised"); break;
        }

        stats[stat]->setText(toColor(QString::number(poke()->stat(stat)), color));
    } else {
        stats[stat]->setText(QString::number(poke()->stat(stat)));
    }
}

PokeTeam *TB_Advanced::poke()
{
    return m_poke;
}
