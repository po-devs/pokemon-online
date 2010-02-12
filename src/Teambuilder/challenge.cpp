#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"

BaseChallengeWindow::BaseChallengeWindow(const Player &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
        : QWidget(parent), emitOnClose(true)
{
    setWindowTitle(windowTitle.arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose, true);

    QGridLayout *mylayout = new QGridLayout(this);
    QPushButton *goback;

    QGroupBox *box = new QGroupBox(tr("Player Info"));
    QVBoxLayout *boxlayout = new QVBoxLayout(box);
    boxlayout->addWidget(new QLabel(p.team.info));

    sleepClause = new QCheckBox(tr("&Sleep Clause"));

    mylayout->addWidget(box ,0,0,1,2 );
    mylayout->addWidget(sleepClause, 1,0,1,2);
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

ChallengeWindow::ChallengeWindow(const Player &p, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1's Info"), tr("&Challenge"), tr("Go &Back"), parent)
{
    QSettings s;
    bool slpCls = s.value("sleep_clause").toBool();

    sleepClause->setChecked(slpCls);
    
    //This is necessary to do that here because this is the function of the derived class that is connected then
    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));
}

void ChallengeWindow::onChallenge()
{
    QSettings s;
    s.setValue("sleep_clause", sleepClause->isChecked());

    emit challenge(id());
    close();
}

ChallengedWindow::ChallengedWindow(const Player &p, bool slpCls, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1 challenged you!"), tr("&Accept"), tr("&Refuse"), parent)
{
    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));

    sleepClause->setChecked(slpCls);
    sleepClause->setDisabled(true);
}
