#include <QColorDialog>
#include <QMessageBox>
#include <QFileDialog>

#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/teamsaver.h"
#include "../PokemonInfo/teamholder.h"

#include "Teambuilder/trainermenu.h"
#include "ui_trainermenu.h"
#include "theme.h"
#include "Teambuilder/teamimporter.h"

TrainerMenu::TrainerMenu(TeamHolder *team) :
    ui(new Ui::TrainerMenu), m_team(team)
{
    ui->setupUi(this);
    Theme::ToolButtonIcon(ui->teamFolderButton, Theme::ChangeTeamFolder);
    Theme::ToolButtonIcon(ui->removeTeam, Theme::DeleteTeam);
    Theme::ToolButtonIcon(ui->saveTeam, Theme::SaveTeam);
    Theme::ToolButtonIcon(ui->loadTeam, Theme::LoadTeam);
    Theme::ToolButtonIcon(ui->importTeam, Theme::ImportTeam);
    Theme::ToolButtonIcon(ui->addTeam, Theme::AddTeam);
    ui->name->setValidator(new QNickValidator(this));
    ui->close->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));

    QPushButton *buttons[6] = {ui->team1, ui->team2, ui->team3, ui->team4, ui->team5, ui->team6};
    memcpy(teamButtons, buttons, sizeof(buttons));

    QButtonGroup *bg = new QButtonGroup(this);
    for (int i = 0; i < 6; i++) {
        buttons[i]->setCheckable(true);
        bg->addButton(buttons[i], i);
    }
    buttons[0]->setChecked(true);

    connect(bg, SIGNAL(buttonClicked(int)), SLOT(changeCurrentTeam(int)));

    connect(ui->pokemonButtons, SIGNAL(teamChanged()), SIGNAL(teamChanged()));
    connect(ui->boxCenter, SIGNAL(clicked()), SIGNAL(openBoxes()));
    //connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SIGNAL(editPoke(int)));
    connect(ui->pokemonButtons, SIGNAL(clicked(int)), SIGNAL(editPoke(int))); //I prefer double click, but for newbies...
    connect(ui->teambuilderLabel, SIGNAL(clicked()), SLOT(openTeam()));

    loadProfileList();
    setupData();
}

void TrainerMenu::changeCurrentTeam(int t)
{
    if (t == team().currentTeam()) {
        return;
    }
    teamButtons[t]->setChecked(true);
    team().setCurrent(t);
    updateTeam();

    emit teamChanged();
}

void TrainerMenu::openTeam()
{
    emit editPoke(ui->pokemonButtons->currentSlot());
}

void TrainerMenu::on_teamName_textEdited()
{
    team().team().setName(ui->teamName->text());
    updateButtonName();
}

void TrainerMenu::openImportDialog()
{
    on_importTeam_clicked();
}

void TrainerMenu::on_importTeam_clicked()
{
    TeamImporter *i = new TeamImporter();
    i->show();

    connect(this, SIGNAL(destroyed()), i, SLOT(deleteLater()));

    connect(i, SIGNAL(done(QString)), SLOT(importTeam(QString)));
    connect(i, SIGNAL(done(QString)), SLOT(updateCurrentTeamAndNotify()));
}

void TrainerMenu::importTeam(const QString &team)
{
    this->team().team().importFromTxt(team);
}

void TrainerMenu::updateTeamButtons()
{
    for (int i = 0; i < team().count(); i++) {
        teamButtons[i]->setVisible(true);
        teamButtons[i]->setText(team().team(i).name().isEmpty() ? tr("Untitled", "Team name") : team().team(i).name());
    }

    for (int i = team().count(); i < 6; i++) {
        teamButtons[i]->setVisible(false);
    }

    teamButtons[team().currentTeam()]->setChecked(true);

    ui->removeTeam->setDisabled(team().count() <= 1);
    ui->addTeam->setDisabled(team().count() >= 6);
}

void TrainerMenu::updateButtonName()
{
    teamButtons[team().currentTeam()]->setText(ui->teamName->text().isEmpty() ? tr("Untitled", "Team name") : ui->teamName->text());
}

void TrainerMenu::on_addTeam_clicked()
{
    if (team().count() >= 6) {
        return;
    }

    team().addTeam();

    updateTeamButtons();
}

void TrainerMenu::on_removeTeam_clicked()
{
    team().removeTeam();

    updateTeamButtons();
    updateTeam();

    emit teamChanged();
}

void TrainerMenu::on_saveTeam_clicked()
{
    //Updates name of team if changed
    saveTTeamDialog(team().team(), this, SLOT(on_teamName_textEdited()));
}

void TrainerMenu::on_loadTeam_clicked()
{
    //Updates name of team if changed

    loadTTeamDialog(team().team(), this, SLOT(updateCurrentTeamAndNotify()));
}

void TrainerMenu::updateCurrentTeamAndNotify()
{
    updateTeam();
    emit teamChanged();
}

void TrainerMenu::updateAll()
{
    setupData();
}

void TrainerMenu::setupData()
{
    updateData();
    updateTeam();
    updateTeamButtons();
}

