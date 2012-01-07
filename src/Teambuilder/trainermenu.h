#ifndef TRAINERMENU_H
#define TRAINERMENU_H

#include <QFrame>

namespace Ui {
    class TrainerMenu;
}

class TeamHolder;

class TrainerMenu : public QFrame
{
    Q_OBJECT

public:
    TrainerMenu(TeamHolder *team);
    ~TrainerMenu();

    void updateAll();
    void updateTeam();
signals:
    void done();
    void teamChanged();
private slots:
    void on_close_clicked(){emit done();}
    void on_name_textEdited();
    void on_winningMessage_textEdited();
    void on_losingMessage_textEdited();
    void on_tieMessage_textEdited();
    void on_infos_textChanged();
    void on_saveProfile_clicked();
    void on_loadProfile_clicked();
    void on_colorButton_clicked();
    void setAvatarPixmap();
private:
    void setupData();
    void setColor();

    Ui::TrainerMenu *ui;
    TeamHolder *m_team;
    TeamHolder &team() {return *m_team;}
    const TeamHolder &team() const {return *m_team;}
};

#endif // TRAINERMENU_H
