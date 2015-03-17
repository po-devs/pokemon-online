#ifndef PLAYERINFOLISTMODEL_H
#define PLAYERINFOLISTMODEL_H

#include <QAbstractListModel>

#include "libraries/PokemonInfo/networkstructs.h"

class PlayerInfoListModel : public QAbstractListModel
{
    Q_OBJECT

    enum {
        RoleName
    };
public:
    explicit PlayerInfoListModel(QObject *parent = 0);
    void add(PlayerInfo pi);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QHash<int,QByteArray> roleNames() const;

signals:

public slots:

private:
    QList<PlayerInfo> m_playerInfoList;
};

#endif // PLAYERINFOLISTMODEL_H
