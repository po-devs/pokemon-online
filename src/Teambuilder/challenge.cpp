#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemoninfo.h"

BaseChallengeWindow::BaseChallengeWindow(const PlayerInfo &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
        : QImageBackground("db/Challenge Window/ChallengeBG"), emitOnClose(true)
{
    setParent(parent);

    setWindowTitle(windowTitle.arg(p.team.name));
    setWindowFlags( Qt::FramelessWindowHint );


    /* Transparent background code */
    setMask(myBackground.mask());
    /* End of transparent background */

    setAttribute(Qt::WA_DeleteOnClose, true);

    QImageButton *goback;
    const int x = 6;

    QLabel *name = new QLabel(toBoldColor(p.team.name, "#1ba6eb"),this);
    name->setGeometry(23-x,21-x,250-23,50-18);
    name->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    name->setFont(QFont("db/Font/kirstyin.ttf", 17));

    for (int i = 0; i < 6; i++) {
        QLabel *icon = new QLabel(this);
        icon->move(180+i*52-x,72-x);
        icon->setPixmap(PokemonInfo::Icon(p.pokes[i]));
    }

    QLabel *pinfo = new QLabel(toColor(p.team.info, Qt::white), this);
    pinfo->setGeometry(177-x,128-x,474-177,180-128);
    pinfo->setWordWrap(true);
    pinfo->setFont(QFont("Verdana", 10));
    pinfo->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QFont boldWhite("db/Font/kirstyin.ttf",8);

    QLabel *ladder = new QLabel(toBoldColor(p.rating == -1 ? "unknown" : QString::number(p.rating), Qt::white),this);
    ladder->setFont(boldWhite);
    ladder->setGeometry(84-x,87-x,160-84,102-87);
    ladder->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    QWidget *container = new QWidget(this);
    container->setGeometry(QRect(9-x,129-x,180,126));
    QVBoxLayout *clausesL= new QVBoxLayout(container);
    clausesL->setSpacing(0);

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i] = new QCheckBox(ChallengeInfo::clause(i));
        clauses[i]->setFont(boldWhite);
        clauses[i]->setToolTip(ChallengeInfo::description(i));
        clausesL->addWidget(clauses[i]);
    }

    goback = new QImageButton("db/Challenge Window/" + buttonNo + "Normal", "db/Challenge Window/" + buttonNo + "Glow");
    goback->setParent(this);
    goback->move(153-x,193-x);

    challenge_b = new QImageButton("db/Challenge Window/" + buttonOk + "Normal", "db/Challenge Window/" + buttonOk + "Glow");
    challenge_b->setParent(this);
    challenge_b->move(333-x,193-x);

    connect(goback, SIGNAL(clicked()), SLOT(onCancel()));

    myid = p.id;

    show();
}

void BaseChallengeWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void BaseChallengeWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition);
        event->accept();
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
        : BaseChallengeWindow(p, tr("%1's Info"), "Challenge", "GoBack", parent)
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
        : BaseChallengeWindow(p, tr("%1 challenged you!"), "Accept", "Decline", parent)
{
    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        this->clauses[i]->setChecked(clauses & (1 << i));
        this->clauses[i]->setDisabled(true);
    }
}
