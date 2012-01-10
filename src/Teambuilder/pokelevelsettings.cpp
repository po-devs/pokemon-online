#include "../PokemonInfo/pokemonstructs.h"
#include "pokelevelsettings.h"
#include "ui_pokelevelsettings.h"

PokeLevelSettings::PokeLevelSettings(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PokeLevelSettings)
{
    ui->setupUi(this);
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
    ui->shiny->setChecked(poke().shiny());
    ui->level->setValue(poke().level());

    updateGender();
}

void PokeLevelSettings::setGender()
{
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
    } else {
        ui->maleButton->setVisible(true);
        ui->femaleButton->setVisible(true);
        ui->neutralButton->setVisible(false);
        ui->maleButton->setDisabled(false);
        ui->femaleButton->setDisabled(false);
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
