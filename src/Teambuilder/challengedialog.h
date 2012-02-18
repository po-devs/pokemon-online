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
    ~ChallengeDialog();

    void setPlayerInfo(const PlayerInfo &info);
    void setTeam(TeamHolder *t);
private slots:
    void changeCurrentTeam();
private:
    Ui::ChallengeDialog *ui;
    QCheckBox* clauses[ChallengeInfo::numberOfClauses];
    QLabel *pokes[6];

    PlayerInfo info;
    TeamHolder *team;

    void updateCurrentTeam();
};

#endif // CHALLENGEDIALOG_H
