#ifndef DAMAGECALC_H
#define DAMAGECALC_H

#include <QComboBox>
#include <QWidget>
#include "./libraries/PokemonInfo/pokemoninfo.h"

namespace Ui {
class DamageCalc;
}

class DamageCalc : public QWidget
{
    Q_OBJECT

public:
    explicit DamageCalc(int gen = 6, QWidget *parent = 0);
    ~DamageCalc();

    void setupCalc(int gen = 6);

private slots:
    void updateGen();
    void updateMyPokeStats();
    void updateOPokeStats();
    void updateMoveInfo();
    void updateMyPokePic();
    void updateOPokePic();
    void updateMyPokeInfo();
    void updateOPokeInfo();
    void updateMyPoke();
    void updateOPoke();
    
    void calculate();
    

private:
    Ui::DamageCalc *ui;

    Pokemon::gen m_currentGen;
    
    int applymod(int d, int m);
    int chainmod(int m1, int m2);
    void showResult(int min, int max);
    void setText(QComboBox *c, const QString &t);

    QHash<int, QString> whash;
};

#endif // DAMAGECALC_H
