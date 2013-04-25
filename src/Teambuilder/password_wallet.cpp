#include "password_wallet.h"
#include "../Utilities/coreclasses.h"

namespace {
    static quint32 MAGIC = 0xB0C3B455;
    static quint16 VERSION = 0x0002;
    inline QString dataLocation() {
#ifdef QT5
        return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
        return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
    }
}

PasswordWallet::PasswordWallet() :
    dataPath(dataLocation() + "/wallet.dat"), serverPass(), userPass()
{
    load();
}

void PasswordWallet::load()
{
    QFile f(dataPath);
    if (!f.open(QIODevice::ReadOnly))
        return;
    DataStream s(&f);
    // sanity checks
    quint32 magic;
    s >> magic;
    if (magic != MAGIC)
        return;
    quint16 version;
    s >> version;
    if (version != VERSION)
        return;

    // read server and user pass
    s >> serverPass;
    s >> userPass;
}

void PasswordWallet::save()
{
    QDir d;
    if (!d.mkpath(dataLocation()))
        return; // XXX: error message?

    QFile f(dataPath);
    if (!f.open(QIODevice::WriteOnly))
        return;
    DataStream s(&f);
    // write magic byte
    s << MAGIC;
    s << VERSION;

    // write server and user pass
    s << serverPass;
    s << userPass;
}

bool PasswordWallet::retrieveUserPassword(const QString& ip,
                                          const QString &serverName,
                                          const QString &trainerName,
                                          const QByteArray &salt, QString &pass,
                                          QStringList &warnings)
{
    for (QVector<UserPassRecord>::iterator i = userPass.begin(); i != userPass.end(); ++i) {
        UserPassRecord& r = *i;
    //foreach(UserPassRecord& r, userPass) {
        if (r.name == trainerName && (r.salt == salt)) {
            if (r.ip != ip)
                warnings.append(tr("Warning: the Server IP Address has changed since password was saved."));
            if (r.server != serverName)
                warnings.append(tr("Warning: the Server Name has changed since password was saved."));
            pass = r.pass;
            return true;
        }
    }
    pass = "";
    return false;
}

bool PasswordWallet::retrieveServerPassword(const QString& ip,
                                            const QString &serverName,
                                            QString &pass,
                                            QStringList &warnings)
{
    for (QVector<ServerPassRecord>::iterator i = serverPass.begin(); i != serverPass.end(); ++i) {
        ServerPassRecord& r = *i;
    //foreach(ServerPassRecord& r, serverPass) {
        if (r.ip == ip || r.server == serverName) {
            if (r.ip != ip)
                warnings.append(tr("Warning: the Server IP Address has changed since password was saved."));
            if (r.server != serverName)
                warnings.append(tr("Warning: the Server Name has changed since password was saved."));
            pass = r.pass;
            return true;
        }
    }
    pass = "";
    return false;
}

void PasswordWallet::saveUserPassword(const QString& ip,
                                      const QString &serverName,
                                      const QString &trainerName,
                                      const QByteArray &salt, const QString &pass)
{
    for (QVector<UserPassRecord>::iterator i = userPass.begin(); i != userPass.end(); ++i) {
        UserPassRecord& r = *i;
    //foreach(UserPassRecord& r, userPass) {
        if (r.name == trainerName && r.salt == salt) {
            r.ip = ip;
            r.server = serverName;
            r.salt = salt;
            r.pass = pass;
            save();
            return;
        }
    }
    UserPassRecord record = {ip, serverName, trainerName, salt, pass};
    userPass.append(record);
    save();
}

void PasswordWallet::saveServerPassword(const QString& ip,
                                        const QString &serverName,
                                        const QString &pass)
{
    for (QVector<ServerPassRecord>::iterator i = serverPass.begin(); i != serverPass.end(); ++i) {
        ServerPassRecord& r = *i;
    //foreach(ServerPassRecord& r, serverPass) {
        if (r.ip == ip || r.server == serverName) {
            r.ip = ip;
            r.server = serverName;
            r.pass = pass;
            save();
            return;
        }
    }
    ServerPassRecord record = {ip, serverName, pass};
    serverPass.append(record);
    save();
}

DataStream &operator<<(DataStream &ds, const PasswordWallet::ServerPassRecord &r)
{
    ds << r.ip;
    ds << r.server;
    ds << r.pass;
    return ds;
}

DataStream &operator>>(DataStream &ds, PasswordWallet::ServerPassRecord &r)
{
    ds >> r.ip;
    ds >> r.server;
    ds >> r.pass;
    return ds;
}

DataStream &operator<<(DataStream &ds, const PasswordWallet::UserPassRecord &r)
{
    ds << r.ip;
    ds << r.server;
    ds << r.name;
    ds << r.salt;
    ds << r.pass;
    return ds;
}

DataStream &operator>>(DataStream &ds, PasswordWallet::UserPassRecord &r)
{
    ds >> r.ip;
    ds >> r.server;
    ds >> r.name;
    ds >> r.salt;
    ds >> r.pass;
    return ds;
}

