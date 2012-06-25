#ifndef TEAMLINE_H
#define TEAMLINE_H

#include <QWidget>

namespace Ui {
    class TeamLine;
}

class Team;

class TeamLine : public QWidget
{
    Q_OBJECT

public:
    explicit TeamLine(QWidget *parent = 0);
    ~TeamLine();

    bool isChecked() const;
    void setChecked(bool checked);
    void setTeamTier(const Team &team, const QString &tier);
private:
    Ui::TeamLine *ui;
};

#endif // TEAMLINE_H