void TrainerMenu::updateData()
{
    ui->name->setText(team().name());
    setColor();
    ui->infos->setPlainText(team().info().info);
    ui->winningMessage->setText(team().info().winning);
    ui->losingMessage->setText(team().info().losing);
    ui->tieMessage->setText(team().info().tie);
    ui->avatarNumber->setValue(team().info().avatar);
    setAvatarPixmap();
}


void TrainerMenu::updateTeam()
{
    ui->pokemonButtons->setTeam(team().team());
    ui->teamName->setText(team().team().name());
    ui->teamTier->setText(team().team().defaultTier());
    updateButtonName();
}

void TrainerMenu::on_teamTier_textEdited()
{
    team().team().defaultTier() = ui->teamTier->text();
}

void TrainerMenu::on_teamFolderButton_clicked()
{
    QSettings s;

    QString folder = QFileDialog::getExistingDirectory(NULL, tr("Folder in which to save the team"), team().team().folder().isEmpty() ?
                                                           s.value("Teams/Folder").toString() : team().team().folder());

    if (folder.length() > 0) {
        team().team().setFolder(folder);

        QSettings s;
        s.setValue("Teams/Folder", folder);
    }
}

void TrainerMenu::setAvatarPixmap()
{
    ui->avatar->setPixmap(Theme::TrainerSprite(ui->avatarNumber->value()));
    team().profile().info().avatar = ui->avatarNumber->value();
}

void TrainerMenu::setTiers(const QStringList &tiers) {
    QCompleter *m_completer = new QCompleter(tiers);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->teamTier->setCompleter(m_completer);

    connect(m_completer, SIGNAL(highlighted(QString)), this, SLOT(setTier(QString)));
}

void TrainerMenu::setTier(const QString &tier) {
    team().team().defaultTier() = tier;
}

void TrainerMenu::setColor()
{
    if (team().color().isValid()) {
        ui->colorButton->setStyleSheet(QString("background: %1; color: white;").arg(team().color().name()));
    } else {
        ui->colorButton->setStyleSheet("");
    }
}

void TrainerMenu::on_name_textEdited()
{
    team().name() = ui->name->text();
}

void TrainerMenu::on_losingMessage_textEdited()
{
    team().info().losing = ui->losingMessage->text();
}

void TrainerMenu::on_tieMessage_textEdited()
{
    team().info().tie = ui->tieMessage->text();
}

void TrainerMenu::on_winningMessage_textEdited()
{
    team().info().winning = ui->winningMessage->text();
}

void TrainerMenu::on_infos_textChanged()
{
    team().info().info = ui->infos->toPlainText();
}

void TrainerMenu::loadProfileList()
{
    QSettings s;
    ui->profileList->blockSignals(true);
    ui->profileList->addItems(team().profile().getProfileList(s.value("Profile/Path").toString()));
    ui->profileList->blockSignals(false);

    for (int i = 0; i < ui->profileList->count(); i++) {
        if (ui->profileList->itemText(i) == team().profile().name()) {
            ui->profileList->setCurrentIndex(i);
            return;
        }
    }
}

void TrainerMenu::on_profileList_currentIndexChanged(int)
{
    if (ui->profileList->currentText() == team().name()) {
        return;
    }
    QSettings s;
    QString path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(ui->profileList->currentText()) + ".xml";
    s.setValue("Profile/Current", path);
    team().profile().loadFromFile(path);
    updateData();
}

void TrainerMenu::on_saveProfile_clicked()
{
    QSettings s;
    QString path;
    if(ui->name->text().isEmpty()) {
        QMessageBox::warning(0, tr("Saving a Profile"), tr("You don't have any nickname for the profile."));
        return;
    }
    if(ui->profileList->findText(ui->name->text()) == -1) {
        path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(ui->name->text()) + ".xml";
    } else {
        path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(ui->profileList->currentText()) + ".xml";
    }
    if(team().profile().saveToFile(path)) {
        // We need to re-do the check again since if name contains illegal char, why we would want to add it
        // before checking if it was successfully saved.
        if(ui->profileList->findText(ui->name->text()) == -1) {
            ui->profileList->addItem(ui->name->text());
            ui->profileList->setCurrentIndex(ui->profileList->findText(ui->name->text()));
        }
        s.setValue("Profile/Current", path);
    }
}

void TrainerMenu::on_deleteProfile_clicked()
{
    if(ui->profileList->currentText().isNull()) {
        QMessageBox::warning(0, tr("Deleting a Profile"), tr("There's no selected profile to delete."));
        return;
    }
    QSettings s;
    QString path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(ui->profileList->currentText()) + ".xml";
    ui->profileList->removeItem(ui->profileList->currentIndex());
    team().profile().deleteProfile(path);
}

void TrainerMenu::on_newProfile_clicked()
{
    ui->name->clear();
    ui->infos->clear();
    ui->winningMessage->clear();
    ui->losingMessage->clear();
    ui->tieMessage->clear();
    ui->avatarNumber->setValue(1);
    ui->colorButton->setStyleSheet(QString("background: white; color: black;"));
}

void TrainerMenu::on_colorButton_clicked()
{
    QColor c = QColorDialog::getColor(team().color());

    if (c.isValid() && (c.lightness() > 140 || c.green() > 180)) {
        return;
    }

    team().color() = c;
    setColor();
}

TrainerMenu::~TrainerMenu()
{
    delete ui;
}
