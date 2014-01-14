#ifndef TRAINERBODY_H
#define TRAINERBODY_H

#include <QWidget>

/* This is the widget displaying a trainer's info */

class QLineEdit;
class QPlainTextEdit;
class AvatarBox;
class QPushButton;
class QSpinBox;
class TeamHolder;

class TB_TrainerBody : public QWidget
{
    Q_OBJECT
public:
    TB_TrainerBody(TeamHolder *team);

    void updateTrainer();
    void setTierList(const QStringList &);
private slots:
    void changeTrainerInfo();
    void setTrainerNick(const QString &);
    void changeTrainerWin();
    void changeTrainerLose();
    void changeTrainerAvatar(int);
    void changeTrainerColor();
    void changeTier(const QString &);
private:
    QLineEdit *m_nick;
    QLineEdit *m_tier;
    QPlainTextEdit *m_winMessage, *m_loseMessage, *m_trainerInfo;
    QPushButton *m_colorButton;
    AvatarBox *m_avatar;
    QSpinBox *m_avatarSelection;

    TeamHolder *m_team;
    TeamHolder* trainerTeam();
};

#endif // TRAINERBODY_H
