#include <QCheckBox>
#include <QRadioButton>

#include "challengedialog.h"
#include "ui_challengedialog.h"
#include "tierratingbutton.h"
#include "theme.h"
#include "Teambuilder/teamholder.h"
#include "../PokemonInfo/pokemoninfo.h"

ChallengeDialog::ChallengeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChallengeDialog),
    emitOnClose(true), challenging(false)
{
    init();
    show();
}

ChallengeDialog::ChallengeDialog(const PlayerInfo &info, TeamHolder *t, int mid, int challengeId) :
    ui(new Ui::ChallengeDialog),
    emitOnClose(true), challenging(false)
{
    init();

    myid = mid;
    challId = challengeId;

    setPlayerInfo(info);
    setTeam(t);

    show();
}

void ChallengeDialog::init()
{
    ui->setupUi(this);
    QLabel *pokes [6] = {ui->poke1, ui->poke2, ui->poke3, ui->poke4, ui->poke5, ui->poke6};
    memcpy(this->pokes, pokes, sizeof(pokes));

    setAttribute(Qt::WA_DeleteOnClose);

    loadSettings(this, QSize(700, 320));

    connect (ui->challenge, SIGNAL(clicked()), SLOT(onChallenge()));
    connect (ui->refuse, SIGNAL(clicked()), SLOT(onCancel()));
}

void ChallengeDialog::setPlayerInfo(const PlayerInfo &info)
{
    this->info = info;

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
    tierGroup = new QButtonGroup(this);
    foreach(QString s, info.ratings.keys()) {
        TierRatingButton *b = new TierRatingButton(s, info.flags[PlayerInfo::LadderEnabled] ? info.ratings[s] : -1);
        b->setProperty("tier", s);

        if (cpt == 0) {
            b->setChecked(true);
        }

        tierGroup->addButton(b,cpt);

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

void ChallengeDialog::setTeam(TeamHolder *t)
{
    this->team = t;

    QMenu *m = new QMenu(ui->teamChoice);
    for (int i = 0; i < team->officialCount(); i++) {
        if (cinfo.desttier.isEmpty() || team->tier(i) == cinfo.desttier) {
            QAction *a = m->addAction(QString ("%1 (%2)").arg(team->team(i).name(), team->tier(i)), this, SLOT(changeCurrentTeam()));
            a->setProperty("slot", i);

            if (team->tier() != cinfo.desttier && !cinfo.desttier.isEmpty()) {
                team->setCurrent(i);
            }
        }
    }
    ui->teamChoice->setMenu(m);

    updateCurrentTeam();
}

void ChallengeDialog::updateCurrentTeam()
{
    ui->teamChoice->setText(QString ("%1 (%2)").arg(team->team().name(), team->tier()));

    for (int i = 0; i < 6; i++) {
        pokes[i]->setPixmap(PokemonInfo::Icon(team->team().poke(i).num()));
    }
}

ChallengeDialog::~ChallengeDialog()
{
    writeSettings(this);

    delete ui;
}

void ChallengeDialog::changeCurrentTeam()
{
    int slot = sender()->property("slot").toInt();

    slot = std::min(slot, team->officialCount());
    team->setCurrent(slot);

    updateCurrentTeam();
}

int ChallengeDialog::id()
{
    return info.id;
}

int ChallengeDialog::cid()
{
    return challId;
}


void ChallengeDialog::closeEvent(QCloseEvent *)
{
    if (emitOnClose)
        onCancel();
}

void ChallengeDialog::forcedClose()
{
    emitOnClose = false;
    close();
}

void ChallengeDialog::onChallenge()
{
    if (challenging) {
        saveData();
        cinfo.dsc = ChallengeInfo::Sent;
        cinfo.opp = id();
        cinfo.mode = ui->mode->currentIndex();
        cinfo.clauses = 0;
        if (info.ratings.size() > 0) {
            cinfo.desttier = tierGroup->checkedButton()->property("tier").toString();
        }
        for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
            cinfo.clauses |= clauses[i]->isChecked() << i;
        }
    } else {
        cinfo.dsc = ChallengeInfo::Accepted;
    }
    cinfo.team = team->currentTeam();

    emit challenge(cinfo);
    emitOnClose = false;
    close();
}

void ChallengeDialog::onCancel()
{
    cinfo.dsc = ChallengeInfo::Refused;

    emit cancel(cinfo);
    close();
}

void ChallengeDialog::setChallengeInfo(const ChallengeInfo &info)
{
    setWindowTitle(tr("%1 challenged you to the %2 tier!").arg(this->info.name, info.desttier));
    this->cinfo = info;

    setTierChecked(info.srctier);

    ui->tierContainer->setDisabled(true);

    setClauses(info.clauses);
    setMode(info.mode);

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i]->setDisabled(true);
    }

    ui->mode->setDisabled(true);

    //updates the teams you can choose depending on the tier you were challenged in
    setTeam(team);

    ui->challenge->setText(tr("Accept", "Challenge"));
    ui->refuse->setText(tr("Decline", "Challenge"));
}

void ChallengeDialog::setClauses(quint32 clauses)
{
    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        this->clauses[i]->setChecked(clauses & (1 << i));
    }
}

void ChallengeDialog::setMode(int mode)
{
    ui->mode->setCurrentIndex(mode);
}

void ChallengeDialog::setChallenging(const QString &tier)
{
    setWindowTitle(tr("%1's info").arg(info.name));

    challenging = true;

    QSettings s;
    setMode(s.value("Challenge/Mode").toInt());

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i]->setChecked(s.value("Challenge/Clause/"+ChallengeInfo::clause(i)).toBool());
    }

    setTierChecked(tier.length() == 0 ? s.value("Challenge/Tier").toString() : tier);

    if (challenging && info.id == myid) {
        ui->challenge->setEnabled(false);
    }
}

void ChallengeDialog::setTierChecked(const QString &tier)
{
    for (int i = 0; i < info.ratings.size(); i++) {
        if (tierGroup->button(i)->property("tier").toString() == tier) {
            tierGroup->button(i)->setChecked(true);
            break;
        }
    }
}

void ChallengeDialog::saveData()
{
    QSettings s;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        s.setValue("Challenge/Clause/"+ChallengeInfo::clause(i), clauses[i]->isChecked());
    }

    s.setValue("Challenge/Mode", ui->mode->currentIndex());

    if (tierGroup->checkedButton()) {
        s.setValue("Challenge/Tier", tierGroup->checkedButton()->property("tier").toString());
    }
}
