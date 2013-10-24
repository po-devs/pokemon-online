#ifndef POKEDEXPOKESELECTION_H
#define POKEDEXPOKESELECTION_H

#include <QWidget>

namespace Ui {
class PokedexPokeSelection;
}

class PokedexPokeSelection : public QWidget
{
    Q_OBJECT

public:
    explicit PokedexPokeSelection(QWidget *parent = 0);
    ~PokedexPokeSelection();

private:
    Ui::PokedexPokeSelection *ui;
};

#endif // POKEDEXPOKESELECTION_H
