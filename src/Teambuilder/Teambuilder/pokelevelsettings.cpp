#include "../PokemonInfo/pokemonstructs.h"
#include "Teambuilder/pokelevelsettings.h"
#include "ui_pokelevelsettings.h"

PokeLevelSettings::PokeLevelSettings(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PokeLevelSettings)
{
    ui->setupUi(this);

    connect(ui->level, SIGNAL(valueChanged(int)), this, SLOT(changeLevel(int)));
    connect(ui->shiny, SIGNAL(toggled(bool)), this, SLOT(changeShinyness(bool)));
    connect(ui->maleButton, SIGNAL(toggled(bool)), this, SLOT(changeGender()));
    // I'll leave it commented, but we don't need this since with checking if maleButton is checked
    // the gender is set to Male and if it's not checks if femaleButton is checked and set gender to
    // female if it is.
    //connect(ui->femaleButton, SIGNAL(toggled(bool)), this, SLOT(changeGender()));
}

PokeLevelSettings::~PokeLevelSettings()
{
    delete ui;
}

void PokeLevelSettings::setPoke(PokeTeam *poke)
{
    m_poke = poke;
    setGender();
}

void PokeLevelSettings::updateAll()

{
    ui->level->setValue(poke().level());

    setGender();
    updateGender();
    updateShiny();

    if (poke().gen() <= 1) {
        ui->shiny->hide();
        ui->genderLabel->hide();
        ui->line->hide();
        ui->line_2->hide();
    } else {
        ui->shiny->show();
        ui->genderLabel->show();
        ui->line->show();
        ui->line_2->show();
    }

    if (poke().gen() <= 2) {
        ui->shiny->setDisabled(true);
    } else {
        ui->shiny->setDisabled(false);
    }
}

void PokeLevelSettings::setGender()
{
    if (poke().gen() <= 1) {
        ui->maleButton->hide();
        ui->femaleButton->hide();
        ui->neutralButton->hide();
    } else {
        if (poke().genderAvail() == Pokemon::MaleAvail) {
            ui->maleButton->setVisible(true);
            ui->femaleButton->setVisible(false);
            ui->neutralButton->setVisible(false);
            ui->maleButton->setDisabled(true);
        } else if (poke().genderAvail() == Pokemon::FemaleAvail) {
            ui->maleButton->setVisible(false);
            ui->femaleButton->setVisible(true);
            ui->neutralButton->setVisible(false);
            ui->femaleButton->setDisabled(true);
        } else if (poke().genderAvail() == Pokemon::NeutralAvail) {
            ui->maleButton->setVisible(false);
            ui->femaleButton->setVisible(false);
            ui->neutralButton->setVisible(true);
        } else { //male & femal
            ui->maleButton->setVisible(true);
            ui->femaleButton->setVisible(true);
            ui->neutralButton->setVisible(false);
            ui->maleButton->setDisabled(false);
            ui->femaleButton->setDisabled(false);
        }

        if (poke().gen() <= 2) {
            ui->maleButton->setDisabled(true);
            ui->femaleButton->setDisabled(true);
            ui->neutralButton->setDisabled(true);
        }
    }
}

void PokeLevelSettings::updateGender()
{
    if (poke().gender() == Pokemon::Female) {
        ui->femaleButton->setChecked(true);
    } else if (poke().gender() == Pokemon::Male) {
        ui->maleButton->setChecked(true);
    } else {
        ui->neutralButton->setChecked(true);
    }
}

void PokeLevelSettings::updateShiny()
{
    ui->shiny->setChecked(poke().shiny());
}

void PokeLevelSettings::changeLevel(int newLevel)
{
    poke().level() = newLevel;
    emit levelUpdated();
}

void PokeLevelSettings::changeShinyness(bool isShiny)
{
    poke().shiny() = isShiny;
    emit shinyUpdated();
}

void PokeLevelSettings::changeGender()
{
    if(ui->maleButton->isChecked()) {
        poke().gender() = Pokemon::Male;
    } else {
        if(ui->femaleButton->isChecked()) {
            poke().gender() = Pokemon::Female;
        }
    }
    emit genderUpdated();
}
