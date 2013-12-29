#ifndef SECURITY_H
#define SECURITY_H

#include <QtCore>

/* For istringmap */
#include <Utilities/coreclasses.h>
/* For QNickValidator */
#include <Utilities/otherwidgets.h>

#include "memoryholder.h"

class WaitingObject;

template<class T> class LoadInsertThread;

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
        Member(const QString &name="", const QString &date="", int auth = 0, bool banned = false,
               const QByteArray &salt="", const QByteArray &hash="", const QString &ip="", int ban_expire_time = 0);
        QString name;
        QString date;
        int auth;
        bool banned;
        QByteArray salt;
        QByteArray hash;
        QString ip;
        int filepos; // position in the file
        unsigned int ban_expire_time;

        void modifyIP(const QString &ip) {
            this->ip = ip;
        }

        void modifyDate(const QString &date) {
            this->date = date;
        }

        bool isProtected() const {
            return hash.length() > 0 && hash[0] != ' ';
        }

        bool isBanned() const {
            return banned && (ban_expire_time == 0 || ban_expire_time > QDateTime::currentDateTimeUtc().toTime_t());
        }

        int authority() const {
            return  auth;
        }

        void setAuth(int auth) {
            this->auth = auth;
        }

        void ban(int time = 0) {
            banned = true;
            if (time == 0) {
                ban_expire_time = 0;
            } else {
                ban_expire_time = QDateTime::currentDateTimeUtc().toTime_t() + time * 60;
            }
        }

        void unban() {
            banned = false;
        }

        void clearPass() {
            hash = "";
        }

        void setBanExpireTime(int time) {
            ban_expire_time = time;
        }

        QString toString() const;

        static const int saltLength = 7;
        static const int hashLength = 32;
        static const int dateLength = 19;
        static const int ipLength = 39; //IPv6 is 39, so lets be ready for the future
        static const int banTimeLength = 10;

        void write(QIODevice *device) const;
    };


    static const istringmap<Member> & getMembers();

    static bool registered(const QString &name);
    static int auth(const QString &name);

    static bool isValid(const QString &name);
    static bool exist(const QString &name);
    static void create (const QString &name, const QString &date, const QString &ip, bool banned = false);
    static Member member(const QString &name);

    static void updateMember(const Member &m);

    static bool bannedIP(const QString &ip);
    static void ban(const QString &name, int time=0);
    static void banIP(const QString &ip, int time=0);
    static void unban(const QString &name);
    static void unbanIP(const QString &ip);
    static int numRegistered(const QString &ip);
    static void setAuth(const QString &name, int auth);
    static void clearPass(const QString &name);
    //static void setBanExpireTime(const QString &name, int time);
    static void updateMemberInDatabase(const Member &m, bool add);
    static int maxAuth(const QString &ip);

    static void loadMemberInMemory(const QString &name, QObject *o=NULL, const char *slot=NULL);

    static QString ip(const QString &name);

    static QStringList membersForIp(const QString &ip);
    static QHash<QString, std::pair<QString, int> > banList();
    static QStringList authList();
    static QStringList &&userList();
    static void deleteUser(const QString &name);

    static void processDailyRun(int maxdays, bool async=true);
    static void exportDatabase();
private slots:
    static void insertMember(QSqlQuery *q, void *m, int update);
    static void loadMember(QSqlQuery *q, const QVariant &name, int query_type);

    static void dailyRunEx(QSqlQuery *q=0);
private:
    static void loadMembers();
    static void loadSqlMembers();

    static MemoryHolder<Member> holder;

    static QHash<QString, unsigned int> bannedIPs;
    static QHash<QString, std::pair<QString, int> > bannedMembers;

    static WaitingObject* getObject();
    static void freeObject(WaitingObject *c);

    static QSet<WaitingObject*> freeObjects;
    static QSet<WaitingObject*> usedObjects;

    static SecurityManager * instance;

    static LoadInsertThread<Member> *thread;

    static LoadInsertThread<Member> * getThread();

    static QNickValidator val;

    static int dailyRunDays;
    static int lastPlace;
    static QFile memberFile;
    static istringmap<Member> members;

    static QMultiHash<QString, QString> playersByIp;
    static QSet<QString> authed;
};

#endif // SECURITY_H
