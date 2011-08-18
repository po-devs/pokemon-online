#include "analyze.h"
#include "network.h"


using namespace NetworkReg;

Analyzer::Analyzer(QTcpSocket *sock, int id) : mysocket(sock, id)
{
    connect(&socket(), SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&socket(), SIGNAL(isFull(QByteArray)), this, SLOT(commandReceived(QByteArray)));
    connect(&socket(), SIGNAL(_error()), this, SLOT(error()));
    connect(this, SIGNAL(sendCommand(QByteArray)), &socket(), SLOT(send(QByteArray)));

    mytimer = new QTimer(this);
    connect(mytimer, SIGNAL(timeout()), this, SLOT(keepAlive()));
    mytimer->start(30000); //every 30 secs
}

Analyzer::~Analyzer()
{
    blockSignals(true);
}

void Analyzer::close() {
    socket().close();
}

QString Analyzer::ip() const {
    return socket().ip();
}

void Analyzer::keepAlive()
{
    notify(KeepAlive);
}

void Analyzer::error()
{
    emit connectionError(socket().error(), socket().errorString());
}

bool Analyzer::isConnected() const
{
    return socket().isConnected();
}


void Analyzer::commandReceived(const QByteArray &commandline)
{
    QDataStream in (commandline);
    in.setVersion(QDataStream::Qt_4_5);
    uchar command;

    in >> command;

    switch (command) {
    case Login:
        {
            QString name, desc;
            quint16 num, max, nport;
            in >> name >> desc >> num >> max >> nport;
            emit loggedIn(name,desc,num, max, nport);
            break;
        }
    case ServNumChange:
        {
            quint16 num;
            in >> num;
            emit numChange(num);
            break;
        }
    case ServNameChange:
        {
            QString name;
            in >> name;
            emit nameChange(name);
            break;
        }
    case ServDescChange:
        {
            QString desc;
            in >> desc;
            emit descChange(desc);
            break;
        }
    case ServMaxChange:
        {
            quint16 max;
            in >> max;
            emit maxChange(max);
        }
    default:
        emit protocolError(UnknownCommand, tr("Protocol error: unknown command received"));
        break;
    }
}

void Analyzer::sendServer(const QString &name, const QString &desc, quint16 numplayers, const QString &ip,quint16 max, quint16 port)
{
    notify(PlayersList, name, desc, numplayers, ip, max, port);
}

void Analyzer::sendServerListEnd()
{
  notify(ServerListEnd);
}

void Analyzer::sendInvalidName()
{
    notify(ServNameChange);
}

void Analyzer::sendNameTaken()
{
    notify(Register);
}

void Analyzer::sendAccept()
{
    notify(Login);
}

Network & Analyzer::socket()
{
    return mysocket;
}

const Network & Analyzer::socket() const
{
    return mysocket;
}

void Analyzer::notify(int command)
{
    if (!isConnected())
        return;
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command);

    emit sendCommand(tosend);
}

