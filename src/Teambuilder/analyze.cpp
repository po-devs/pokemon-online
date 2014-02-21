/**
 * See network protocol here: http://wiki.pokemon-online.eu/view/Network_Protocol_v2
*/

#include "../Shared/config.h"
#include <Utilities/functions.h>
#include <PokemonInfo/battlestructs.h>
#include <PokemonInfo/teamholder.h>

#include "analyze.h"

using namespace NetworkCli;

Analyzer::Analyzer(bool reg_connection) : registry_socket(reg_connection), mysocket(new QTcpSocket()), commandCount(0)
{
    connect(&socket(), SIGNAL(connected()), SIGNAL(connected()));
    connect(&socket(), SIGNAL(connected()), this, SLOT(wasConnected()));
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));
    connect(&socket(), SIGNAL(isFull(QByteArray)), SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), SLOT(error()));

    /* Commands that will be redirected to channels */
    channelCommands << BattleList << JoinChannel << LeaveChannel << ChannelBattle;
}

void Analyzer::login(const TeamHolder &team, bool ladder, bool away, const QColor &color, const QString &defaultChannel, const QStringList &autoJoin)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);

    Flags network;
    network.setFlags( (1 << LoginCommand::HasClientType) | (1 << LoginCommand::HasVersionNumber) | (1 << LoginCommand::HasReconnect)
                      | (1 << LoginCommand::HasTrainerInfo) | (1 << LoginCommand::HasTeams));

    if (!defaultChannel.isEmpty()) {
        network.setFlag(LoginCommand::HasDefaultChannel, true);
    }

    if (autoJoin.length() > 0) {
        network.setFlag(LoginCommand::HasAdditionalChannels, true);
    }

    if (color.isValid()) {
        network.setFlag(LoginCommand::HasColor, true);
    }

    QString path = appDataPath("cookies") + "/" + QString::fromUtf8(sha_hash(socket().ip().toUtf8()).left(16));
    bool hasCookie = QFileInfo(path).exists();

    network.setFlag(LoginCommand::HasCookie, hasCookie);
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
    //    HasCookie

    Flags data;
    data.setFlag(PlayerFlags::SupportsZipCompression, true);
    data.setFlag(PlayerFlags::LadderEnabled, ladder);
    data.setFlag(PlayerFlags::Idle, away);
    //                  SupportsZipCompression,
    //                  LadderEnabled,
    //                  IdsWithMessage,
    //                  Idle

    out << uchar(Login) << ownVersion << network;

#ifdef OS
    out << QString(OS);
#else
#warning Unknown OS version to send. Update the code to add your version
    out << QString("unknown_OS");
#endif

    out << CLIENT_VERSION_NUMBER << team.profile().name() << data;

    /* Can reconnect even if the last 2 bytes of the IP are different */
    out << uchar(16);

    if(!defaultChannel.isEmpty()) {
        out << defaultChannel;
    }

    if (autoJoin.length() > 0) {
        out << autoJoin;
    }

    if (color.isValid()) {
        out << color;
    }

    out << team.profile().info();

    out << uchar(team.count());
    for (int i = 0; i < team.count(); i++) {
        out << team.team(i);
    }

    if (hasCookie) {
        QString cookie = QString::fromUtf8(getFileContent(path));
        out << cookie;
    }

    emit sendCommand(tosend);
}

void Analyzer::logout()
{
    if (socket().isConnected()) {
        notify(Logout);

        /* Waits for the writing to finish */
        connect(&socket(), SIGNAL(disconnected()), SLOT(deleteLater()));
        socket().disconnectFromHost();
    } else {
        deleteLater();
    }
}

void Analyzer::sendChallengeStuff(const ChallengeInfo &c)
{
    notify(ChallengeStuff, c);
}

void Analyzer::getUserInfo(const QString &name)
{
    notify(GetUserInfo, name);
}

void Analyzer::sendPM(int id, const QString &mess)
{
    notify(SendPM, qint32(id), mess);
}

void Analyzer::kick(int id)
{
    notify(PlayerKick, qint32(id));
}

void Analyzer::ban(int id)
{
    notify(PlayerBan, qint32(id));
}

