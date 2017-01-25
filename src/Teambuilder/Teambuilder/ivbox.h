#ifndef IVBOX_H
#define IVBOX_H

#include <QWidget>

namespace Ui {
    class IvBox;
}

class PokeTeam;
class QLabel;
class QSpinBox;
class QRadioButton;

class IvBox : public QWidget
{
    Q_OBJECT
    
public:
    explicit IvBox(QWidget *parent = 0);
    void setPoke(PokeTeam *poke);
    ~IvBox();

public slots:
    void updateAll();

    void updateStats();
    void updateStat(int stat);
    void changeIV(int newValue);
    void changeHiddenPower(int newType);
    void changeHPSelection(int row);
signals:
    void statsUpdated();
    /* in gen 2, IVs determine shiny & gender */
    void genderUpdated();
    void shinyUpdated();
private:
    Ui::IvBox *ui;

    PokeTeam *m_poke;
    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void updateIVs();
    void updateIV(int stat);
    void updateHiddenPower();
    void updateHiddenPowerSelection();
    int calculateHiddenPowerPower();
    int calculateHiddenPowerType();

    QLabel *m_statslabel[6];
    QSpinBox *m_ivchangers[6];
};

#endif // IVBOX_H
