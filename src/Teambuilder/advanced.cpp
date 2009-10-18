#include "advanced.h"
#include <QtGui>

TB_Advanced::TB_Advanced(PokeTeam *_poke)
{
    resize(300,400);

    m_poke = _poke;

    QHBoxLayout *layout = new QHBoxLayout(this);
    QVBoxLayout *firstColumn = new QVBoxLayout();
    QVBoxLayout *secondColumn = new QVBoxLayout();

    layout->addLayout(firstColumn,1);
    layout->addLayout(secondColumn,0);

    QGroupBox *hiddenpower = new QGroupBox(tr("Hidden Power"));
    firstColumn->addWidget(hiddenpower);
    QGridLayout *hidpower = new QGridLayout(hiddenpower);

    hpchoice = new QComboBox();
    hpower = new QLabel();
    hidpower->addWidget(new QLabel("Type:"), 0, 0);
    hidpower->addWidget(hpchoice, 0, 1);
    hidpower->addWidget(new QLabel("Power:"), 1,0);
    hidpower->addWidget(hpower,1,1);

    for (int i = 1; i < TypeInfo::NumberOfTypes() - 1; i++)
    {
	hpchoice->addItem(TypeInfo::Name(i));
    }

    QGroupBox *dvs = new QGroupBox("DVs");
    firstColumn->addWidget(dvs);
    QGridLayout *dvlayout = new QGridLayout(dvs);
    QStringList stats_l;
    stats_l << "Hit Points" << "Attack" << "Defense" << "Speed" << "Special attack" << "Special defense";

    for (int i = 0; i < 6; i++)
    {
	dvlayout->addWidget(new QLabel(stats_l[i]), i, 0);
	dvlayout->addWidget((dvchoice[i]= new QComboBox()), i, 1);

	QColor colors[3] = {Qt::darkBlue, Qt::black, Qt::red};
	QColor mycol = colors[poke()->natureBoost(i)+1];

	dvlayout->addWidget((stats[i]=new QLabel()), i, 2);

	QPalette pal = stats[i]->palette();
	pal.setColor(QPalette::WindowText, mycol);
	stats[i]->setPalette(pal);
    }
    stats_l.clear();
    stats_l << "HP" << "Attack" << "Defense" << "Speed" << "SpAttack" << "SpDefense";

    hpanddvchoice = new QCompactTable(5, 6);
    hpanddvchoice->verticalHeader()->hide();
    hpanddvchoice->setHorizontalHeaderLabels(stats_l);

    secondColumn->addWidget(pokeImage=new QLabel());

    QHBoxLayout *levellayout = new QHBoxLayout();
    secondColumn->addLayout(levellayout);
    levellayout->addWidget(new QLabel(tr("Level")));
    levellayout->addWidget(level = new QComboBox());

    QGroupBox *gender = new QGroupBox(tr("Gender"));
    QVBoxLayout *genderLayout = new QVBoxLayout(gender);
    secondColumn->addWidget(gender);

    switch(poke()->genderAvail())
    {
	case Pokemon::NeutralAvail:
	    genderLayout->addWidget(new QRadioButton("Neutral"));
	    break;
	case Pokemon::FemaleAvail:
	    genderLayout->addWidget(new QRadioButton("Female"));
	    break;
	case Pokemon::MaleAvail:
	    genderLayout->addWidget(new QRadioButton("Male"));
	    break;
	default:
	    genderLayout->addWidget(gender1 = new QRadioButton("Male"));
	    genderLayout->addWidget(gender2 = new QRadioButton("Female"));
	    break;
    }


    QGroupBox *ability = new QGroupBox(tr("Ability"));
    secondColumn->addWidget(ability);
    QVBoxLayout *abilityLayout = new QVBoxLayout();

    abilityLayout->addWidget(ability1=new QRadioButton(AbilityInfo::Name(poke()->abilities()[0])));
    if (poke()->abilities()[1] != 0)
	abilityLayout->addWidget(ability2=new QRadioButton(AbilityInfo::Name(poke()->abilities()[1])));

    secondColumn->addWidget(shiny = new QCheckBox(tr("Shiny")));
}

void TB_Advanced::changeAbility()
{}

void TB_Advanced::changeShininess()
{}

PokeTeam *TB_Advanced::poke()
{
    return m_poke;
}
