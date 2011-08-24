#ifndef EVMANAGER_H
#define EVMANAGER_H

#include <QWidget>

class PokeTeam;
class QLabel;
class QSlider;
class QLineEdit;
class QImageButtonLR;

/* Manages the EV bars, inside the TB_PokemonBody */
class TB_EVManager : public QWidget
{
    Q_OBJECT
public:
    TB_EVManager(PokeTeam *poke=NULL);

    /* Nature selectors */

    int myStatUp;
    int myStatDown;

    void setPokemon(PokeTeam *poke);
    void updateEVs();
    void updateEV(int stat);
    void updateMain();
    void changeGen(int);
public slots:
    void changeEV(int newvalue);
    void changeEV(const QString &newvalue);
    void checkNButtonR();
    void updateNatureButtons();
    void checkNButtonL();
signals:
    void EVChanged(int stat);
    void natureChanged(int up, int down);
private:
    QSlider *m_sliders[6];
    QLabel *m_stats[6];
    QLabel *m_descs[6];
    QLineEdit *m_evs[6];
    QSlider *m_mainSlider;
    PokeTeam *m_poke;
    QLabel * m_mainLabel;
    QImageButtonLR *natureButtons[5];

    PokeTeam *poke();
    const PokeTeam *poke() const;
    QSlider *slider(int stat);
    const QSlider *slider(int stat) const;
    QLineEdit *evLabel(int stat);
    const QLineEdit *evLabel(int stat) const;
    QLabel *statLabel(int stat);
    /* the reverse of slider(int), evlabel(int) */
    int stat(QObject *sender) const;
    int gen() const;
};

#endif // EVMANAGER_H