void Analyzer::tempban(int id, int time)
{
    notify(PlayerTBan, qint32(id), qint32(time));
}

void Analyzer::sendChanMessage(int channelid, const QString &message)
{
    notify(SendChatMessage, Flags(1), Flags(0), qint32(channelid), message);
}

void Analyzer::sendTeam(const TeamHolder &team)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);

    out << quint8(SendTeam) << Flags(1 + (team.color().isValid() << 1) + (1 << 2) + (1 << 3)) ;
    out << team.name();
    if (team.color().isValid()) {
        out << team.color();
    }
    out << team.info() << quint8(false) << quint8(team.count());

    for (int i = 0; i < team.count(); i++) {
        out << team.team(i);
    }

    emit sendCommand(tosend);
}

void Analyzer::sendBattleResult(int id, int result)
{
    notify(BattleFinished, qint32(id), qint32(result));
}

void Analyzer::reconnect(int id, const QByteArray &pass, int ccount)
{
    notify(Reconnect, quint32(id), pass, quint32(ccount == -1 ? getCommandCount() : ccount));
}

void Analyzer::battleCommand(int id, const BattleChoice &comm)
{
    notify(BattleMessage, qint32(id), comm);
}

void Analyzer::channelCommand(int command, int channelid, const QByteArray &body)
{
    QByteArray tosend;
    DataStream out(&tosend, QIODevice::WriteOnly);
    out << uchar(command) << qint32(channelid);

    /* We don't know for sure that << body will do what we want (as ServerSide we don't want
       to add a layer here), so a little hack, hoping datastreams concatenate data the same way as
        we do. */
    emit sendCommand(tosend + body);
}

void Analyzer::battleMessage(int id, const QString &str)
{
    if (sender()->property("isbattle").toBool())
        notify(BattleChat, qint32(id), str);
    else
        notify(SpectatingBattleChat, qint32(id), str);
}

void Analyzer::CPUnban(const QString &name)
{
    notify(NetworkCli::CPUnban, name);
}

void Analyzer::disconnectFromHost()
{
    socket().disconnectFromHost();
    socket().disconnect(this, SLOT(commandReceived(QByteArray)));
}

QString Analyzer::getIp() const
{
    return socket().ip();
}

void Analyzer::getRanking(const QString &tier, const QString &name)
{
    notify(ShowRankings, tier, false, name);
}

void Analyzer::getRanking(const QString &tier, int page)
{
    notify(ShowRankings, tier, true, qint32(page));
}

void Analyzer::connectTo(const QString &host, quint16 port)
{
    if (mysocket.isConnected()) {
        mysocket.close();
    }
    mysocket.connectToHost(host, port);
}

void Analyzer::error()
{
    emit connectionError(socket().error(), socket().errorString());
}

void Analyzer::wasConnected()
{
    /* At least makes client use full bandwith, even if the server doesn't */
    socket().setLowDelay(true);
}

/*{
    QHash<qint32, Battle> battles;
    in >> battles;
    emit battleListReceived(battles);
    break;
}*/

