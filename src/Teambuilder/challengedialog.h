#ifndef CHALLENGEDIALOG_H
#define CHALLENGEDIALOG_H

#include <QDialog>

#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"

namespace Ui {
    class ChallengeDialog;
}

class QCheckBox;
class TeamHolder;
class QLabel;
class QButtonGroup;

class ChallengeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChallengeDialog(QWidget *parent = 0);
    ChallengeDialog(const PlayerInfo &info, TeamHolder *t, int mid, int challengeId);
    ~ChallengeDialog();

    /* Get the challenge info */
    ChallengeInfo challengeInfo() { return cinfo; }

    void setPlayerInfo(const PlayerInfo &info);
    void setTeam(TeamHolder *t);
    void setChallengeInfo(const ChallengeInfo &info);
    void setChallenging(const QString &tier = "");

    int id();
    int cid();
    /* defined once again so we can make a distinction between user closure and programmed closure */
    void closeEvent(QCloseEvent *event);
    /* Won't emit refused signals when closed */
    void forcedClose();
signals:
    void challenge(const ChallengeInfo &c);
    void cancel(const ChallengeInfo &c);
public slots:
    void onChallenge();
    void onCancel();
protected slots:
    void changeCurrentTeam();
protected:
    Ui::ChallengeDialog *ui;
    QCheckBox* clauses[ChallengeInfo::numberOfClauses];
    QLabel *pokes[6];
    QButtonGroup *tierGroup;

    int myid;
    int challId;

    PlayerInfo info;
    ChallengeInfo cinfo;
    TeamHolder *team;

    bool emitOnClose, challenging;

    void updateCurrentTeam();
    void setClauses(quint32 clauses);
    void setMode(int mode);
    void init();
    void saveData();
    void setTierChecked(const QString &tier);
};

#endif // CHALLENGEDIALOG_H
