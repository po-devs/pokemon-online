#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/battlestructs.h"


BaseChallengeWindow::BaseChallengeWindow(const PlayerInfo &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
        : QWidget(parent), emitOnClose(true)
{
    setWindowTitle(windowTitle.arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose, true);

    QGridLayout *mylayout = new QGridLayout(this);
    QPushButton *goback;

    QGroupBox *box = new QGroupBox(tr("Player Info"));
    QVBoxLayout *boxlayout = new QVBoxLayout(box);
    boxlayout->addWidget(new QLabel(p.team.info));

    mylayout->addWidget(box ,0,0,1,2 );

    QVBoxLayout *clausesL= new QVBoxLayout();
    QGroupBox *boxC = new QGroupBox(tr("C&lauses"));
    boxC->setLayout(clausesL);
    clausesL->setSpacing(2);
    mylayout->addWidget(boxC, 1,0,1,2);

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i] = new QCheckBox(ChallengeInfo::clause(i));
        clauses[i]->setToolTip(ChallengeInfo::description(i));
        clausesL->addWidget(clauses[i]);
    }

    mylayout->addWidget(challenge_b = new QPushButton(buttonOk), 2,0);
    mylayout->addWidget(goback = new QPushButton(buttonNo), 2,1);

    connect(goback, SIGNAL(clicked()), SLOT(onCancel()));

    myid = p.id;

    show();

    if (width() < 300) {
	resize(300, height());
    }
}

void BaseChallengeWindow::closeEvent(QCloseEvent *)
{
    if (emitOnClose)
        onCancel();
}

void BaseChallengeWindow::forcedClose()
{
    emitOnClose = false;
    close();
}

int BaseChallengeWindow::id()
{
    return myid;
}


void BaseChallengeWindow::onChallenge()
{
    emit challenge(id());
    myid = -1;
    close();
}

void BaseChallengeWindow::onCancel()
{
    if (id() != -1) {
	emit cancel(id());
    }
    close();
}

ChallengeWindow::ChallengeWindow(const PlayerInfo &p, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1's Info"), tr("&Challenge"), tr("Go &Back"), parent)
{
    QSettings s;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i]->setChecked(s.value("clause_"+ChallengeInfo::clause(i)).toBool());
    }

    //This is necessary to do that here because this is the function of the derived class that is connected then
    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));
}

void ChallengeWindow::onChallenge()
{
    QSettings s;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        s.setValue("clause_"+ChallengeInfo::clause(i), clauses[i]->isChecked());
    }

    emit challenge(id());
    close();
}

ChallengedWindow::ChallengedWindow(const PlayerInfo &p, quint32 clauses, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1 challenged you!"), tr("&Accept"), tr("&Refuse"), parent)
{
    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        this->clauses[i]->setChecked(clauses & (1 << i));
        this->clauses[i]->setDisabled(true);
    }
}
