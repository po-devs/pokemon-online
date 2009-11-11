#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"

ChallengeWindow::ChallengeWindow(const Player &p, QWidget *parent)
	: QWidget(parent)
{
    setWindowTitle(tr("%1's Info").arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose);

    QGridLayout *mylayout = new QGridLayout(this);
    QPushButton *challenge, *goback;

    mylayout->addWidget( new QEntitled("Player Info: ", new QLabel(p.team.info)),0,0,1,2 );
    mylayout->addWidget( challenge = new QPushButton(tr("&Challenge")), 1,0);
    mylayout->addWidget( goback = new QPushButton(tr("Go &Back")), 1,1);

    connect(challenge, SIGNAL(clicked()), SLOT(onChallenge()));
    connect(goback, SIGNAL(clicked()), SLOT(close()));

    id = p.id;

    show();

    if (width() < 300) {
	resize(300, height());
    }
}

void ChallengeWindow::onChallenge()
{
    emit challenge(id);
    close();
}
