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
        Member(const QString &name="", const QString &date = QDate::currentDate().toString(Qt::ISODate), const QString &salt="", const QString &hash="", const QString &ip="");
        QString name;
        QString date;
        QString salt;
        QString hash;
        QString ip;

        void modifyIP(const QString ip) {
            this->ip = ip.leftJustified(ipLength);
        }

        void modifyDate(const QString &date) {
            this->date = date;
        }

        bool isProtected() const {
            return hash.length() > 0 && hash[0] != ' ';
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
        return members[name];
    }
    //Change the member in the file
    static void updateMember(const Member &m);
    //Just change in memory
    static void updateMemory(const Member &m);

private:
    static void loadMembers();
    static QHash<QString, Member> members;
    static QHash<QString, int> memberPlaces;
    static int lastPlace;
    static QFile memberFile;
    static QNickValidator val;
};

#endif // SECURITY_H
