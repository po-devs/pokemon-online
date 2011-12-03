#ifndef PASSWORD_WALLET_H
#define PASSWORD_WALLET_H

#include <QtGui>

class PasswordWallet : public QObject {
public:
    PasswordWallet();

    bool retrieveUserPassword(const QString& ip, const QString& serverName, const QString& trainerName, const QString& salt, QString& pass, QStringList& warnings);
    bool retrieveServerPassword(const QString& ip, const QString& serverName, QString& pass, QStringList& warnings);

    void saveUserPassword(const QString& ip, const QString& serverName, const QString& trainerName, const QString& salt, const QString& pass);
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
        QString salt;
        QString pass;
    };

private:
    void load();

    void save();

    const QString dataPath;

    QVector<ServerPassRecord> serverPass;
    QVector<UserPassRecord> userPass;
    
};

QDataStream &operator<<(QDataStream &ds, const PasswordWallet::ServerPassRecord &r);
QDataStream &operator>>(QDataStream &ds, PasswordWallet::ServerPassRecord &r);

QDataStream &operator<<(QDataStream &ds, const PasswordWallet::UserPassRecord &r);
QDataStream &operator>>(QDataStream &ds, PasswordWallet::UserPassRecord &r);
#endif
