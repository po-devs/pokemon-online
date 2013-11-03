#include <QColor>
#include "../QtWebsocket/QWsSocket.h"
namespace Nw {
#include "../Shared/networkcommands.h"
}
#include "../Teambuilder/network.h"
#include "../PokemonInfo/battlestructs.h"
#include "pokemontojson.h"
#include "dualwielder.h"
#include <functional>

DualWielder::DualWielder(QObject *parent) : QObject(parent), web(NULL), network(NULL), registryRead(false), myid(-1)
{
    /* No need to waste network bandwith */
    jserial.setIndentMode(QJson::IndentCompact);

    /* Connects BattleInput / BattleConverter */
    input.addOutput(&battleConverter);
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

void DualWielder::init(QWsSocket *sock, const QString &host, QHash<QString,QString> aliases, const QString &servers)
{
    this->servers = servers;
    web = sock;
    mIp = web->ip();
    this->aliases = aliases;

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

    DataStream in (commandline, version.version);
    uchar command;

    in >> command;

    switch (command) {
    case Nw::ZipCommand: {
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
    case Nw::SendChatMessage: {
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
    case Nw::KeepAlive: {
        quint16 ping;
        in >> ping;
        notify(Nw::KeepAlive, ping);
        break;
    }
    case Nw::PlayersList: {
        QVariantMap _map;
        PlayerInfo p;
        while (!in.atEnd()) {
            in >> p;
            if (toIgnore.contains(p.id)) {
                toIgnore.remove(p.id);
                continue;
            } else {
                toIgnore.clear();
            }
            bool fullInfo = p.id == myid;
            if (importantPlayers.contains(p.id)) {
                importantPlayers.remove(importantPlayers.indexOf(p.id),1);
                fullInfo = true;
            }
            QVariantMap map;
            map.insert("name", p.name);
            if (fullInfo) {
                map.insert("info", p.info);
                map.insert("avatar", p.avatar);
            }
            map.insert("auth", p.auth);
            //map.insert("away", p.away());
            if (p.color.isValid()) {
                map.insert("color", p.color);
            }

            if (fullInfo) {
                QVariantMap ratings;
                foreach(QString tier, p.ratings.keys()) {
                    ratings.insert(tier, p.ratings[tier]);
                }
                map.insert("ratings", ratings);
            }
            _map.insert(QString::number(p.id), map);
        }
        if (_map.count() > 0) {
            web->write("players|"+QString::fromUtf8(jserial.serialize(_map)));
        }
        break;
    }
    case Nw::Login: {
        Flags network;
        in >> network;

        QVariantMap _map;

        if (network[0]) {
            QByteArray reconnectPass;
            in >> reconnectPass;
            _map.insert("reconnectPass", reconnectPass.toBase64());
        }
        PlayerInfo p;
        in >> p;
        myid = p.id;
        _map.insert("id",p.id);

        QVariantMap map;
        map.insert("name", p.name);
        if (p.id == myid) {
            map.insert("info", p.info);
            map.insert("avatar", p.avatar);
        }
        map.insert("auth", p.auth);
        //map.insert("battling", p.battling());
        //map.insert("away", p.away());
        if (p.color.isValid()) {
            map.insert("color", p.color);
        }
        if (p.id == myid) {
            QVariantMap ratings;
            foreach(QString tier, p.ratings.keys()) {
                ratings.insert(tier, p.ratings[tier]);
            }
            map.insert("ratings", ratings);
        }
        _map.insert("info", map);
        QStringList tiers;
        in >> tiers;
        QVariantList list;
        foreach(QString t, tiers) {
            list.push_back(t);
        }
        _map.insert("tiers", tiers);

        web->write("login|"+QString::fromUtf8(jserial.serialize(_map)));
        break;
    }
    case Nw::Logout: {
        qint32 id;
        in >> id;
        web->write("playerlogout|"+QString::number(id));
        break;
    }
    case Nw::JoinChannel: {
        qint32 chan,id;
        in >> chan >> id;

        web->write("join|"+QString::number(chan)+"|"+QString::number(id));
        break;
    }
    case Nw::LeaveChannel: {
        qint32 chan,id;
        in >> chan >> id;

        web->write("leave|"+QString::number(chan)+"|"+QString::number(id));
        break;
    }
//    case ChallengeStuff: {
//        ChallengeInfo c;
//        in >> c;
//        emit challengeStuff(c);
//        break;
//    }
    case Nw::EngageBattle: {
        Flags network;
        Battle battle;
        qint32 battleid;


        if (version < ProtocolVersion(2,0)) {
            in >> battleid >> network >> battle.mode;
        } else {
            in >> network >> battleid;
        }

        in >> battle;

        QVariantMap params;
        params.insert("ids", QVariantList() << battle.id1 << battle.id2);

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
            params.insert("conf", toJson(conf));
            params.insert("team", toJson(team));
        }
        web->write("battlestarted|"+QString::number(battleid)+"|"+QString::fromUtf8(jserial.serialize(params)));
        break;
    }
    case Nw::BattleFinished: {
        qint8 desc, mode;
        qint32 battleid;
        qint32 id1, id2;
        in >> battleid >> desc >> mode >> id1 >> id2;

        QVariantMap params;
        params.insert("result", desc);
        params.insert("mode", mode);
        params.insert("winner", id1);
        params.insert("loser", id2);
        web->write("battlefinished|"+QString::number(battleid)+"|"+QString::fromUtf8(jserial.serialize(params)));

        /* We don't want rating updates on the webclient */
        toIgnore.clear();
        toIgnore.insert(id1);
        toIgnore.insert(id2);
        break;
    }
    case Nw::BattleMessage: {
        qint32 battleid;
        QByteArray command;
        in >> battleid >> command;

        input.receiveData(command);
        QVariantMap jcommand = battleConverter.getCommand();
        if (jcommand.count() > 0) {
            web->write("battlecommand|"+QString::number(battleid)+"|"+QString::fromUtf8(jserial.serialize(jcommand)));
        }
        break;
    }
    case Nw::AskForPass: {
        QByteArray salt;
        in >> salt;

        if (salt.length() < 6 || strlen((" " + salt).data()) < 7)
            web->write(QString("msg|" "Protocol error: The server requires insecure authentication."));
        else
            web->write("challenge|"+QString::fromUtf8(salt));
        break;
    }
    case Nw::Register: {
        web->write(QString("unregistered|"));
        break;
    }
    case Nw::PlayerKick: {
        qint32 p,src;
        in >> p >> src;

        QVariantMap map;
        map.insert("source", src);
        map.insert("target", p);
        web->write("playerkick|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
    case Nw::PlayerBan: {
        qint32 p,src;
        in >> p >> src;

        QVariantMap map;
        map.insert("source", src);
        map.insert("target", p);
        web->write("playerban|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
    case Nw::PlayerTBan: {
        qint32 p,src,time;
        in >> p >> src >> time;

        QVariantMap map;
        map.insert("source", src);
        map.insert("target", p);
        map.insert("time", time);
        web->write("playerban|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
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
    case Nw::SendPM: {
        qint32 idsrc;
        QString mess;
        in >> idsrc >> mess;

        QVariantMap map;
        map.insert("src", idsrc);
        map.insert("message", mess);
        web->write("pm|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
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
    case Nw::SpectateBattle: {
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

            web->write("watchbattle|"+QString::number(battleId)+"|"+QString::fromUtf8(jserial.serialize(toJson(conf))));
        } else {
            web->write("stopwatching|"+QString::number(battleId));
        }
        break;
    }
    case Nw::SpectatingBattleMessage: {
        qint32 battleId;
        QByteArray command;

        in >> battleId >> command;
        input.receiveData(command);

        QVariantMap jcommand = battleConverter.getCommand();
        if (jcommand.count() > 0) {
            web->write("battlecommand|"+QString::number(battleId)+"|"+QString::fromUtf8(jserial.serialize(jcommand)));
        }
        break;
    }
    case Nw::VersionControl_: {
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
    case Nw::TierSelection: {
        qint32 len;
        in >> len; //Unused, but describes length of following data

        uchar level;
        in >> level;

        std::function<void(QVariantList&, int)> func;
        func = [&in, &level, &func](QVariantList &parent, int curlevel) {
            if (in.atEnd()) {
                return;
            }
            QString name;
            in >> name;

            //peek of the next level
            in >> level;

            if (level > curlevel) {
                /* Next is a child */
                QVariantMap current;
                QVariantList tiers;

                current.insert("name", name);
                func(tiers, level);
                current.insert("tiers", tiers);

                parent.push_back(current);
            } else {
                parent.push_back(name);
            }

            //needs to be outside the ifs, since it can be called after both branches
            //because level will be modified by the recursive calls!
            if (level == curlevel) {
                func(parent, level);
            }

            return;
        };

        QVariantList root;
        func(root, level);

        QString res = "tiers|" + jserial.serialize(root);
        web->write(res);

        break;
    }
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
    case Nw::ShowRankings2: {
        quint8 mode;
        quint32 id;
        quint8 count;

        in >> mode >> id >> count;

        QVariantMap rankings;

        for (int i = 0; i < count; i++) {
            QString tier;
            quint16 rating;
            qint32 ranking, total;

            in >> tier >> rating >> ranking >> total;

            QVariantMap obj;
            obj.insert("rating", rating);
            obj.insert("ranking", ranking);
            obj.insert("total", total);

            rankings.insert(tier, obj);
        }

        web->write("rankings|" + QString::number(id) + "|" + jserial.serialize(rankings));
        break;
    }
    case Nw::Announcement: {
        QString announcement;
        in >> announcement;
        web->write("announcement|"+announcement);
        break;
    }
    case Nw::ChannelsList: {
        QHash<qint32, QString> channels;
        in >> channels;

        QVariantMap map;
        QHashIterator<qint32,QString> it(channels);
        while (it.hasNext()) {
            it.next();
            map.insert(QString::number(it.key()), it.value());
        }
        web->write("channels|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
    case Nw::ChannelPlayers: {
        QVector<qint32> ids;
        qint32 chanid;
        in >> chanid >> ids;

        QVariantMap map;
        map.insert("channel", chanid);
        QVariantList list;
        foreach(int id, ids) {
            list.push_back(id);
        }
        map.insert("players", list);
        web->write("channelplayers|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
    case Nw::AddChannel: {
        QString name;
        qint32 id;
        in >> name >> id;

        QVariantMap map;
        map.insert("name", name);
        map.insert("id", id);
        web->write("newchannel|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
    case Nw::RemoveChannel: {
        qint32 id;
        in >> id;

        web->write("removechannel|"+QString::number(id));
        break;
    }
    case Nw::ChanNameChange: {
        qint32 id;
        QString name;
        in >> id >> name;

        QVariantMap map;
        map.insert("name", name);
        map.insert("id", id);
        web->write("channelnamechange|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
    case Nw::BattleList: {
        quint32 channel;
        in >> channel;

        QHash<qint32, Battle> battles;
        in >> battles;

        QVariantMap res;
        QHashIterator<qint32, Battle> it(battles);
        while (it.hasNext()) {
            it.next();

            QVariantMap data;
            data.insert("ids", QVariantList() << it.value().id1 << it.value().id2);
            //data.insert("mode", it.value().mode);
            res.insert(QString::number(it.key()), data);
        }
        web->write("channelbattlelist|"+QString::number(channel)+"|"+QString::fromUtf8(jserial.serialize(res)));
        break;
    }
    case Nw::ChannelBattle: {
        qint32 chanid, id;
        Battle battle;
        in >> chanid >> id >> battle;
        QVariantMap map;
        map.insert("battleid", id);
        QVariantMap data;
        data.insert("ids", QVariantList() << battle.id1 << battle.id2);
        map.insert("battle", data);
        web->write("channelbattle|"+QString::number(chanid)+"|"+QString::fromUtf8(jserial.serialize(map)));
        break;
    }
//    case SpecialPass: {
//        QSettings s;
//        s.beginGroup("password");
//        s.setValue(QCryptographicHash::hash(QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5), QCryptographicHash::Sha1), QCryptographicHash::hash(getIp().toUtf8(), QCryptographicHash::Md5));
//        s.endGroup();
//        break;
//    }
    case Nw::ServerPass: {
        QByteArray salt;
        in >> salt;
        web->write("serverpass|"+QString::fromUtf8(salt));
        break;
    }
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
            QString host = data.section(":", 0, -2);
            int port = data.section(":", -1).toInt();

            network = new Network(aliases.value(host, host), port);

            connect(network, SIGNAL(connected()), SLOT(socketConnected()));
            connect(network, SIGNAL(disconnected()), SLOT(socketDisconnected()));
            connect(network, SIGNAL(disconnected()), network, SLOT(deleteLater()));
            connect(network, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketDisconnected()));
            connect(network, SIGNAL(error(QAbstractSocket::SocketError)), network, SLOT(deleteLater()));
            connect(network, SIGNAL(isFull(QByteArray)), SLOT(readSocket(QByteArray)));
            connect(this, SIGNAL(sendCommand(QByteArray)), network, SLOT(send(QByteArray)));
        } else if (command == "registry" && !registryRead) {
            web->write(servers);
            registryRead = true;
        } else {
            web->write(QString("error|You need to choose a server to connect to."));
        }
    } else {
        if (command == "login") {
            QVariantMap params = jparser.parse(data.toUtf8()).toMap();

            QByteArray tosend;
            DataStream out(&tosend, QIODevice::WriteOnly);

            Flags network;
            network.setFlags((1 << Nw::LoginCommand::HasClientType) | (params.contains("version") << Nw::LoginCommand::HasVersionNumber));

            if (!params.value("default").isNull()) {
                network.setFlag(Nw::LoginCommand::HasDefaultChannel, true);
            }

            if (params.value("autojoin").toList().size() > 0) {
                network.setFlag(Nw::LoginCommand::HasAdditionalChannels, true);
            }

            if (params.value("color").value<QColor>().isValid()) {
                network.setFlag(Nw::LoginCommand::HasColor, true);
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

            out << uchar(Nw::Login) << ProtocolVersion() << network << "webclient";

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
                notify(Nw::SendChatMessage, Flags(1), Flags(0), qint32(0), data);
            } else {
                notify(Nw::SendChatMessage, Flags(1), Flags(0), qint32(params.value("channel").toInt()), params.value("message").toString());
            }
        } else if (command == "auth") {
            notify(Nw::AskForPass, QByteArray::fromHex(data.toUtf8()));
        } else if (command == "join") {
            notify(Nw::JoinChannel, data);
        } else if (command == "leave") {
            notify(Nw::LeaveChannel, qint32(data.toInt()));
        } else if (command == "pm") {
            QVariantMap params = jparser.parse(data.toUtf8()).toMap();
            notify(Nw::SendPM, qint32(params.value("to").toInt()), params.value("message").toString());
        } else if (command == "teamChange") {
            qDebug() << "teamChange event";
            QVariantMap params = jparser.parse(data.toUtf8()).toMap();
            Flags network(params.contains("name") | (params.contains("color") << 1));

            QByteArray tosend;
            DataStream out(&tosend, QIODevice::WriteOnly);

            out << uchar(Nw::SendTeam) << network;

            if (params.contains("name")) {
                out << params.value("name").toString();
            }
            if (params.contains("color")) {
                out << params.value("color").value<QColor>();
            }

            qDebug() << "network: " << network.data;
            qDebug() << "color: " << params.value("color").toString() << ", "  << params.value("color").value<QColor>().name();

            emit sendCommand(tosend);
        } else if (command == "register") {
            notify(Nw::Register);
        } else if (command == "watch") {
            notify(Nw::SpectateBattle, qint32(data.toInt()), Flags(true));
        } else if (command == "stopwatching") {
            notify(Nw::SpectateBattle, qint32(data.toInt()), Flags(false));
        } else if (command =="forfeit") {
            /* Do both just in case. */
            notify(Nw::BattleFinished, qint32(data.toInt()), Forfeit);
            notify(Nw::BattleFinished, qint32(data.toInt()), Close);
        } else if (command == "player") {
            int p = data.toInt();
            /* Spam control */
            if (importantPlayers.size() > 10) {
                importantPlayers.clear();
            }
            importantPlayers.push_back(p);
            notify(Nw::PlayersList, qint32(p));
        } else if (command == "spectatingchat") {
            int battle = data.section("|", 0, 0).toInt();
            QString chat = data.section("|", 1);
            notify(Nw::SpectatingBattleChat, qint32(battle), chat);
        } else if (command == "findbattle") {
            QVariantMap params = jparser.parse(data.toUtf8()).toMap();
            FindBattleData fdata;
            fdata.rated = params.value("rated", false).toBool();
            fdata.sameTier = params.value("sameTier", true).toBool();
            fdata.rated = params.contains("range");
            fdata.range = params.value("range", 300).toInt();
            fdata.teams = 0;
            notify(Nw::FindBattle, fdata);
        } else if (command == "battlechoice") {
            qDebug() << "battle choice";
            int battle = data.section("|", 0, 0).toInt();
            QVariantMap params = jparser.parse(data.section("|", 1).toUtf8()).toMap();

            BattleChoice choice = fromJson(params);
            notify(Nw::BattleMessage, qint32(battle), choice);
        } else if (command == "battlechat") {
            int battle = data.section("|", 0, 0).toInt();
            QString chat = data.section("|", 1);
            notify(Nw::BattleChat, qint32(battle), chat);
        } else if (command == "getrankings") {
            int battle = data.toInt();
            notify(Nw::ShowRankings2, qint8(0), qint32(battle));

            qDebug() << "rankings ask";
        }
    }
}

void DualWielder::socketConnected()
{
    if (web) {
        notify(Nw::SetIP, web->ip());
        web->write(QString("connected|"));
    }
}

void DualWielder::socketDisconnected()
{
    network = NULL;
    if (web) {
        qDebug() << "Closed connection to server " << web->ip();

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
        if (network->state() == QAbstractSocket::ConnectedState) {
            /* Gives the server the curtesy to know that there will be no reconnection */
            notify(Nw::Logout);
            network->close();
        } else {
            network->deleteLater();
        }
        network = NULL;
    }

    deleteLater();
}
