#include <QMessageBox>
#include <PokemonInfo/pokemonstructs.h>
#include <PokemonInfo/pokemoninfo.h>
#include <TeambuilderLibrary/theme.h>
#include "ivbox.h"
#include "ui_ivbox.h"

IvBox::IvBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IvBox)
{
    ui->setupUi(this);

    QLabel *statsLabels[6] = {ui->hpivlabel, ui->attivlabel, ui->defivlabel, ui->spattivlabel, ui->spdefivlabel, ui->speedivlabel};
    QSpinBox *ivChangers[6] = {ui->hpivspin, ui->attivspin, ui->defivspin, ui->spattivspin, ui->spdefivspin, ui->speedivspin};

    memcpy(m_statslabel, statsLabels, sizeof(statsLabels));
    memcpy(m_ivchangers, ivChangers, sizeof(ivChangers));

#define setNums(var) for(int i = 0; i < 6; i++) { var[i]->setProperty("ivstat", i); }
    setNums(m_ivchangers);
#undef setNums

    for(int i = 0; i < 6; i++) {
        connect(m_ivchangers[i], SIGNAL(valueChanged(int)), this, SLOT(changeIV(int)));
    }

    for(int i = 1; i < Type::Curse-1; i++) {
        ui->hiddenPowerType->addItem(TypeInfo::Name(i));
    }

    connect(ui->hiddenPowerType, SIGNAL(activated(int)), this, SLOT(changeHiddenPower(int)));
    connect(ui->hpchoice, SIGNAL(cellActivated(int,int)), SLOT(changeHPSelection(int)));
}

IvBox::~IvBox()
{
    delete ui;
}

void IvBox::setPoke(PokeTeam *poke)
{
    m_poke = poke;
}

// We need to initialize in a separate function since setPoke() is obviously called after IvBox::IvBox
// So if we do this things inside that function, a nicely crash.

void IvBox::updateAll()
{
    updateIVs();
    updateStats();

    updateHiddenPower();

    if (poke().gen() <= 2) {
        ui->hpivspin->setDisabled(true);
        ui->spdefivspin->setDisabled(true);
        ui->hpchoice->hide();

        for (int i = 0; i < 6; i++) {
            m_ivchangers[i]->setRange(0, 15);
        }
    } else {
        ui->hpivspin->setDisabled(false);
        ui->spdefivspin->setDisabled(false);
        if (poke().gen() > 6) {
            ui->hpchoice->hide();
        } else {
            ui->hpchoice->show();
        }

        for (int i = 0; i < 6; i++) {
            m_ivchangers[i]->setRange(0, 31);
        }
    }

    if (poke().gen() <= 1) {
        ui->hiddenPowerPower->setVisible(false);
        ui->hiddenPowerTitle->setVisible(false);
        ui->hiddenPowerType->setVisible(false);

        ui->spdefivdesc->hide();
        ui->spdefivlabel->hide();
        ui->spdefivspin->hide();
        ui->spattivdesc->setText(tr("Special: "));
    } else {
        if (poke().gen() > 2) {
            ui->finalStats->setTitle(tr("&IVs"));
        }
        ui->hiddenPowerPower->setVisible(poke().gen() <= 5);
        ui->hiddenPowerTitle->setVisible(true);
        ui->hiddenPowerType->setVisible(true);

        ui->spdefivdesc->setVisible(true);
        ui->spdefivlabel->setVisible(true);
        ui->spdefivspin->setVisible(true);
        ui->spattivdesc->setText(tr("Sp. Atk: "));
    }
}

void IvBox::updateStats()
{
    blockSignals(true);
    for(int i = 0; i < 6; i++) {
        updateStat(i);
    }
    blockSignals(false);
    emit statsUpdated();
}

void IvBox::updateStat(int stat)
{
    if(poke().natureBoost(stat) != 0) {
        QColor themeColor;
        switch(poke().natureBoost(stat)) {
        case -1:  themeColor = Theme::Color("Teambuilder/statHindered"); break;
        case 1: themeColor = Theme::Color("Teambuilder/statRaised"); break;
        }
        m_statslabel[stat]->setText(toColor(QString::number(poke().stat(stat)), themeColor));
    } else {
        m_statslabel[stat]->setNum(poke().stat(stat));
    }

    emit statsUpdated();
}

void IvBox::changeIV(int newValue)
{
    int stat = sender()->property("ivstat").toInt();

    if(!poke().illegal() && poke().gen().num == 6 && poke().num().pokenum >= Pokemon::Xerneas) {
        int numFlawless = 6;
        for (int i = 0; i < 6; i++) {
            if(i == stat) {
                if(newValue < 31) {
                    numFlawless--;
                }
            } else if(poke().DV(i) < 31) {
                numFlawless--;
            }
        }
        if(numFlawless < 3) {
            newValue = 31;
            QMessageBox::information(NULL, tr("Invalid IVs"), tr("%1 must have at least 3 flawless IVs.").arg(PokemonInfo::Name(poke().num().pokenum)));
            m_ivchangers[stat]->setValue(31);
        }
    }


    if(poke().DV(stat) != newValue) {
        poke().setDV(stat, newValue);
        updateIVs();
        updateHiddenPower();
    }
}

void IvBox::updateIVs()
{
    for(int i = 0; i < 6; i++) {
        updateIV(i);
    }

    if (poke().gen() <= 2) {
        emit genderUpdated();
        emit shinyUpdated();
    }
}

