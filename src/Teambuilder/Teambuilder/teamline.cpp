#include "Teambuilder/teamline.h"
#include "ui_teamline.h"
#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/pokemoninfo.h>

TeamLine::TeamLine(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TeamLine)
{
    ui->setupUi(this);
}

TeamLine::~TeamLine()
{
    delete ui;
}

bool TeamLine::isChecked() const
{
    return ui->teamName->isChecked();
}

void TeamLine::setChecked(bool checked)
{
    ui->teamName->setChecked(checked);
}

void TeamLine::setTeamTier(const Team &team, const QString &tier)
{
    ui->teamName->setText(tr("%1 (%2)", "Team and tier in find battle").arg(team.name(), tier));

    QLabel *pokes[] = {ui->poke1, ui->poke2, ui->poke3, ui->poke4, ui->poke5, ui->poke6};

    for (int i = 0; i < 6; i++) {
        pokes[i]->setPixmap(PokemonInfo::Icon(team.poke(i).num()));
    }
}
