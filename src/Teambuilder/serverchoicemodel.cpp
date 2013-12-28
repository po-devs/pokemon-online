#include "TeambuilderLibrary/theme.h"
#include "serverchoicemodel.h"
#include <QHostAddress>

int ServerChoiceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return infos.length();
}

int ServerChoiceModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 4;
}

QVariant ServerChoiceModel::data(const QModelIndex &index, int role) const
{
    int section = index.column();
    const ServerInfo &info = infos[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        if (section == Locked) {
            return QVariant();
        } else if (section == Name) {
            return info.name;
        } else if (section == Players) {
            return info.max ? tr("%1 / %2", "Players / Max").arg(info.num).arg(info.max) : QString::number(info.num);
        } else if (section == IP) {
            return info.ip + ":" + QString::number(info.port ? info.port : 5080);
        }
        break;
    case Qt::DecorationRole:
        if (section == Locked) {
            return info.passwordProtected ? Theme::lockedServer() : Theme::unlockedServer();
        }
        break;
    case Qt::ToolTipRole:
        if (section == Locked) {
            return info.passwordProtected ? tr("The server is password protected") : tr("The server is not password protected");
        }
        return info.desc;
    case Qt::SizeHintRole:
        if (section == Locked) {
            return Theme::lockedServer().size();
        }
        break;
    case SortRole :
        if (section == Locked) {
            return info.passwordProtected;
        } else if (section == Players) {
            return info.num;
        } else if (section == IP) {
            QHostAddress h = QHostAddress(info.ip);
            int protocol = h.protocol();

            return protocol == QAbstractSocket::IPv4Protocol ? QVariant(h.toIPv4Address()) : QVariant(info.ip);
        }
        return data(index);
        break;
    case DescRole:
        return info.desc;
    }

    return QVariant();
}

QVariant ServerChoiceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (role) {
    case Qt::DisplayRole:
        if (section == Locked) {
            return QVariant();
        } else if (section == Name){
            return tr("Server Name");
        } else if (section == Players) {
            return tr("Players / Max");
        } else {
            return tr("Advanced Connection", "Server List Header");
        }
        break;
    case Qt::ToolTipRole:
        if (section == Locked) {
            return tr("Whether or not the server requires a password to log in");
        } else if (section == Name)  {
            return tr("The name of the server");
        } else if (section == Players) {
            return tr("The number of players / maximum number of players in the server");
        } else {
            return tr("The advanced connection required to access the server when the registry is down");
        }
        break;
    case Qt::DecorationRole :
        if (section == Locked) {
            return Theme::unlockedLockedRegistry();
        }
        break;
    case Qt::SizeHintRole :
        if (section == Locked) {
            return Theme::unlockedLockedRegistry().size();
        }
        break;
#if QT_VERSION >= QT_VERSION_CHECK(4,8,0)
    case Qt::InitialSortOrderRole :
        if (section == Locked || section == Name) {
            return Qt::AscendingOrder;
        } else {
            return Qt::DescendingOrder;
        }
        break;
#endif
    };
    return QVariant();
}

void ServerChoiceModel::addServer(const ServerInfo &info)
{
    beginInsertRows(QModelIndex(), infos.count(), infos.count());
    infos.push_back(info);
    endInsertRows();
}

void ServerChoiceModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    infos.clear();
    endRemoveRows();
}
