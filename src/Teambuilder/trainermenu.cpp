#include "trainermenu.h"
#include "ui_trainermenu.h"
#include "teamholder.h"
#include "../Utilities/otherwidgets.h"
#include "theme.h"
#include <QColorDialog>
#include <QMessageBox>

TrainerMenu::TrainerMenu(TeamHolder *team) :
    ui(new Ui::TrainerMenu), m_team(team)
{
    ui->setupUi(this);
    ui->name->setValidator(new QNickValidator(this));

    connect(ui->pokemonButtons, SIGNAL(teamChanged()), SIGNAL(teamChanged()));
    connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SIGNAL(editPoke(int)));

    loadProfileList();
    setupData();
}

void TrainerMenu::updateAll()
{
    setupData();
}

void TrainerMenu::setupData()
{
    updateData();
    updateTeam();
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
}

void TrainerMenu::setAvatarPixmap()
{
    ui->avatar->setPixmap(Theme::TrainerSprite(ui->avatarNumber->value()));
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
    ui->profileList->addItems(team().profile().getProfileList(s.value("profiles_path").toString()));
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
        path = s.value("profiles_path").toString() + "/" + ui->name->text() + ".xml";
    } else {
        path = s.value("profiles_path").toString() + "/" + ui->profileList->currentText() + ".xml";
    }
    if(team().profile().saveToFile(path)) {
        // We need to re-do the check again since if name contains illegal char, why we would want to add it
        // before checking if it was successfully saved.
        if(ui->profileList->findText(ui->name->text()) == -1) {
            ui->profileList->addItem(ui->name->text());
        }
        s.setValue("current_profile", path);
    }
}

void TrainerMenu::on_loadProfile_clicked()
{
    if(ui->profileList->currentText().isNull()) {
        QMessageBox::warning(0, tr("Loading a Profile"), tr("There's no profile selected."));
        return;
    }
    QSettings s;
    QString path = s.value("profiles_path").toString() + "/" + ui->profileList->currentText() + ".xml";
    s.setValue("current_profile", path);
    team().profile().loadFromFile(path);
    updateData();
}

void TrainerMenu::on_deleteProfile_clicked()
{
    if(ui->profileList->currentText().isNull()) {
        QMessageBox::warning(0, tr("Deleting a Profile"), tr("There's no selected profile to delete."));
        return;
    }
    QSettings s;
    QString path = s.value("profiles_path").toString() + "/" + ui->profileList->currentText() + ".xml";
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
