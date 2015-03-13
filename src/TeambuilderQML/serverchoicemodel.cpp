#include "serverchoicemodel.h"
#include <QHostAddress>
#include <QSettings>
#include <TeambuilderLibrary/theme.h>
#include "../Teambuilder/analyze.h"

namespace PokemonOnlineQML {

ServerChoiceModel::ServerChoiceModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    QSettings settings;
    registry_connection = new Analyzer(true);
    //connect(registry_connection, SIGNAL(connected()), SLOT(connected()));

    registry_connection->connectTo(
        settings.value("ServerChoice/RegistryServer", "registry.pokemon-online.eu").toString(),
        settings.value("ServerChoice/RegistryPort", 5090).toUInt()
    );
    registry_connection->setParent(this);
//    connect(registry_connection, SIGNAL(connectionError(int,QString)), SLOT(connectionError(int , QString)));
    //connect(registry_connection, SIGNAL(regAnnouncementReceived(QString)), ui->announcement, SLOT(setText(QString)));
    //connect(registry_connection, SIGNAL(regAnnouncementReceived(QString)), ui->announcement, SLOT(show()));
//    connect(&manager, SIGNAL(finished(QNetworkReply*)), SLOT(announcementReceived(QNetworkReply*)));
    connect(registry_connection, SIGNAL(serverReceived(ServerInfo)), this, SLOT(addServer(ServerInfo)));
//    connect(this, SIGNAL(clearList()), model, SLOT(clear()));
//    connect(registry_connection, SIGNAL(serverReceived(ServerInfo)), SLOT(serverAdded()));
}

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
    case NameRole:
        return info.name;
    case IpRole:
        return info.ip;
    case MaxRole:
        return info.max;
    case NumRole:
        return info.num;
    case PasswordProtectedRole:
        return info.passwordProtected;
    case PortRole:
        return info.port;
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

QHash<int, QByteArray> ServerChoiceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DescRole] = "desc";
    roles[NumRole] = "num";
    roles[IpRole] = "ip";
    roles[MaxRole] = "max";
    roles[PortRole] = "port";
    roles[PasswordProtectedRole] = "passwordProtected";
    return roles;
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

}
