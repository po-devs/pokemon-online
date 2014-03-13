#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/pokemoninfo.h>
#include <TeambuilderLibrary/theme.h>
#include "evbox.h"
#include "ui_evbox.h"

EvBox::EvBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EvBox)
{
    ui->setupUi(this);

    QSlider *sliders[6] = {ui->hpslider, ui->atkslider, ui->defslider, ui->satkslider, ui->sdefslider, ui->speedslider};
    QLabel *descs[6] = {ui->hpdesc, ui->atkdesc, ui->defdesc, ui->satkdesc, ui->sdefdesc, ui->speeddesc};
    QLabel *labels[6] = {ui->hplabel, ui->atklabel, ui->deflabel, ui->satklabel, ui->sdeflabel, ui->speedlabel};
    QLineEdit *edits[6] = {ui->hpedit, ui->atkedit, ui->defedit, ui->satkedit, ui->sdefedit, ui->speededit};
    QImageButtonLR *boosts[6] = {ui->hpboost, ui->atkboost, ui->defboost, ui->satkboost, ui->sdefboost, ui->speedboost};

    memcpy(m_sliders, sliders, sizeof(sliders));
    memcpy(m_descs, descs, sizeof(descs));
    memcpy(m_stats, labels, sizeof(labels));
    memcpy(m_evs, edits, sizeof(edits));
    memcpy(m_boosts, boosts, sizeof(boosts));

#define setNums(var) for (int i = 0; i < 6; i++) { var[i]->setProperty("stat", i);}
    setNums(m_sliders);
    setNums(m_evs);
    setNums(m_boosts);
#undef setNums

    for (int i = 0; i < 6; i++) {
        connect(m_sliders[i], SIGNAL(valueChanged(int)), this, SLOT(changeEV(int)));
        connect(m_evs[i], SIGNAL(textChanged(QString)), this, SLOT(changeEV(QString)));
        connect(m_boosts[i], SIGNAL(rightClick()), this, SLOT(changeToMinusBoost()));
        connect(m_boosts[i], SIGNAL(leftClick()), this, SLOT(changeToPlusBoost()));
    }
}

void EvBox::updateNatureButtons()
{

    bool oldgen = poke().gen() <= 2;

    for (int i = 0; i < 6; i++) {
            m_boosts[i]->setHidden(oldgen);
    }


		
    for (int j = 1; j<6;j++){
        if (NatureInfo::Boost(poke().nature(),j) == 1)
            Theme::ChangePics(m_boosts[j], "plus");
        else if(NatureInfo::Boost(poke().nature(),j) == 0)
            Theme::ChangePics(m_boosts[j], "equal");
        else
            Theme::ChangePics(m_boosts[j], "minus");
    }
}

EvBox::~EvBox()
{
    delete ui;
}

void EvBox::setPoke(PokeTeam *poke)
{
    m_poke = poke;
}

void EvBox::updateAll()
{
    for (int i = 0; i < 6; i++) {
        updateEV(i);
    }

    updateMain();
    updateNatureButtons();

    if (poke().gen() <= 1) {
        ui->sdefboost->hide();
        ui->sdefdesc->hide();
        ui->sdefedit->hide();
        ui->sdeflabel->hide();
        ui->sdefslider->hide();
        ui->satkdesc->setText(tr("Special: "));
    } else {
        ui->sdefboost->show();
        ui->sdefdesc->show();
        ui->sdefedit->show();
        ui->sdeflabel->show();
        ui->sdefslider->show();
        ui->satkdesc->setText(tr("Special Attack: "));
    }

    if (poke().gen() <= 2) {
        ui->sdefslider->setDisabled(true);
        ui->sdefedit->setDisabled(true);
        ui->sdefboost->setHidden(true);

    } else {
        ui->sdefslider->setDisabled(false);
        ui->sdefedit->setDisabled(false);
    }
}

void EvBox::updateEVs()
{
    for (int i = 0; i < 6; i++) {
        updateEV(i);
    }
}

void EvBox::updateEV(int stat)
{
    int ev = poke().EV(stat);

    m_sliders[stat]->setValue(ev);

    m_evs[stat]->blockSignals(true);
    m_evs[stat]->setText(QString::number(ev));
    m_evs[stat]->blockSignals(false);

    /* first the color : red if the stat is hindered by the nature, black if normal, blue if the stat is enhanced */
    QColor color;

    switch (poke().natureBoost(stat)) {
    case -1: color = Theme::Color("Teambuilder/statHindered"); break;
    case 1: color = Theme::Color("Teambuilder/statRaised"); break;
    }

    m_stats[stat]->setText(toColor(QString::number(poke().stat(stat)), color));

    if (poke().gen() <= 2 && stat == SpAttack) {
        updateEV(SpDefense);
    }
}

void EvBox::updateMain()
{
    ui->totalslider->setValue(poke().EVSum());
    ui->totallabel->setText(QString::number(poke().EVSum()));
}

void EvBox::changeEV(int newValue)
{
    int stat = sender()->property("stat").toInt();
    poke().setEV(stat, newValue/4*4);
    updateEV(stat);
    updateMain();
}

void EvBox::changeEV(const QString &newValue)
{
    int stat = sender()->property("stat").toInt();
    poke().setEV(stat, std::max(std::min(newValue.toInt(), 252), 0));
    updateEV(stat);
    updateMain();
}

void EvBox::changeToPlusBoost()
{
    int plus = sender()->property("stat").toInt();
    int minus = NatureInfo::StatHindered(poke().nature());

    if (plus == minus) {
        minus = NatureInfo::StatBoosted(poke().nature());
    } else if (minus == 0) {
        minus = plus == Attack ? Defense : Attack;
    }

    emit natureChanged(NatureInfo::NatureOf(plus,minus));
    emit natureBoostChanged();
}

void EvBox::changeToMinusBoost()
{
    int minus = sender()->property("stat").toInt();
    int plus = NatureInfo::StatBoosted(poke().nature());

    if (plus == minus) {
        plus = NatureInfo::StatHindered(poke().nature());
    } else if (plus == 0) {
        plus = minus == Attack ? Defense : Attack;
    }

    emit natureChanged(NatureInfo::NatureOf(plus,minus));
    emit natureBoostChanged();
}
