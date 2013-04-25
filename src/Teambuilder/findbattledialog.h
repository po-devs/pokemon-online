#ifndef FINDBATTLEDIALOG_H
#define FINDBATTLEDIALOG_H

#include <QWidget>

namespace Ui {
    class FindBattleDialog;
}

class TeamHolder;
class TeamLine;
struct FindBattleData;

class FindBattleDialog : public QWidget
{
    Q_OBJECT

public:
    explicit FindBattleDialog(QWidget *parent = 0);
    void setTeam(TeamHolder *t);

    ~FindBattleDialog();
signals:
    void findBattle(const FindBattleData&);
public slots:
    void throwChallenge();
private:
    Ui::FindBattleDialog *ui;
    QList<TeamLine*> teamLines;
};

#endif // FINDBATTLEDIALOG_H
