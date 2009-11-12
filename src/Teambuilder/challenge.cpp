#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"


BaseChallengeWindow::BaseChallengeWindow(const Player &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
	: QWidget(parent)
{
    setWindowTitle(windowTitle.arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose, true);

    QGridLayout *mylayout = new QGridLayout(this);
    QPushButton *challenge, *goback;

    QGroupBox *box = new QGroupBox(tr("Player Info"));
    QVBoxLayout *boxlayout = new QVBoxLayout(box);
    boxlayout->addWidget(new QLabel(p.team.info));

    mylayout->addWidget( box ,0,0,1,2 );
    mylayout->addWidget( challenge = new QPushButton(buttonOk), 1,0);
    mylayout->addWidget( goback = new QPushButton(buttonNo), 1,1);

    connect(challenge, SIGNAL(clicked()), SLOT(onChallenge()));
    connect(goback, SIGNAL(clicked()), SLOT(onCancel()));

    myid = p.id;

    show();

    if (width() < 300) {
	resize(300, height());
    }
}

void BaseChallengeWindow::closeEvent(QCloseEvent *)
{
    onCancel();
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

}



ChallengedWindow::ChallengedWindow(const Player &p, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1 challenged you!"), tr("&Accept"), tr("&Refuse"), parent)
{

}
