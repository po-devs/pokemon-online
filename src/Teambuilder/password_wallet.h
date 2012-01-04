#ifndef PASSWORD_WALLET_H
#define PASSWORD_WALLET_H

#include <QtGui>

class DataStream;

class PasswordWallet : public QObject {
public:
    PasswordWallet();

    bool retrieveUserPassword(const QString& ip, const QString& serverName, const QString& trainerName, const QByteArray& salt, QString& pass, QStringList& warnings);
    bool retrieveServerPassword(const QString& ip, const QString& serverName, QString& pass, QStringList& warnings);

    void saveUserPassword(const QString& ip, const QString& serverName, const QString& trainerName, const QByteArray& salt, const QString& pass);
    void saveServerPassword(const QString& ip, const QString& serverName, const QString& pass);

    struct ServerPassRecord
    {
        QString ip;
        QString server;
        QString pass;
    };
    
    struct UserPassRecord
    {
        QString ip;
        QString server;
        QString name;
        QByteArray salt;
        QString pass;
    };

private:
    void load();

    void save();

    const QString dataPath;

    QVector<ServerPassRecord> serverPass;
    QVector<UserPassRecord> userPass;
    
};

DataStream &operator<<(DataStream &ds, const PasswordWallet::ServerPassRecord &r);
DataStream &operator>>(DataStream &ds, PasswordWallet::ServerPassRecord &r);

DataStream &operator<<(DataStream &ds, const PasswordWallet::UserPassRecord &r);
DataStream &operator>>(DataStream &ds, PasswordWallet::UserPassRecord &r);
#endif
