#include "advanced.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/pokemoninfo.h"

TB_Advanced::TB_Advanced(PokeTeam *_poke)
{
    resize(300,400);

    m_poke = _poke;

    QVBoxLayout *baselayout = new QVBoxLayout(this);
    QHBoxLayout *uplayout = new QHBoxLayout();
    baselayout->addLayout(uplayout);

    QVBoxLayout *firstColumn = new QVBoxLayout();
    QVBoxLayout *secondColumn = new QVBoxLayout();

    uplayout->addLayout(firstColumn,1);
    uplayout->addLayout(secondColumn,0);

    QGroupBox *hiddenpower = new QGroupBox(tr("&Hidden Power"));
    firstColumn->addWidget(hiddenpower);
    QGridLayout *hidpower = new QGridLayout(hiddenpower);

    QLabel * l_type = new QLabel("&Type:");
    hpchoice = new QComboBox();
    l_type->setBuddy(hpchoice);
    hpower = new QLabel();
    hidpower->addWidget(l_type, 0, 0);
    hidpower->addWidget(hpchoice, 0, 1);
    hidpower->addWidget(new QLabel("Power:"), 1,0);
    hidpower->addWidget(hpower,1,1);

    for (int i = 1; i < TypeInfo::NumberOfTypes() - 1; i++)
    {
	hpchoice->addItem(TypeInfo::Name(i));
    }

    connect(hpchoice, SIGNAL(activated(int)), SLOT(changeHiddenPower(int)));

    QGroupBox *dvs = new QGroupBox("&DVs");
    firstColumn->addWidget(dvs);
    QGridLayout *dvlayout = new QGridLayout(dvs);
    QStringList stats_l;
    stats_l << "Hit Points" << "Attack" << "Defense" << "Speed" << "Special attack" << "Special defense";

    for (int i = 0; i < 6; i++)
    {
        QLabel * l = new QLabel(stats_l[i]);
        QSpinBox * s = new QSpinBox();
        dvchoice[i] = s;
        l->setBuddy(s);
        dvlayout->addWidget(l, i, 0);
        dvlayout->addWidget(s, i, 1);

	dvchoice[i]->setRange(0,31);
	dvchoice[i]->setAccelerated(true);
	connect(dvchoice[i], SIGNAL(valueChanged(int)), SLOT(changeDV(int)));

	QColor colors[3] = {Qt::darkBlue, Qt::black, Qt::red};
	QColor mycol = colors[poke()->natureBoost(i)+1];

	dvlayout->addWidget((stats[i]=new QLabel()), i, 2);

	QPalette pal = stats[i]->palette();
	pal.setColor(QPalette::WindowText, mycol);
	stats[i]->setPalette(pal);
    }

    secondColumn->addWidget(pokeImage=new QLabel());
    updatePokeImage();

    QHBoxLayout *levellayout = new QHBoxLayout();
    secondColumn->addLayout(levellayout);
    QLabel * l_lvl = new QLabel(tr("&Level"));
    levellayout->addWidget(l_lvl);
    levellayout->addWidget(level = new QSpinBox());
    l_lvl->setBuddy(level);
    level->setRange(2,100);
    level->setValue(poke()->level());
    level->setAccelerated(true);
    connect(level, SIGNAL(valueChanged(int)), SLOT(changeLevel(int)));

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

    QGroupBox *ability = new QGroupBox(tr("&Ability"));
    secondColumn->addWidget(ability);
    QVBoxLayout *abilityLayout = new QVBoxLayout(ability);

    abilityLayout->addWidget(ability1=new QRadioButton(AbilityInfo::Name(poke()->abilities()[0])));
    if (poke()->abilities()[1] != 0) {
	abilityLayout->addWidget(ability2=new QRadioButton(AbilityInfo::Name(poke()->abilities()[1])));
	connect(ability1, SIGNAL(toggled(bool)), SLOT(changeAbility(bool)));
	updateAbility();
    } else {
	ability1->setChecked(true);
	ability1->setEnabled(false);
    }

    secondColumn->addWidget(shiny = new QCheckBox(tr("&Shiny")));
    if (poke()->shiny()) {
	shiny->setChecked(true);
    }
    connect(shiny, SIGNAL(toggled(bool)), SLOT(changeShininess(bool)));

	stats_l.clear();
    stats_l << "HP" << "Att" << "Def" << "Speed" << "Sp Att" << "Sp Def";

    hpanddvchoice = new QCompactTable(0, 6);
    hpanddvchoice->setMinimumHeight(160);
    hpanddvchoice->setSelectionBehavior(QAbstractItemView::SelectRows);
    hpanddvchoice->setShowGrid(false);
    hpanddvchoice->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    baselayout->addWidget(hpanddvchoice,Qt::AlignCenter);

    hpanddvchoice->horizontalHeader()->setStretchLastSection(true);
    hpanddvchoice->verticalHeader()->hide();
    hpanddvchoice->setHorizontalHeaderLabels(stats_l);
    hpanddvchoice->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(hpanddvchoice, SIGNAL(cellActivated(int,int)), SLOT(changeDVsAccordingToHP(int)));

    updateDVs();
    updateHiddenPower();
    
    /* This is to get the widgets to get to their actual widths */
    show();

    int width = hpanddvchoice->horizontalHeader()->width();
    /* making 6 columns of the same width */
    for (int i = 0; i < 6; i++) {
	hpanddvchoice->setColumnWidth(i, width/6);
    }
}

void TB_Advanced::changeAbility(bool ab1)
{
    poke()->ability() = ab1? poke()->abilities()[0] : poke()->abilities()[1];
}

void TB_Advanced::changeShininess(bool shine)
{
    poke()->shiny() = shine;
    updatePokeImage();
}

void TB_Advanced::changeGender(bool gend1)
{
    poke()->gender() = gend1 ? Pokemon::Male : Pokemon::Female;
    updatePokeImage();
}

void TB_Advanced::updatePokeImage()
{
    pokeImage->setPixmap(poke()->picture());
}

void TB_Advanced::updateAbility()
{
    if (poke()->ability() == poke()->abilities()[0])
    {
	ability1->setChecked(true);
    } else {
	ability2->setChecked(true);
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
	updateDV(stat);
	updateHiddenPower();
    }
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
    poke()->level() =level;
    updateStats();
}

void TB_Advanced::updateHiddenPower()
{
    hpower->setText(QString::number(calculateHiddenPowerPower()));

    int type = calculateHiddenPowerType();

    hpchoice->setCurrentIndex(type - 1);
    updateHpAndDvChoice();
}

void TB_Advanced::updateHpAndDvChoice()
{
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
    return HiddenPowerInfo::Power(poke()->DV(0), poke()->DV(1), poke()->DV(2), poke()->DV(3), poke()->DV(4), poke()->DV(5));
}

int TB_Advanced::calculateHiddenPowerType() const
{
    return HiddenPowerInfo::Type(poke()->DV(0), poke()->DV(1), poke()->DV(2), poke()->DV(3), poke()->DV(4), poke()->DV(5));
}

void TB_Advanced::changeHiddenPower(int newtype)
{
    if (newtype == calculateHiddenPowerType())
	return;

    updateHpAndDvChoice();
    //We pick the first possible set of DVs (defaulted as the 'best' possible one)
    changeDVsAccordingToHP(0);
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
}

void TB_Advanced::updateStat(int stat)
{
    stats[stat]->setText(QString::number(poke()->stat(stat)));
}

PokeTeam *TB_Advanced::poke()
{
    return m_poke;
}