void IvBox::updateIV(int stat)
{
    m_ivchangers[stat]->setValue(poke().DV(stat));
    updateStat(stat);
}

void IvBox::updateHiddenPower()
{
    if (poke().gen() > 6 && hasValidHiddenPower()) {
        ui->hiddenPowerType->setCurrentIndex(poke().hiddenPower() - 1);
        return;
    }

    ui->hiddenPowerPower->setText(tr("(%1 pow)").arg(QString::number(calculateHiddenPowerPower())));
    int type = calculateHiddenPowerType();
    poke().setHiddenPower(type);
    ui->hiddenPowerType->setCurrentIndex(type - 1);
    updateHiddenPowerSelection();
}

void IvBox::updateHiddenPowerSelection()
{
    QList<QStringList> possibilities = HiddenPowerInfo::PossibilitiesForType(calculateHiddenPowerType(), poke().gen());

    while (ui->hpchoice->rowCount() > 0) {
        ui->hpchoice->removeRow(0);
    }

    foreach(QStringList s, possibilities) {
        int c = ui->hpchoice->rowCount();
        ui->hpchoice->insertRow(c);
        if(!poke().illegal() && poke().gen().num == 6 && poke().num().pokenum >= Pokemon::Xerneas) {
            int numFlawless = 0;
            for (int i = 0; i < 6; i++) {
                if(s[i].toInt() == 31) {
                    numFlawless++;
                }
            }
            if(numFlawless < 3) {
                ui->hpchoice->removeRow(c);
            }
        }
        if(ui->hpchoice->rowCount() == c + 1) {
            for (int i = 0; i < 6; i++) {
                ui->hpchoice->setItem(c, i, new QTableWidgetItem(s[i]));
            }
        }
    }
}

int IvBox::calculateHiddenPowerPower()
{
    return HiddenPowerInfo::Power(poke().gen().num, poke().DV(0), poke().DV(1), poke().DV(2), poke().DV(3), poke().DV(4), poke().DV(5));
}

int IvBox::calculateHiddenPowerType()
{
    return HiddenPowerInfo::Type(poke().gen().num, poke().DV(0), poke().DV(1), poke().DV(2), poke().DV(3), poke().DV(4), poke().DV(5));
}

void IvBox::changeHiddenPower(int newType)
{
    newType += 1;
    if (newType == calculateHiddenPowerType()) {
        return;
    }

    if (poke().gen() > 6) {
        poke().setHiddenPower(newType);
        if (!hasValidHiddenPower()) {
            QMessageBox::information(NULL, tr("Invalid Hidden Power"), tr("Cannot have Hidden Power type %1 with those IVs.").arg(poke().hiddenPower()));
            int type = calculateHiddenPowerType();
            ui->hiddenPowerType->setCurrentIndex(type - 1);
            poke().setHiddenPower(type);
        }
        return;
    }

    if (poke().gen() > 2) {
        QList<QStringList> possibilities = HiddenPowerInfo::PossibilitiesForType(newType, poke().gen());

        if(!poke().illegal() && poke().gen().num == 6 && poke().num().pokenum >= Pokemon::Xerneas) {
            foreach(QStringList s, possibilities) {
                int numFlawless = 0;
                for (int i = 0; i < 6; i++) {
                    if(s[i].toInt() == 31) {
                        numFlawless++;
                    }
                }
                if(numFlawless < 3) {
                    possibilities.removeAt(possibilities.indexOf(s));
                }
            }
        }

        if (possibilities.size() == 0) {
            QMessageBox::information(NULL, tr("Impossible type"), tr("%1 can't have Hidden Power type %2.").arg(PokemonInfo::Name(poke().num().pokenum), ui->hiddenPowerType->currentText()));
            int type = calculateHiddenPowerType();
            ui->hiddenPowerType->setCurrentIndex(type - 1);
            return;
        }

        QStringList possibility = possibilities.front();

        for (int i = 0; i < std::max(6, possibility.size()); i++) {
            poke().setDV(i, possibility[i].toInt());
        }
    } else {
        QPair<quint8,quint8> dvs = HiddenPowerInfo::AttDefDVsForGen2(newType);
        poke().setDV(Attack, dvs.first);
        poke().setDV(Defense, dvs.second);
    }

    updateIVs();
    updateHiddenPower();
}

void IvBox::changeHPSelection(int row)
{
    for (int i = 0; i < 6; i++) {
        poke().setDV(i, ui->hpchoice->item(row, i)->text().toInt());
    }

    updateIVs();
}

bool IvBox::hasValidHiddenPower()
{
    if (poke().gen() > 6) {
        int minPossible = 0;
        int maxPossible = 0;
        for (int i = 0; i < 6; i++) {
            //Speed comes before sp.atk and sp.def
            int b = i == 5 ? 3 : (i > 2 ? i+1 : i);

            minPossible += poke().DV(i) == 31 ? 0 : (poke().DV(i) % 2) << b;
            maxPossible += poke().DV(i) == 31 ? 1 << b : (poke().DV(i) % 2) << b;
        }
        minPossible = (minPossible*15)/63 + 1;
        maxPossible = (maxPossible*15)/63 + 1;
        if (maxPossible < poke().hiddenPower() || poke().hiddenPower() < minPossible) {
            return false;
        }
    }
    return true;
}
