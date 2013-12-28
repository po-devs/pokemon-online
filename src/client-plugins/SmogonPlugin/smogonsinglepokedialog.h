#ifndef SMOGONSINGLEPOKEDIALOG_H
#define SMOGONSINGLEPOKEDIALOG_H

#include <QDialog>

class PokeTeam;
class PokemonTab;

namespace Ui {
class SmogonSinglePokeDialog;
}

class SmogonSinglePokeDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SmogonSinglePokeDialog(QWidget *parent = 0);
    ~SmogonSinglePokeDialog();

    void setPokemon(PokeTeam *p);
    PokemonTab *getPokemonTab();
private:
    Ui::SmogonSinglePokeDialog *ui;
};

#endif // SMOGONSINGLEPOKEDIALOG_H
