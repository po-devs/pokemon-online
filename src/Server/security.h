#ifndef SECURITY_H
#define SECURITY_H

#include <QtNetwork>
#include <QtCore>
#include <QtSql>
#include "../Utilities/otherwidgets.h"

class WaitingObject;
class LoadThread;
class InsertThread;

class SecurityManager : public QObject
{
    Q_OBJECT
public:

    enum QueryType {
        GetInfoOnUser,
    };

    static void init();

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

        static const int saltLength = 7;
    };

    static bool isValid(const QString &name);
    static bool exist(const QString &name);
    static void create (const QString &name, const QString &date, const QString &ip);
    static Member member(const QString &name);

    static void updateMember(const Member &m);

    static bool bannedIP(const QString &ip);
    static bool isInMemory(const QString &name);
    static void ban(const QString &name);
    static void unban(const QString &name);
    static void IPunban(const QString &ip);
    static void setauth(const QString &name, int auth);
    static void clearPass(const QString &name);
    static void addNonExistant(const QString &name);
    static void addMemberInMemory(const Member &m);
    static void removeMemberInMemory(const QString &name);
    static void updateMemberInDatabase(const Member &m, bool add);
    static void cleanCache();
    static int maxAuth(const QString &ip);

    static void loadMemberInMemory(const QString &name, QObject *o=NULL, const char *slot=NULL);

    static QString ip(const QString &name);

    static QStringList membersForIp(const QString &ip);
    static QHash<QString, QString> banList();

private slots:
    void freeObject();

private:
    static void loadMembers();
    static QHash<QString, Member> members;
    static QSet<QString> nonExistentMembers;
    static QMutex memberMutex;
    static QLinkedList<QString> cachedMembersOrder;
    static QMutex cachedMembersMutex;
    static QSet<QString> bannedIPs;
    static QHash<QString, QString> bannedMembers;

    static WaitingObject* getObject();
    static void freeObject(WaitingObject *c);
    static LoadThread * getThread();


    static QSet<WaitingObject*> freeObjects;
    static QSet<WaitingObject*> usedObjects;

    static SecurityManager * instance;

    static const int loadThreadCount=4;
    static int nextLoadThreadNumber;
    static LoadThread *threads;
    static InsertThread *ithread;

    static QNickValidator val;
};

class LoadThread : public QThread
{
public:
    void pushQuery(const QString &name, WaitingObject *w, SecurityManager::QueryType query_type);

    void run();

    static void processQuery (QSqlQuery *q, const QString &name, SecurityManager::QueryType query_type);
private:
    struct Query {
        QString member;
        WaitingObject *w;
        SecurityManager::QueryType query_type;

        Query(const QString &m, WaitingObject *w, SecurityManager::QueryType query_type)
            : member(m), w(w), query_type(query_type)
        {

        }
    };

    QLinkedList<Query> queries;
    QMutex queryMutex;
    QSemaphore sem;
};

class InsertThread : public QThread
{
public:
    /* update/insert ? */
    void pushMember(const SecurityManager::Member &m, bool update=true);

    void run();

    static void processMember (QSqlQuery *q, const SecurityManager::Member &m, bool update=true);
private:
    QLinkedList<QPair<SecurityManager::Member, bool> > members;
    QMutex memberMutex;
    QSemaphore sem;
};

#endif // SECURITY_H
