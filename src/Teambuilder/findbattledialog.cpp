#include <PokemonInfo/battlestructs.h>
#include <PokemonInfo/teamholder.h>

#include "findbattledialog.h"
#include "ui_findbattledialog.h"
#include "Teambuilder/teamline.h"

FindBattleDialog::FindBattleDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindBattleDialog)
{
    ui->setupUi(this);

    QSettings s;
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    ui->rated->setChecked(s.value("FindBattle/ForceRated", false).toBool());
    ui->sameTier->setChecked(s.value("FindBattle/SameTier", true).toBool());
    ui->rangeOn->setChecked(s.value("FindBattle/RangeOn", true).toBool());
    ui->range->setText(QString::number(s.value("FindBattle/Range", 200).toInt()));

    connect(ui->findbattle, SIGNAL(clicked()), SLOT(close()));
    connect(ui->cancel, SIGNAL(clicked()), SLOT(close()));
    connect(ui->findbattle, SIGNAL(clicked()), SLOT(throwChallenge()));
}

void FindBattleDialog::throwChallenge()
{
    FindBattleData d;
    d.rated = ui->rated->isChecked();
    d.sameTier = ui->sameTier->isChecked();
    d.range = std::max(ui->range->text().toInt(), 100);
    d.ranged = ui->rangeOn->isChecked();
    d.teams = 0;

    for (int i = 0; i < teamLines.size(); i++) {
        if (teamLines[i]->isChecked()) {
            d.teams |= 1 << i;
        }
    }

    QSettings s;

    s.setValue("FindBattle/ForceRated", d.rated);
    s.setValue("FindBattle/SameTier", d.sameTier);
    s.setValue("FindBattle/RangeOn", d.ranged);
    s.setValue("FindBattle/Range", d.range);
    s.setValue("FindBattle/Teams", d.teams);

    emit findBattle(d);
}


void FindBattleDialog::setTeam(TeamHolder *t)
{
    QSettings s;
    int teams = s.value("FindBattle/Teams").toInt();

    for (int i = 0; i < t->officialCount(); i++) {
        TeamLine *line = new TeamLine();
        line->setTeamTier(t->team(i), t->tier(i));
        teamLines.append(line);
        ui->teams->addWidget(line);

        line->setChecked(teams == 0 || (teams & (1 << i)));
    }
}

FindBattleDialog::~FindBattleDialog()
{
    delete ui;
}
