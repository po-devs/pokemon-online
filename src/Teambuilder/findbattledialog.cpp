#include "findbattledialog.h"
#include "ui_findbattledialog.h"
#include "teamholder.h"
#include "teamline.h"
#include "../PokemonInfo/battlestructs.h"

FindBattleDialog::FindBattleDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindBattleDialog)
{
    ui->setupUi(this);

    QSettings s;
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    ui->rated->setChecked(s.value("find_battle_force_rated").toBool());
    ui->sameTier->setChecked(s.value("find_battle_same_tier").toBool());
    ui->rangeOn->setChecked(s.value("find_battle_range_on").toBool());
    ui->range->setText(QString::number(s.value("find_battle_range").toInt()));

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

    s.setValue("find_battle_force_rated", d.rated);
    s.setValue("find_battle_same_tier", d.sameTier);
    s.setValue("find_battle_range_on", d.ranged);
    s.setValue("find_battle_range", d.range);

    emit findBattle(d);
}


void FindBattleDialog::setTeam(TeamHolder *t)
{
    for (int i = 0; i < t->count(); i++) {
        TeamLine *line = new TeamLine();
        line->setTeamTier(t->team(i), t->tier(i));
        teamLines.append(line);
        ui->teams->addWidget(line);
    }
}

FindBattleDialog::~FindBattleDialog()
{
    delete ui;
}
