#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"


BaseChallengeWindow::BaseChallengeWindow(const Player &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
	: QWidget(parent)
{
    setWindowTitle(windowTitle.arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout *mylayout = new QGridLayout(this);
    QPushButton *challenge, *goback;

    mylayout->addWidget( new QEntitled("Player Info: ", new QLabel(p.team.info)),0,0,1,2 );
    mylayout->addWidget( challenge = new QPushButton(buttonOk), 1,0);
    mylayout->addWidget( goback = new QPushButton(buttonNo), 1,1);

    connect(challenge, SIGNAL(clicked()), SLOT(onChallenge()));
    connect(goback, SIGNAL(clicked()), SLOT(onCancel()));
    connect(this, SIGNAL(destroyed()), SLOT(onCancel()));

    id = p.id;

    show();

    if (width() < 300) {
	resize(300, height());
    }
}


void BaseChallengeWindow::onChallenge()
{
    emit challenge(id);
    id = -1;
    close();
}

void BaseChallengeWindow::onCancel()
{
    qDebug("arg");
    if (id != -1) {
	emit cancel(id);
    }
}

ChallengeWindow::ChallengeWindow(const Player &p, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1's Info"), tr("&Challenge"), tr("Go &Back"), parent)
{

}



ChallengedWindow::ChallengedWindow(const Player &p, QWidget *parent)
	: BaseChallengeWindow(p, tr("%1 challenged you!"), tr("&Accept"), tr("&Refuse"), parent)
{

}
