#ifndef CHALLENGEDIALOG_H
#define CHALLENGEDIALOG_H

#include <QDialog>

#include "../PokemonInfo/networkstructs.h"
#include "../PokemonInfo/battlestructs.h"

namespace Ui {
    class ChallengeDialog;
}

class QCheckBox;

class ChallengeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChallengeDialog(QWidget *parent = 0);
    ~ChallengeDialog();

    void setPlayerInfo(const PlayerInfo &info);

private:
    Ui::ChallengeDialog *ui;
    PlayerInfo info;
    QCheckBox* clauses[ChallengeInfo::numberOfClauses];
};

#endif // CHALLENGEDIALOG_H
