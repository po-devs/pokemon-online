#include "trainermenu.h"
#include "ui_trainermenu.h"
#include "teamholder.h"
#include "../Utilities/otherwidgets.h"
#include "theme.h"
#include <QColorDialog>

TrainerMenu::TrainerMenu(TeamHolder *team) :
    ui(new Ui::TrainerMenu), m_team(team)
{
    ui->setupUi(this);
    ui->name->setValidator(new QNickValidator(this));

    connect(ui->pokemonButtons, SIGNAL(teamChanged()), SIGNAL(teamChanged()));
    connect(ui->pokemonButtons, SIGNAL(doubleClicked(int)), SIGNAL(editPoke(int)));

    setupData();
}

void TrainerMenu::updateAll()
{
    setupData();
}

void TrainerMenu::setupData()
{
    ui->name->setText(team().name());
    setColor();
    ui->infos->setPlainText(team().info().info);
    ui->winningMessage->setText(team().info().winning);
    ui->losingMessage->setText(team().info().losing);
    ui->tieMessage->setText(team().info().tie);
    ui->avatarNumber->setValue(team().info().avatar);
    setAvatarPixmap();

    updateTeam();
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

void TrainerMenu::on_saveProfile_clicked()
{
    QSettings s;
    team().profile().saveToFile(s.value("profile_location").toString());
}

void TrainerMenu::on_loadProfile_clicked()
{
    QSettings s;
    team().profile().loadFromFile(s.value("profile_location").toString());
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
