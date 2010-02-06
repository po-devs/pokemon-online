#ifndef CHALLENGE_H
#define CHALLENGE_H

#include <QtGui>

class Player;

class BaseChallengeWindow : public QWidget
{
    Q_OBJECT
public:
    BaseChallengeWindow(const Player &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent=0);

    int id();
    /* defined once again so we can make a distinction between user closure and programmed closure */
    void closeEvent(QCloseEvent *event);
signals:
    void challenge(int id);
    void cancel(int id);
public slots:
    void onChallenge();
    void onCancel();
protected:
    QCheckBox *sleepClause;
    QPushButton *challenge_b;
private:
    int myid;
};

class ChallengeWindow : public BaseChallengeWindow
{
    Q_OBJECT
public:
    ChallengeWindow(const Player &p, QWidget *parent=0);
protected slots:
    void onChallenge();
};

class ChallengedWindow: public BaseChallengeWindow
{
    Q_OBJECT
public:
    ChallengedWindow(const Player &p, bool slpCls, QWidget *parent = 0);
};

#endif // CHALLENGE_H
