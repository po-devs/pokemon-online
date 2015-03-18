#include "playerinfolistmodel.h"
#include "libraries/PokemonInfo/networkstructs.h"
PlayerInfoListModel::PlayerInfoListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

void PlayerInfoListModel::add(PlayerInfo pi)
{
    emit beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_playerInfoList.append(pi);
    qDebug() << m_playerInfoList.size();
    emit endInsertRows();
}

int PlayerInfoListModel::rowCount(const QModelIndex &parent) const
{
    return m_playerInfoList.size();
}

QVariant PlayerInfoListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    switch (role) {
    case RoleName:
        return m_playerInfoList[index.row()].name;
    case RolePlayerId:
        return m_playerInfoList[index.row()].id;
    }
    return QVariant();
}

QHash<int, QByteArray> PlayerInfoListModel::roleNames() const
{
    QHash<int, QByteArray> r;
    r[RoleName] = "name";
    r[RolePlayerId] = "playerId";
    return r;
}
