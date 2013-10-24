#include "basestatswidget.h"
#include "ui_basestatswidget.h"
#include "../PokemonInfo/pokemoninfo.h"

BaseStatsWidget::BaseStatsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BaseStatsWidget)
{
    ui->setupUi(this);

    QProgressBar *bars[6] = {ui->hpbar, ui->atkbar, ui->defbar, ui->spatkbar, ui->spdefbar, ui->speedbar};
    memcpy(stats, bars, sizeof(bars));
}

BaseStatsWidget::~BaseStatsWidget()
{
    delete ui;
}

void BaseStatsWidget::setGen(const Pokemon::gen &gen)
{
    if (gen.num == 1) {
        ui->spdefbar->hide();
        ui->spdef->hide();
        ui->spatk->setText(tr("Special", "Special Stat"));
    } else {
        ui->spatk->setText(tr("Sp. Atk"));
        ui->spdef->show();
        ui->spdefbar->show();
    }
    curgen=gen;

    setNum(curnum);
}

void BaseStatsWidget::setNum(const Pokemon::uniqueId &num)
{
    PokeBaseStats b = PokemonInfo::BaseStats(num, curgen);

    for (int i = 0; i < 6; i++) {
        stats[i]->setValue(std::min(int(b.baseStat(i)), stats[i]->maximum()));
        stats[i]->setFormat(QString("%1").arg(int(b.baseStat(i))));
    }

    curnum = num;
}
