#include <QCheckBox>
#include <QRadioButton>

#include "challengedialog.h"
#include "ui_challengedialog.h"
#include "tierratingbutton.h"
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
    ui->description->setText(info.info);

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i] = new QCheckBox(ChallengeInfo::clause(i));
        clauses[i]->setToolTip(ChallengeInfo::description(i));
        ui->clauses->addWidget(clauses[i]);
    }

    ui->tierContainer->setFixedHeight(ui->avatar->height());;
    QGridLayout *tiers = (QGridLayout*) ui->tierContainer->layout();
    int cpt = 0;
    QButtonGroup* tierGroup = new QButtonGroup(this);
    foreach(QString s, info.ratings.keys()) {
        TierRatingButton *b = new TierRatingButton(s, info.ratings[s]);

        if (cpt == 0) {
            b->setChecked(true);
        }

        tierGroup->addButton(b);

        tiers->addWidget(b, cpt/3, cpt%3);

        cpt++;
    }

    for (int i = 0; i < 3; i++) {
        tiers->setColumnStretch(i, 1);
    }
    for (int i = 0; i < 2; i++) {
        tiers->setRowStretch(i, 1);
    }
}

ChallengeDialog::~ChallengeDialog()
{
    delete ui;
}
