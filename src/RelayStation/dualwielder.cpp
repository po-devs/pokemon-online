#include <QColor>
#include "../QtWebsocket/QWsSocket.h"
#include "../Shared/networkcommands.h"
#include "../Teambuilder/network.h"

#include "dualwielder.h"

DualWielder::DualWielder(QObject *parent) : QObject(parent), web(NULL), network(NULL)
{
    /* No need to waste network bandwith */
    jserial.setIndentMode(QJson::IndentCompact);
}

DualWielder::~DualWielder()
{
    if (network) {
        network->close();
    }
    if (web) {
        web->close(QWsSocket::CloseGoingAway);
    }

    qDebug() << "Closing connection with IP " << ip();
}

void DualWielder::init(QWsSocket *sock, QString host)
{
    web = sock;
    mIp = web->ip();

    qDebug() << "Connection accepted, IP " + ip();

    sock->write("defaultserver|"+ host);

    connect(sock, SIGNAL(frameReceived(QString)), SLOT(readWebSocket(QString)));
    connect(sock, SIGNAL(disconnected()), SLOT(webSocketDisconnected()));
    connect(sock, SIGNAL(disconnected()), sock, SLOT(deleteLater()));
}

QString DualWielder::ip() const
{
    return mIp;
}

void DualWielder::readSocket(const QByteArray &commandline)
{
    //No point in dealing with commands if the websocket is closed
    if (!web) {
        return;
    }

    DataStream in (commandline);
    uchar command;

    in >> command;

    switch (command) {
    case ZipCommand: {
        quint8 contentType;

        in >> contentType;

        if (contentType > 1) {
            return;
        }

        int length = commandline.length()-1-1;

        if (length <= 0) {
            return;
        }
        char data[length];

        in.readRawData(data, length);

        QByteArray info = qUncompress((uchar*)data, length);

        if (contentType == 0) {
            if (info.length() == 0) {
                return;
            }

            readSocket(info);
        } else { //contentType == 1
            DataStream in2(info);
            QByteArray packet;
            do {
                in2 >> packet;

                if (packet.length() > 0) {
                    readSocket(packet);
                }
            } while (packet.length() > 0);
        }
        break;
    }
    case SendMessage: {
        Flags network,data;

        in >> network >> data;
        int channel = -1;

        if (network[0]) {
            in >> channel;
        }
        QString message;

        in >> message;

        QVariantMap map;

        map.insert("channel", channel);
        map.insert("html", bool(data[0]));
        map.insert("message", message);

        web->write("chat|"+QString::fromUtf8(jserial.serialize(map)));

        break;
    }
    case KeepAlive: {
        quint16 ping;
        in >> ping;
        notify(KeepAlive, ping);
        break;
    }
//    case PlayersList: {
//        if (!registry_socket) {
//            PlayerInfo p;
//            while (!in.atEnd()) {
//                in >> p;
//                emit playerReceived(p);
//            }
//            break;
//        } else {
//            // Registry socket;
//            ServerInfo s;
//            in >> s;

//            emit serverReceived(s);
//        }
//    }
//    case Login: {
//        Flags network;
//        in >> network;

//        if (network[0]) {
//            QByteArray reconnectPass;
//            in >> reconnectPass;
//            emit reconnectPassGiven(reconnectPass);
//        }
//        PlayerInfo p;
//        in >> p;
//        QStringList tiers;
//        in >> tiers;
//        emit playerLogin(p, tiers);
//        break;
//    }
//    case Logout: {
//        qint32 id;
//        in >> id;
//        emit playerLogout(id);
//        break;
//    }
//    case ChallengeStuff: {
//        ChallengeInfo c;
//        in >> c;
//        emit challengeStuff(c);
//        break;
//    }
//    case EngageBattle: {
//        Flags network;
//        quint8 mode;
//        qint32 battleid, id1, id2;
//        in >> battleid >> network >> mode >> id1 >> id2;

//        if (network[0]) {
//            /* This is a battle we take part in */
//            TeamBattle team;
//            BattleConfiguration conf;
//            if (version < ProtocolVersion(1,0)) {
//                conf.oldDeserialize(in);
//            } else {
//                in >> conf;
//            }
//            in >> team;
//            if (network[1]) {
//                in >> team.items;
//            }
//            emit battleStarted(battleid, id1, id2, team, conf);
//        } else {
//            /* this is a battle of strangers */
//            emit battleStarted(battleid, id1, id2);
//        }
//        break;
//    }
//    case BattleFinished: {
//        qint8 desc, mode;
//        qint32 battleid;
//        qint32 id1, id2;
//        in >> battleid >> desc >> mode >> id1 >> id2;
//        emit battleFinished(battleid, desc, id1, id2);
//        break;
//    }
//    case BattleMessage: {
//        qint32 battleid;
//        QByteArray command;
//        in >> battleid >> command;

//        emit battleMessage(battleid, command);
//        break;
//    }
//    case AskForPass: {
//        QByteArray salt;
//        in >> salt;

//        if (salt.length() < 6 || strlen((" " + salt).data()) < 7)
//            emit protocolError(5080, tr("The server requires insecure authentication."));
//        emit passRequired(salt);
//        break;
//    }
//    case Register: {
//        emit notRegistered(true);
//        break;
//    }
//    case PlayerKick: {
//        qint32 p,src;
//        in >> p >> src;
//        emit playerKicked(p,src);
//        break;
//    }
//    case PlayerBan: {
//        qint32 p,src;
//        in >> p >> src;
//        emit playerBanned(p,src);
//        break;
//    }
//    case PlayerTBan: {
//        qint32 p, src, time;
//        in >> p >> src >> time;
//        emit playerTempBanned(p, src, time);
//    }
//    case SendTeam: {
//        Flags network;
//        in >> network;
//        if (network[0]) {
//            QString name;
//            in >> name;
//        }
//        if (network[1]) {
//            QStringList tiers;
//            in >> tiers;
//            emit teamApproved(tiers);
//        }
//        break;
//    }
//    case SendPM: {
//        qint32 idsrc;
//        QString mess;
//        in >> idsrc >> mess;
//        emit PMReceived(idsrc, mess);
//        break;
//    }
//    case GetUserInfo: {
//        UserInfo ui;
//        in >> ui;
//        emit userInfoReceived(ui);
//        break;
//    }
//    case GetUserAlias: {
//        QString s;
//        in >> s;
//        emit userAliasReceived(s);
//        break;
//    }
//    case GetBanList: {
//        QString s, i;
//        quint32 dt;
//        in >> s >> i >> dt;
//        emit banListReceived(s,i,QDateTime::fromTime_t(dt));
//        break;
//    }
//    case OptionsChange: {
//        qint32 id;
//        Flags f;
//        in >> id >> f;
//        emit awayChanged(id, f[1]);
//        emit ladderChanged(id, f[0]);
//        break;
//    }
//    case SpectateBattle: {
//        Flags f;
//        qint32 battleId;
//        in >> f >> battleId;

//        if (f[0]) {
//            BattleConfiguration conf;
//            if (version < ProtocolVersion(1,0)) {
//                conf.oldDeserialize(in);
//            } else {
//                in >> conf;
//            }
//            emit spectatedBattle(battleId, conf);
//        } else {
//            emit spectatingBattleFinished(battleId);
//        }
//        break;
//    }
//    case SpectatingBattleMessage: {
//        qint32 battleId;
//        in >> battleId;
//        /* Such a headache, it really looks like wasting ressources */
//        char *buf;
//        uint len;
//        in.readBytes(buf, len);
//        QByteArray command(buf, len);
//        delete [] buf;
//        emit spectatingBattleMessage(battleId, command);
//        break;
//    }
    case VersionControl_: {
        ProtocolVersion server, feature, minor, major;
        Flags f;
        QString serverName;

        in >> server >> f >> feature >> minor >> major >> serverName;

        web->write("servername|"+serverName);

//        ProtocolVersion version;

//        if (version < major) {
//            emit versionDiff(major, -3);
//        } else if (version < minor) {
//            emit versionDiff(minor, -2);
//        } else if (version < feature) {
//            emit versionDiff(feature, -1);
//        } else if (version < server) {
//            emit versionDiff(server, 0);
//        }

        this->version = server;

        break;
    }
//    case TierSelection: {
//        QByteArray tierList;
//        in >> tierList;
//        emit tierListReceived(tierList);
//        break;
//    }
//    case ShowRankings: {
//        bool starting;
//        in >> starting;
//        if (starting)
//        {
//            qint32 startingPage, startingRank, total;
//            in >> startingPage >> startingRank >> total;
//            emit rankingStarted(startingPage, startingRank, total);
//        } else {
//            QString name;
//            qint32 points;
//            in >> name >> points;
//            emit rankingReceived(name, points);
//        }
//        break;
//    }
    case Announcement: {
        QString announcement;
        in >> announcement;
        web->write("announcement|"+announcement);
        break;
    }
//    case ChannelsList: {
//        QHash<qint32, QString> channels;
//        in >> channels;
//        emit channelsListReceived(channels);
//        break;
//    }
//    case ChannelPlayers: {
//        QVector<qint32> ids;
//        qint32 chanid;
//        in >> chanid >> ids;

//        emit channelPlayers(chanid, ids);
//        break;
//    }
//    case AddChannel: {
//        QString name;
//        qint32 id;
//        in >> name >> id;

//        emit addChannel(name, id);
//        break;
//    }
//    case RemoveChannel: {
//        qint32 id;
//        in >> id;

//        emit removeChannel(id);
//        break;
//    }
//    case ChanNameChange: {
//        qint32 id;
//        QString name;
//        in >> id >> name;

//        emit channelNameChanged(id, name);
//        break;
//    }
//    case SpecialPass: {
//        QSettings s;
//        s.beginGroup("password");
//        s.setValue(QCryptographicHash::hash(QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5), QCryptographicHash::Sha1), QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5));
//        s.endGroup();
//        break;
//    }
//    case ServerPass: {
//        QByteArray salt;
//        in >> salt;
//        emit serverPassRequired(salt);
//        break;
//    }
//    /* Non-standard command, shouldn't exist */
//    case ServerInfoChanged: {
//        Flags f;
//        in >> f;
//        if (f[0]) {
//            QString s;
//            in >> s;
//            emit serverNameReceived(s);
//        }
//        break;
//    }
//    case NetworkCli::Reconnect: {
//        bool success;
//        in >> success;

//        if (success) {
//            emit reconnectSuccess();
//        } else {
//            quint8 reason;
//            in >> reason;
//            emit reconnectFailure(reason);
//        }
//        break;
//    }
    default: {
        //web->write(QString("msg|" "Protocol error: unknown command received -- maybe an update for the program is available"));
    }
    }
}

