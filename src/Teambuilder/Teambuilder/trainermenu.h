#ifndef TRAINERMENU_H
#define TRAINERMENU_H

#include <QFrame>
#include <QCompleter>
#include "Teambuilder/teambuilderwidget.h"

namespace Ui {
    class TrainerMenu;
}

class QPushButton;
class TeamHolder;

class TrainerMenu : public TeamBuilderWidget
{
    Q_OBJECT

public:
    TrainerMenu(TeamHolder *team);
    ~TrainerMenu();

    void updateAll();
    void updateTeam();
    void setTiers(const QStringList &tiers);

public slots:
    void updateCurrentTeamAndNotify();
    void importTeam(const QString &team);
    void openImportDialog();
    void openTeam();

signals:
    void done();
    void openBoxes();
    void editPoke(int);

private slots:
    void on_close_clicked(){ emit done(); }
    void on_name_textEdited();
    void on_winningMessage_textEdited();
    void on_losingMessage_textEdited();
    void on_tieMessage_textEdited();
    void on_teamName_textEdited();
    void on_infos_textChanged();
    void on_saveProfile_clicked();
    void on_colorButton_clicked();
    void setAvatarPixmap();
    void on_deleteProfile_clicked();
    void changeCurrentTeam(int);
    void on_addTeam_clicked();
    void on_removeTeam_clicked();
    void on_saveTeam_clicked();
    void on_loadTeam_clicked();
    void on_importTeam_clicked();
    void on_teamFolderButton_clicked();
    void on_profileList_currentIndexChanged(int);
    void on_newProfile_clicked();
    void on_teamTier_textEdited();
    void setTier(const QString &tier);

private:
    void setupData();
    void updateData();
    void updateTeamButtons();
    void setColor();
    void loadProfileList();
    void updateButtonName();

    Ui::TrainerMenu *ui;
    QPushButton *teamButtons[6];

    TeamHolder *m_team;
    TeamHolder &team() {return *m_team;}
    const TeamHolder &team() const {return *m_team;}
};

#endif // TRAINERMENU_H