void Analyzer::commandReceived(const QByteArray &commandline)
{
    DataStream in (commandline, version.version);
    uchar command;

    in >> command;

    if (command != NetworkCli::Reconnect) {
        commandCount ++;
    }

    if (channelCommands.contains(command)) {
        qint32 chanid;
        in >> chanid;

        /* Because we're giving a pointer to a locally declared instance, this connection must
           be a DIRECT connection and never go in Queued mode. If you want queued mode, find another
           way to transfer the data */
        emit channelCommandReceived(command, chanid, &in);
        return;
    }
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

            commandReceived(info);
        } else { //contentType == 1
            DataStream in2(info);
            QByteArray packet;
            do {
                in2 >> packet;

                if (packet.length() > 0) {
                    commandReceived(packet);
                }
            } while (packet.length() > 0);
        }
        break;
    }
    case SendChatMessage: {
        Flags network,data;

        in >> network >> data;
        int channel = -1;

        if (network[0]) {
            in >> channel;
        }
        QString message;

        in >> message;

        if (channel != -1) {
            emit channelMessageReceived(message, channel, data[0]);
        } else {
            if (data[0]) {
                emit htmlMessageReceived(message);
            } else {
                emit messageReceived(message);
            }
        }
        break;
    }
    case KeepAlive: {
        quint16 ping;
        in >> ping;
        notify(KeepAlive, ping);
        break;
    }
    case PlayersList: {
        if (!registry_socket) {
            PlayerInfo p;
            while (!in.atEnd()) {
                in >> p;
                emit playerReceived(p);
            }
            break;
        } else {
            // Registry socket;
            ServerInfo s;
            in >> s;

            emit serverReceived(s);
        }
    }
    case Login: {
        Flags network;
        in >> network;

        if (network[0]) {
            in >> reconnectPass;
            emit reconnectPassGiven(reconnectPass);
        }
        PlayerInfo p;
        in >> p;
        QStringList tiers;
        in >> tiers;
        emit playerLogin(p, tiers);
        break;
    }
    case Logout: {
        qint32 id;
        in >> id;
        emit playerLogout(id);
        break;
    }
    case ChallengeStuff: {
        ChallengeInfo c;
        in >> c;
        emit challengeStuff(c);
        break;
    }
    case EngageBattle: {
        qint32 battleid;
        Flags network;
        Battle battle;

        if (version < ProtocolVersion(2,0)) {
            in >> battleid >> network >> battle.mode;
        } else {
            in >> network >> battleid;
        }

        in >> battle;

        if (network[0]) {
            /* This is a battle we take part in */
            TeamBattle team;
            BattleConfiguration conf;
            if (version < ProtocolVersion(1,0)) {
                conf.oldDeserialize(in);
            } else {
                in >> conf;
            }
            in >> team;
            if (network[1]) {
                in >> team.items;
            }
            emit battleStarted(battleid, battle, team, conf);
        } else {
            emit battleStarted(battleid, battle);
        }
        break;
    }
    case BattleFinished: {
        qint8 desc, mode;
        qint32 battleid;
        qint32 id1, id2;
        in >> battleid >> desc >> mode >> id1 >> id2;
        emit battleFinished(battleid, desc, id1, id2);
        break;
    }
    case BattleMessage: {
        qint32 battleid;
        QByteArray command;
        in >> battleid >> command;

        emit battleMessage(battleid, command);
        break;
    }
    case AskForPass: {
        QByteArray salt;
        in >> salt;

        if (salt.length() < 6 || strlen((" " + salt).data()) < 7)
            emit protocolError(5080, tr("The server requires insecure authentication."));
        emit passRequired(salt);
        break;
    }
    case Register: {
        emit notRegistered(true);
        break;
    }
    case PlayerKick: {
        qint32 p,src;
        in >> p >> src;
        emit playerKicked(p,src);
        break;
    }
    case PlayerBan: {
        qint32 p,src;
        in >> p >> src;
        emit playerBanned(p,src);
        break;
    }
    case PlayerTBan: {
        qint32 p, src, time;
        in >> p >> src >> time;
        emit playerTempBanned(p, src, time);
    }
    case SendTeam: {
        Flags network;
        in >> network;
        if (network[0]) {
            QString name;
            in >> name;
        }
        if (network[1]) {
            QStringList tiers;
            in >> tiers;
            emit teamApproved(tiers);
        }
        break;
    }
    case SendPM: {
        qint32 idsrc;
        QString mess;
        in >> idsrc >> mess;
        emit PMReceived(idsrc, mess);
        break;
    }
    case GetUserInfo: {
        UserInfo ui;
        in >> ui;
        emit userInfoReceived(ui);
        break;
    }
    case GetUserAlias: {
        QString s;
        in >> s;
        emit userAliasReceived(s);
        break;
    }
    case GetBanList: {
        QString s, i;
        quint32 dt;
        in >> s >> i >> dt;
        emit banListReceived(s,i,QDateTime::fromTime_t(dt));
        break;
    }
    case OptionsChange: {
        qint32 id;
        Flags f;
        in >> id >> f;
        emit awayChanged(id, f[1]);
        emit ladderChanged(id, f[0]);
        break;
    }
    case SpectateBattle: {
        Flags f;
        qint32 battleId;
        in >> f >> battleId;

        if (f[0]) {
            BattleConfiguration conf;
            if (version < ProtocolVersion(1,0)) {
                conf.oldDeserialize(in);
            } else {
                in >> conf;
            }
            emit spectatedBattle(battleId, conf);
        } else {
            emit spectatingBattleFinished(battleId);
        }
        break;
    }
    case SpectatingBattleMessage: {
        qint32 battleId;
        in >> battleId;
        /* Such a headache, it really looks like wasting ressources */
        char *buf;
        uint len;
        in.readBytes(buf, len);
        QByteArray command(buf, len);
        delete [] buf;
        emit spectatingBattleMessage(battleId, command);
        break;
    }
    case NetworkCli::VersionControl_: {
        ProtocolVersion server, feature, minor, major;
        Flags f;
        QString serverName;

        in >> server >> f >> feature >> minor >> major >> serverName;

        emit serverNameReceived(serverName);

        ProtocolVersion version;

        if (version < major) {
            emit versionDiff(major, -3);
        } else if (version < minor) {
            emit versionDiff(minor, -2);
        } else if (version < feature) {
            emit versionDiff(feature, -1);
        } else if (version < server) {
            emit versionDiff(server, 0);
        }

        this->version = server;

        break;
    }
    case Cookie: {
        Flags network; QString content;
        in >> network >> content;
        QString path = appDataPath("cookies", true) + "/" + QString::fromUtf8(sha_hash(socket().ip().toUtf8()).left(16));

        if (!network[0]) {
            QFile(path).remove();
        } else {
            writeFileContent(path, content.toUtf8());
        }
    }
    case TierSelection: {
        QByteArray tierList;
        in >> tierList;
        emit tierListReceived(tierList);
        break;
    }
    case ShowRankings: {
        bool starting;
        in >> starting;
        if (starting)
        {
            qint32 startingPage, startingRank, total;
            in >> startingPage >> startingRank >> total;
            emit rankingStarted(startingPage, startingRank, total);
        } else {
            QString name;
            qint32 points;
            in >> name >> points;
            emit rankingReceived(name, points);
        }
        break;
    }
    case Announcement: {
        if(!registry_socket) {
            QString ann;
            in >> ann;
            emit announcement(ann);
            break;
        } else {
            QString announcement;
            in >> announcement;
            emit regAnnouncementReceived(announcement);
            break;
        }
    }
    case ChannelsList: {
        QHash<qint32, QString> channels;
        in >> channels;
        emit channelsListReceived(channels);
        break;
    }
    case ChannelPlayers: {
        QVector<qint32> ids;
        qint32 chanid;
        in >> chanid >> ids;

        emit channelPlayers(chanid, ids);
        break;
    }
    case AddChannel: {
        QString name;
        qint32 id;
        in >> name >> id;

        emit addChannel(name, id);
        break;
    }
    case RemoveChannel: {
        qint32 id;
        in >> id;

        emit removeChannel(id);
        break;
    }
    case ChanNameChange: {
        qint32 id;
        QString name;
        in >> id >> name;

        emit channelNameChanged(id, name);
        break;
    }
    case SpecialPass: {
        QSettings s;
        s.beginGroup("password");
        s.setValue(QCryptographicHash::hash(QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5), QCryptographicHash::Sha1), QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5));
        s.endGroup();
        break;
    }
    case ServerPass: {
        QByteArray salt;
        in >> salt;
        emit serverPassRequired(salt);
        break;
    }
    /* Non-standard command, shouldn't exist */
    case ServerInfoChanged: {
        Flags f;
        in >> f;
        if (f[0]) {
            QString s;
            in >> s;
            emit serverNameReceived(s);
        }
        break;
    }
    case NetworkCli::Reconnect: {
        bool success;
        in >> success;

        if (success) {
            emit reconnectSuccess();
        } else {
            quint8 reason;
            in >> reason;
            emit reconnectFailure(reason);
        }
        break;
    }
    default: {
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received -- maybe an update for the program is available"));
    }
    }
}

Network<QTcpSocket*> & Analyzer::socket()
{
    return mysocket;
}

const Network<QTcpSocket*> & Analyzer::socket() const
{
    return mysocket;
}

void Analyzer::getBanList()
{
    notify(GetBanList);
}
