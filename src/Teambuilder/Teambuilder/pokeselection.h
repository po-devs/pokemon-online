#ifndef POKESELECTION_H
#define POKESELECTION_H

#include <QWidget>
#include "../PokemonInfo/pokemonstructs.h"

namespace Ui {
class PokeSelection;
}

class AdvancedSearch;
class QAbstractItemModel;

class PokeSelection : public QWidget
{
    Q_OBJECT
    
public:
    explicit PokeSelection(Pokemon::uniqueId pokemon, QAbstractItemModel *pokemonModel);
    ~PokeSelection();
    void show();
signals:
    void pokemonChosen(Pokemon::uniqueId);
private slots:
    void updateTypes();
    void updateSprite();
    void setNum(const Pokemon::uniqueId&);
    void setPokemon(const QModelIndex &);
    void changeForme(int);
    void finish();
    void toggleSearchWindow();
private:
    Ui::PokeSelection *ui;

    Pokemon::uniqueId num() const {
        return m_num;
    }

    Pokemon::gen getGen();

    Pokemon::uniqueId m_num;

    //ui stuff
    AdvancedSearch *search;
    int oldwidth, newwidth, oldx;
};

#endif // POKESELECTION_H
