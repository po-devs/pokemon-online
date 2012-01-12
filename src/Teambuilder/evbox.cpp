#include "../PokemonInfo/pokemonstructs.h"
#include "evbox.h"
#include "ui_evbox.h"
#include "theme.h"

EvBox::EvBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EvBox)
{
    ui->setupUi(this);

    QSlider *sliders[6] = {ui->hpslider, ui->atkslider, ui->defslider, ui->satkslider, ui->sdefslider, ui->speedslider};
    QLabel *descs[6] = {ui->hpdesc, ui->atkdesc, ui->defdesc, ui->satkdesc, ui->sdefdesc, ui->speeddesc};
    QLabel *labels[6] = {ui->hplabel, ui->atklabel, ui->deflabel, ui->satklabel, ui->sdeflabel, ui->speedlabel};
    QLineEdit *edits[6] = {ui->hpedit, ui->atkedit, ui->defedit, ui->satkedit, ui->sdefedit, ui->speededit};

    memcpy(m_sliders, sliders, sizeof(sliders));
    memcpy(m_descs, descs, sizeof(descs));
    memcpy(m_stats, labels, sizeof(labels));
    memcpy(m_evs, edits, sizeof(edits));

#define setNums(var) for (int i = 0; i < 6; i++) { var[i]->setProperty("stat", i);}
    setNums(m_sliders);
    setNums(m_evs);
#undef setNums

    for(int i = 0; i < 6; i++) {
        connect(m_sliders[i], SIGNAL(valueChanged(int)), this, SLOT(changeEV(int)));
        connect(m_evs[i], SIGNAL(textChanged(QString)), this, SLOT(changeEV(QString)));
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
}

void EvBox::updateEV(int stat)
{
    int ev = poke().EV(stat);

    m_sliders[stat]->setValue(ev);
    m_evs[stat]->setText(QString::number(ev));

    /* first the color : red if the stat is hindered by the nature, black if normal, blue if the stat is enhanced */
    QColor color;

    switch (poke().natureBoost(stat)) {
    case -1: color = Theme::Color("Teambuilder/statHindered"); break;
    case 1: color = Theme::Color("Teambuilder/statRaised"); break;
    }

    m_stats[stat]->setText(toColor(QString::number(poke().stat(stat)), color));
}

void EvBox::updateMain()
{
    ui->totalslider->setValue(poke().EVSum());
    ui->totallabel->setText(QString::number(poke().EVSum()));
}

void EvBox::changeEV(int newValue)
{
    int stat = sender()->property("stat").toInt();
    poke().setEV(stat, newValue);
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
