#ifndef POKEMONEDITORDIALOG_H
#define POKEMONEDITORDIALOG_H

#include <QDialog>

#include "PokemonInfo/pokemon.h"

class QAbstractItemModel;
class MainEngineInterface;
class PokeMovesModel;

namespace Ui {
class PokemonEditorDialog;
}

class PokemonEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PokemonEditorDialog(MainEngineInterface *client);
    ~PokemonEditorDialog();

public slots:
    void setPokemon(Pokemon::uniqueId id);
private slots:
    void on_pokemonFrame_clicked();
private:
    Ui::PokemonEditorDialog *ui;
    QAbstractItemModel *pokeModel;
    PokeMovesModel *movesModel;

    Pokemon::uniqueId current;
};

#endif // POKEMONEDITORDIALOG_H
