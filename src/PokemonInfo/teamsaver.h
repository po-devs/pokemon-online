#ifndef TEAMSAVER_H
#define TEAMSAVER_H

#include <QtCore>

class TrainerTeam;

/* Temporary object to manage team saving */
class TeamSaver : public QObject
{
    Q_OBJECT
public:
    TeamSaver(TrainerTeam *t);
public slots:
    void fileNameReceived(const QString &name);
    void fileNameReceivedL(const QString &name);
private:
    TrainerTeam *t;
};


#endif // TEAMSAVER_H
