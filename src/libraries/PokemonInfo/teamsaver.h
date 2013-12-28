#ifndef TEAMSAVER_H
#define TEAMSAVER_H

#include <QtCore>

class Team;

/* Temporary object to manage team saving */
class TeamSaver : public QObject
{
    Q_OBJECT
public:
    TeamSaver(Team *t);
public slots:
    void fileNameReceived(const QString &name);
    void fileNameReceivedL(const QString &name);
private:
    Team *t;
};


#endif // TEAMSAVER_H
