#ifndef CHALLENGE_H
#define CHALLENGE_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"

class QImageButton;

class PlayerInfo;

class BaseChallengeWindow : public QLabel
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
    QImageButton *challenge_b;
    QCheckBox *clauses[ChallengeInfo::numberOfClauses];
    QComboBox *battleMode;
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

protected:
    void keyPressEvent(QKeyEvent * event);
};

class ChallengedWindow: public BaseChallengeWindow
{
    Q_OBJECT
public:
    ChallengedWindow(const PlayerInfo &p, const ChallengeInfo &c, QWidget *parent = 0);
};

#endif // CHALLENGE_H
