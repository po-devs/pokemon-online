#ifndef SECURITY_H
#define SECURITY_H

#include <QtCore>

#include "../Utilities/otherwidgets.h"
#include "memoryholder.h"

class WaitingObject;
class QSqlQuery;
class LoadThread;
template<class T> class InsertThread;

class SecurityManager : public QObject
{
    Q_OBJECT

    friend class ScriptEngine;
public:

    enum QueryType {
        GetInfoOnUser,
    };

    static void init();
    static void destroy();

    /* A member as stored in the file */
    struct Member {
        Member(const QString &name="", const QByteArray &date="", int auth = 0, bool banned = false,
               const QByteArray &salt="", const QByteArray &hash="", const QByteArray &ip="");
        QString name;
        QByteArray date;
        int auth;
        bool banned;
        QByteArray salt;
        QByteArray hash;
        QByteArray ip;


        void modifyIP(const QByteArray &ip) {
            this->ip = ip;
        }

        void modifyDate(const QByteArray &date) {
            this->date = date;
        }

        bool isProtected() const {
            return hash.length() > 0 && hash[0] != ' ';
        }

        bool isBanned() const {
            return banned;
        }

        int authority() const {
            return  auth;
        }

        void setAuth(int auth) {
            this->auth = auth;
        }

        void ban() {
            banned = true;
        }

        void unban() {
            banned = false;
        }

        void clearPass() {
            hash = "";
        }

        QString toString() const;

        static const int saltLength = 7;
    };

    static bool isValid(const QString &name);
    static bool exist(const QString &name);
    static void create (const QString &name, const QString &date, const QString &ip);
    static Member member(const QString &name);

    static void updateMember(const Member &m);

    static bool bannedIP(const QString &ip);
    static void ban(const QString &name);
    static void unban(const QString &name);
    static void IPunban(const QString &ip);
    static void setAuth(const QString &name, int auth);
    static void clearPass(const QString &name);
    static void updateMemberInDatabase(const Member &m, bool add);
    static int maxAuth(const QString &ip);

    static void loadMemberInMemory(const QString &name, QObject *o=NULL, const char *slot=NULL);

    static QString ip(const QString &name);

    static QStringList membersForIp(const QString &ip);
    static QHash<QString, QString> banList();
    static QStringList authList();
    static QStringList userList();
    static void deleteUser(const QString &name);

    /* Exports the whole database to members.txt. Done in the main thread (and please call it
       only from there), so hangs the server */
    static void exportDatabase();

private slots:
    static void insertMember(QSqlQuery *q, void *m, int update);
    static void loadMember(QSqlQuery *q, const QVariant &name, int query_type);

private:
    static void loadMembers();

    static MemoryHolder<Member> holder;

    static QSet<QString> bannedIPs;
    static QHash<QString, QString> bannedMembers;

    static WaitingObject* getObject();
    static void freeObject(WaitingObject *c);

    static QSet<WaitingObject*> freeObjects;
    static QSet<WaitingObject*> usedObjects;

    static SecurityManager * instance;

    static const int loadThreadCount=2;
    static int nextLoadThreadNumber;
    static LoadThread **threads;
    static InsertThread<Member> *ithread;

    static LoadThread * getThread();

    static QNickValidator val;
};

#endif // SECURITY_H
