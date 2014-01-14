#ifndef EVBOX_H
#define EVBOX_H

#include <QWidget>

namespace Ui {
    class EvBox;
}

class PokeTeam;
class QSlider;
class QLabel;
class QLineEdit;
class QImageButtonLR;

class EvBox : public QWidget
{
    Q_OBJECT

public:
    explicit EvBox(QWidget *parent = 0);
    void setPoke(PokeTeam *poke);
    void updateAll();

    ~EvBox();

signals:
    void natureChanged(int);
    void natureBoostChanged();

private slots:
    void changeEV(const QString &newValue);
    void changeEV(int newValue);
    void changeToPlusBoost();
    void changeToMinusBoost();

public slots:
    void updateEVs();

private:
    Ui::EvBox *ui;
    QSlider *m_sliders[6];
    QLabel *m_stats[6];
    QLabel *m_descs[6];
    QLineEdit *m_evs[6];
    QImageButtonLR *m_boosts[6];

    PokeTeam *m_poke;
    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void updateMain();
    void updateEV(int stat);
    void updateNatureButtons();
};

#endif // EVBOX_H
