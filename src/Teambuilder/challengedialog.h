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

class ChallengeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChallengeDialog(QWidget *parent = 0);
    ChallengeDialog(const PlayerInfo &info, TeamHolder *t);
    ~ChallengeDialog();

    void setPlayerInfo(const PlayerInfo &info);
    void setTeam(TeamHolder *t);
    void setChallengeInfo(const ChallengeInfo &info);
    void setChallenging();

    int id();
    /* defined once again so we can make a distinction between user closure and programmed closure */
    void closeEvent(QCloseEvent *event);
    /* Won't emit refused signals when closed */
    void forcedClose();
signals:
    void challenge(int id, const ChallengeInfo &c);
    void cancel(int id, const ChallengeInfo &c);
public slots:
    void onChallenge();
    void onCancel();
protected slots:
    void changeCurrentTeam();
protected:
    Ui::ChallengeDialog *ui;
    QCheckBox* clauses[ChallengeInfo::numberOfClauses];
    QLabel *pokes[6];

    PlayerInfo info;
    ChallengeInfo cinfo;
    TeamHolder *team;

    bool emitOnClose, challenging;

    void updateCurrentTeam();
    void setClauses(quint32 clauses);
    void setMode(int mode);
    void init();
    void saveData();
};

#endif // CHALLENGEDIALOG_H
