#ifndef CHALLENGE_H
#define CHALLENGE_H

#include <QtGui>

class PlayerInfo;

class BaseChallengeWindow : public QWidget
{
    Q_OBJECT
public:
    BaseChallengeWindow(const PlayerInfo &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent=0);

    int id();
    /* defined once again so we can make a distinction between user closure and programmed closure */
    void closeEvent(QCloseEvent *event);
    /* Won't emit refused signals when closed */
    void forcedClose();
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

    bool emitOnClose;
};

class ChallengeWindow : public BaseChallengeWindow
{
    Q_OBJECT
public:
    ChallengeWindow(const PlayerInfo &p, QWidget *parent=0);
protected slots:
    void onChallenge();
};

class ChallengedWindow: public BaseChallengeWindow
{
    Q_OBJECT
public:
    ChallengedWindow(const PlayerInfo &p, bool slpCls, QWidget *parent = 0);
};

#endif // CHALLENGE_H
