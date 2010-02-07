#ifndef SECURITY_H
#define SECURITY_H

#include <QtNetwork>
#include <QtCore>
#include "../Utilities/otherwidgets.h"

class SecurityManager
{
public:
    /* The file where the members are */
    static const char * path;

    static void init();

    /* A member as stored in the file */
    struct Member {
        Member(const QString &name="", const QString &date = QDate::currentDate().toString(Qt::ISODate), const QString &auth = "000",
               const QString &salt="", const QString &hash="", const QString &ip="");
        QString name;
        QString date;
        QString auth;
        QString salt;
        QString hash;
        QString ip;


        void modifyIP(const QString ip) {
            this->ip = ip.leftJustified(ipLength, ' ', true);
        }

        void modifyDate(const QString &date) {
            this->date = date;
        }

        bool isProtected() const {
            return hash.length() > 0 && hash[0] != ' ';
        }

        bool isBanned() const {
            return auth.length() > 1 && auth[1] == '1';
        }

        int authority() const {
            if (auth.length() < 3) {
                return 0;
            } else {
                return auth[0].toAscii() - '0';
            }
        }

        void setAuth(int _auth) {
            if (auth.length() < 3) {
                auth[0] = '0';
                auth[1] = '0' + _auth;
                auth[2] = '0';
            } else {
                auth[0] = '0' + _auth;
            }
        }

        void ban() {
            if (auth.length() < 3) {
                auth = "010";
            } else {
                auth[1] = '1';
            }
        }

        void unban() {
            if (auth.length() < 3) {
                auth = "000";
            } else {
                auth[1] = '0';
            }
        }

        void clearPass() {
            hash = QString().leftJustified(hashLength);
        }

        static const int saltLength = 7;
        static const int hashLength = 32;
        static const int ipLength = 39; //IPv6 is 39, so lets be ready for the future

        void write(QIODevice *device) const;
    };

    static bool isValid(const QString &name);
    static bool exist(const QString &name);
    static void create (const Member &m);
    static Member member(const QString &name) {
        return members[name.toLower()];
    }
    //Change the member in the file
    static void updateMember(const Member &m);
    //Just change in memory
    static void updateMemory(const Member &m);

    static bool bannedIP(const QString &ip);
    static void ban(const QString &name);
    static void unban(const QString &name);
    static void IPunban(const QString &ip);
    static void setauth(const QString &name, int auth);
    static void clearPass(const QString &name);
    static int maxAuth(const QString &ip);

    static QString ip(const QString &name);

    static QMap<QString, Member> getMembers();
    static QList<QString> membersForIp(const QString &ip);
    static QSet<QString> banList();

private:
    static void loadMembers();
    static QMap<QString, Member> members;
    static QHash<QString, int> memberPlaces;
    static QSet<QString> bannedIPs;
    static QSet<QString> bannedMembers;
    static QMultiMap<QString, QString> playersByIp;
    static int lastPlace;
    static QFile memberFile;
    static QNickValidator val;
};

#endif // SECURITY_H
