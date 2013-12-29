#ifndef POKETABLEMODEL_H
#define POKETABLEMODEL_H

#include <QAbstractTableModel>
#include <PokemonInfo/pokemon.h>
#include <PokemonInfo/geninfo.h>

class PokeTableModel : public QAbstractTableModel {
public:
    PokeTableModel(Pokemon::gen gen=Pokemon::gen(), QObject *parent = NULL);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setGen(Pokemon::gen gen);
private:
    Pokemon::gen gen;
};

#endif // POKETABLEMODEL_H
