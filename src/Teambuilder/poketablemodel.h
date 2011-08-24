#ifndef POKETABLEMODEL_H
#define POKETABLEMODEL_H

#include <QAbstractTableModel>

class PokeTableModel : public QAbstractTableModel {
public:
    PokeTableModel(int gen=5, QObject *parent = NULL);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    void setGen(int gen);
private:
    int gen;
};

#endif // POKETABLEMODEL_H
