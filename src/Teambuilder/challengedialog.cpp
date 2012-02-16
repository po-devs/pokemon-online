#include "challengedialog.h"
#include "ui_challengedialog.h"
#include "theme.h"

ChallengeDialog::ChallengeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChallengeDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

void ChallengeDialog::setPlayerInfo(const PlayerInfo &info)
{
    this->info = info;

    setWindowTitle(tr("%1's info").arg(info.name));
    ui->name->setText(info.name);
    ui->avatar->setPixmap(Theme::TrainerSprite(info.avatar));
    ui->avatar->setFixedSize(Theme::TrainerSprite(1).size());
}

ChallengeDialog::~ChallengeDialog()
{
    delete ui;
}