void DualWielder::readWebSocket(const QString &frame)
{
    /* Shouldn't happen, but idk how websockets work */
    if (!web) {
        return;
    }

    QString command = frame.section("|",0,0);
    QString data = frame.section("|", 1);

    if (!network) {
        if (command == "connect") {
            qDebug() << "Connecting websocket to server at " << data;
            network = new Network(data.section(":", 0, -2), data.section(":", -1).toInt());

            connect(network, SIGNAL(connected()), SLOT(socketConnected()));
            connect(network, SIGNAL(disconnected()), SLOT(socketDisconnected()));
            connect(network, SIGNAL(disconnected()), network, SLOT(deleteLater()));
            connect(network, SIGNAL(error(QAbstractSocket::SocketError)), network, SLOT(deleteLater()));
            connect(network, SIGNAL(isFull(QByteArray)), SLOT(readSocket(QByteArray)));
            connect(this, SIGNAL(sendCommand(QByteArray)), network, SLOT(send(QByteArray)));
        } else {
            web->write(QString("error|You need to choose a server to connect to."));
        }
    } else {
        if (command == "login") {
            QVariantMap params = jparser.parse(data.toUtf8()).toMap();

            QByteArray tosend;
            DataStream out(&tosend, QIODevice::WriteOnly);

            Flags network;
            network.setFlags((1 << LoginCommand::HasClientType) | (params.contains("version") << LoginCommand::HasVersionNumber));

            if (!params.value("default").isNull()) {
                network.setFlag(LoginCommand::HasDefaultChannel, true);
            }

            if (params.value("autojoin").toList().size() > 0) {
                network.setFlag(LoginCommand::HasAdditionalChannels, true);
            }

            if (params.value("color").value<QColor>().isValid()) {
                network.setFlag(LoginCommand::HasColor, true);
            }
            //    HasClientType,
            //    HasVersionNumber,
            //    HasReconnect,
            //    HasDefaultChannel,
            //    HasAdditionalChannels,
            //    HasColor,
            //    HasTrainerInfo,
            //    HasTeams,
            //    HasEventSpecification,
            //    HasPluginList

            Flags data;
            data.setFlag(PlayerFlags::SupportsZipCompression, true);
            data.setFlag(PlayerFlags::LadderEnabled, params.value("ladder", true).toBool());
            data.setFlag(PlayerFlags::Idle, params.value("idle", false).toBool());
            //                  SupportsZipCompression,
            //                  ShowTeam,
            //                  LadderEnabled,
            //                  Idle,
            //                  IdsWithMessage

            out << uchar(Login) << ProtocolVersion() << network << "webclient";

            if (params.contains("version")) {
                out << quint16(params.value("version").toInt());
            }
            out << params.value("name", QString("guest%1").arg(rand())).toString() << data;

            if (!params.value("default").isNull()) {
                out << params.value("default").toString();
            }

            if (params.value("autojoin").toList().size() > 0) {
                out << params.value("autojoin").toStringList();
            }

            if (params.value("color").value<QColor>().isValid()) {
                out << params.value("color").value<QColor>();
            }

            emit sendCommand(tosend);
        } else if (command == "chat") {
            QVariantMap params = jparser.parse(data.toUtf8()).toMap();

            if (params.count() == 0) {
                notify(SendMessage, Flags(1), Flags(0), qint32(0), data);
            } else {
                notify(SendMessage, Flags(1), Flags(0), qint32(params.value("channel").toInt()), params.value("message").toString());
            }
        }
    }
}

void DualWielder::socketConnected()
{
    if (web) {
        web->write(QString("connected|"));
    }
}

void DualWielder::socketDisconnected()
{
    network = NULL;
    if (web) {
        web->write(QString("disconnected|"));
        web->close(QWsSocket::CloseNormal, "The Pokemon Online server closed the connection");
        web = NULL;
    }

    deleteLater();
}

void DualWielder::webSocketDisconnected()
{
    web = NULL;
    if (network) {
        /* Gives the server the curtesy to know that there will be no reconnection */
        notify(Logout);
        network->close();
        network = NULL;
    }

    deleteLater();
}
