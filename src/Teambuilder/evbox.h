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

class EvBox : public QWidget
{
    Q_OBJECT

public:
    explicit EvBox(QWidget *parent = 0);
    void setPoke(PokeTeam *poke);
    void updateAll();

    ~EvBox();

private:
    Ui::EvBox *ui;
    QSlider *m_sliders[6];
    QLabel *m_stats[6];
    QLabel *m_descs[6];
    QLineEdit *m_evs[6];

    PokeTeam *m_poke;
    PokeTeam &poke() {return *m_poke;}
    const PokeTeam &poke() const {return *m_poke;}

    void updateMain();
    void updateEV(int stat);
};

#endif // EVBOX_H
