#ifndef CHALLENGE_H
#define CHALLENGE_H

#include <QtGui>

class Player;

class ChallengeWindow : public QWidget
{
    Q_OBJECT
public:
    ChallengeWindow(const Player &p, QWidget *parent=0);
signals:
    void challenge(int id);
public slots:
    void onChallenge();
private:
    int id;
};

#endif // CHALLENGE_H
