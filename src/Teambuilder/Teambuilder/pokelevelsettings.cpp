#include <QMessageBox>

#include <QStringListModel>
#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/pokemoninfo.h>
#include "Teambuilder/pokelevelsettings.h"
#include "Teambuilder/pokeedit.h"
#include "ui_pokelevelsettings.h"

PokeLevelSettings::PokeLevelSettings(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::PokeLevelSettings)
{
    ui->setupUi(this);
    QButtonGroup *genderGroup = new QButtonGroup(this);
    genderGroup->addButton(ui->maleButton);
    genderGroup->addButton(ui->femaleButton);
    genderGroup->addButton(ui->neutralButton);

    connect(ui->level, SIGNAL(valueChanged(int)), this, SLOT(changeLevel(int)));
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

void PokeLevelSettings::fillAbilities()
{
    if (!PokeEdit::hackMons) {
        ui->ability->hide();
        ui->ability1->show();
        ui->ability2->show();
        ui->ability3->show();
        m_abilities[0] = ui->ability1;
        m_abilities[1] = ui->ability2;
        m_abilities[2] = ui->ability3;

        QButtonGroup *abilityGroup = new QButtonGroup(this);
        for (int i =0; i < 3; i++) {
            abilityGroup->addButton(m_abilities[i]);
        }
        for (int i = 0; i < 3; i++) {
            m_abilities[i]->disconnect(SIGNAL(toggled(bool)), this);
            connect(m_abilities[i], SIGNAL(toggled(bool)), this, SLOT(changeAbility()));
        }
    } else {
        ui->ability1->hide();
        ui->ability2->hide();
        ui->ability3->hide();
        QStringList abilities;
        for (int i = 1; i < AbilityInfo::NumberOfAbilities(poke().gen().num); i++) {
            abilities.push_back(AbilityInfo::Name(i));
        }
        qSort(abilities);
        abilities.removeAll("");
        QString ability = AbilityInfo::Name(poke().ability());
        ui->ability->setModel(new QStringListModel(abilities, this));
        ui->ability->show();
        ui->ability->disconnect(SIGNAL(currentIndexChanged(int)), this);
        connect(ui->ability, SIGNAL(currentIndexChanged(int)), this, SLOT(changeAbility()));
        int index = ui->ability->findText(ability);
        if (index != -1) {
            ui->ability->setCurrentIndex(index);
        }
    }
}

void PokeLevelSettings::setPoke(PokeTeam *poke)
{
    m_poke = poke;
    fillAbilities();
    setGender();
    updateAll();
}

void PokeLevelSettings::updateAll()

{
    ui->level->setValue(poke().level());

    setGender();
    updateGender();
    setAbilities();
    updateAbility();

    if (poke().gen() <= 1) {
        for(int i = 0; i < ui->genderLine->count(); i++) {
            ui->genderLine->itemAt(i)->widget()->hide();
        }
        //ui->line_2->hide();
    } else {
        for(int i = 0; i < ui->genderLine->count(); i++) {
            ui->genderLine->itemAt(i)->widget()->show();
        }
        ui->line->show();
        //ui->line_2->show();
    }

    if (poke().gen() >= 3) {
        for(int i = 0; i < ui->abilityLine->count(); i++) {
            ui->abilityLine->itemAt(i)->widget()->show();
        }
    } else {
        for(int i = 0; i < ui->abilityLine->count(); i++) {
            ui->abilityLine->itemAt(i)->widget()->hide();
        }
    }
}

void PokeLevelSettings::changeAbility()
{
    if (poke().gen() < 3) {
        return;
    }

    int abilityToSet;
    if (!PokeEdit::hackMons) {
        if (m_abilities[1]->isChecked()) {
            abilityToSet = poke().abilities().ab(1);
        } else if (m_abilities[2]->isChecked()) {
            abilityToSet = poke().abilities().ab(2);
        } else {
            abilityToSet = poke().abilities().ab(0);
        }
    } else {
        abilityToSet = AbilityInfo::Number(ui->ability->currentText());
    }
    try {
        poke().setAbility(abilityToSet, PokeEdit::hackMons);
    } catch (const QString &s) {
        QMessageBox::information(NULL, tr("Invalid moveset"), s);
        for(int i = 0; i < 3; i++) {
            if(poke().ability() == poke().abilities().ab(i)) {
                m_abilities[i]->setChecked(true);
            } else {
                m_abilities[i]->setChecked(false);
            }
        }
    }
}

void PokeLevelSettings::setAbilities()
{
    if (!PokeEdit::hackMons) {
        for(int i = 0; i < 3; i++) {
            if(poke().abilities().ab(i) != 0 && poke().gen() >= 3 && (i == 0 || poke().abilities().ab(i) != poke().abilities().ab(0))) {
                m_abilities[i]->setVisible(true);
                m_abilities[i]->setText(AbilityInfo::Name(poke().abilities().ab(i)));
                // Hack to make the tooltip wrap
                QString tooltip = QString("<FONT>");
                tooltip += (AbilityInfo::EffectDesc(poke().abilities().ab(i)));
                tooltip += QString("</FONT>");
                m_abilities[i]->setToolTip(tooltip);

                if (poke().abilities().ab(i) == poke().ability()) {
                    m_abilities[i]->setChecked(true);
                }
            } else {
                 m_abilities[i]->setVisible(false);
            }
        }
    } else {
        ui->ability->setVisible(true);
        int index = ui->ability->findText(AbilityInfo::Name(poke().ability()));
        if (index != -1) {
            ui->ability->setCurrentIndex(index);
        }
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

void PokeLevelSettings::updateAbility()
{
    if (poke().gen() < 3) {
        return;
    }
    if (!PokeEdit::hackMons) {
        if (poke().ability() == poke().abilities().ab(0)) {
            m_abilities[0]->setChecked(true);
        } else if (poke().ability() == poke().abilities().ab(1)) {
            m_abilities[1]->setChecked(true);
        } else if (poke().ability() == poke().abilities().ab(2)) {
            m_abilities[2]->setChecked(true);
        }
    } else {
        ui->ability->setVisible(true);
        int index = ui->ability->findText(AbilityInfo::Name(poke().ability()));
        if (index != -1) {
            ui->ability->setCurrentIndex(index);
        }
    }
}

void PokeLevelSettings::changeLevel(int newLevel)
{
    poke().level() = newLevel;
    emit levelUpdated();
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
