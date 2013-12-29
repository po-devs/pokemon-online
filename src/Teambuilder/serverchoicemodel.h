#ifndef SERVERCHOICEMODEL_H
#define SERVERCHOICEMODEL_H

#include <QAbstractTableModel>

#include <PokemonInfo/networkstructs.h>

class ServerChoiceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    enum Columns {
        Locked,
        Name,
        Players,
        IP
    };

    enum Roles {
        SortRole = Qt::UserRole +1,
        DescRole
    };

public slots:
    void addServer(const ServerInfo &info);
    void clear();
private:
    QList<ServerInfo> infos;
};

#endif // SERVERCHOICEMODEL_H
