#ifndef POKEMONEDITORDIALOG_H
#define POKEMONEDITORDIALOG_H

#include <QDialog>

#include <PokemonInfo/pokemon.h>
#include <PokemonInfo/geninfo.h>

class QAbstractItemModel;
class MainEngineInterface;
class PokeMovesModel;
class QModelIndex;

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
    void addMove();
    void accept();
protected:
    void saveChanges();
    void updateFiles(Pokemon::uniqueId num, QHash<int, QSet<int> > moves, Pokemon::gen gen = Pokemon::gen());
private slots:
    void on_pokemonFrame_clicked();
    void moveEntered(const QModelIndex&);
    void removeRow(int row);
private:
    Ui::PokemonEditorDialog *ui;
    QAbstractItemModel *pokeModel;
    PokeMovesModel *movesModel;

    Pokemon::uniqueId current;
};

#endif // POKEMONEDITORDIALOG_H
