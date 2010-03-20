#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemoninfo.h"

BaseChallengeWindow::BaseChallengeWindow(const PlayerInfo &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
        : QImageBackground("db/Challenge Window/ChallengeBG.png"), emitOnClose(true)
{
    setParent(parent);

    setWindowTitle(windowTitle.arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose, true);

    QColor grey = "#414141";

    QLabel *name = new QLabel(toColor(p.team.name, grey),this);
    name->setGeometry(54,0,290,52);
    name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    name->setFont(QFont("Trebuchet MS", 20, QFont::DemiBold));

    QLabel *trainerPic = new QLabel(this);
    trainerPic->move(13,85);
    QPixmap px (QString("db/Trainer Sprites/%1.png").arg(p.avatar));
    if (px.isNull())
        px = QString("db/Trainer Sprites/%1.png").arg(168);
    trainerPic->setPixmap(px);

    bool hidden = p.pokes[0]==0;

    if (hidden) {
        QLabel *hiddenTeam = new QLabel(this);
        hiddenTeam->move(163,82);
        hiddenTeam->setPixmap(QPixmap("db/Challenge Window/HiddenInnerBall.png"));
    } else {
        for (int i = 0; i < 6; i++) {
            QLabel *icon = new QLabel(this);
            icon->move(168+i*51,84);
            icon->setPixmap(PokemonInfo::Icon(p.pokes[i]));
        }
    }

    QFont treb("Trebuchet MS", 10);

    QLabel *pinfo = new QLabel(toColor(p.team.info, grey), this);
    pinfo->setGeometry(18,200,280,50);
    pinfo->setWordWrap(true);
    pinfo->setFont(treb);
    pinfo->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QLabel *ladder = new QLabel(toColor(p.rating == -1 ? "unknown" : QString::number(p.rating), grey),this);
    ladder->setFont(treb);
    ladder->setGeometry(100,148,83,18);
    ladder->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    QLabel *tier = new QLabel(toBoldColor(p.tier, Qt::white),this);
    tier->setFont(QFont("Trebuchet MS", 10, QFont::Bold));
    tier->setGeometry(210,148,80,18);
    ladder->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    QWidget *container = new QWidget(this);
    container->setGeometry(QRect(322,157,136,136));
    QVBoxLayout *clausesL= new QVBoxLayout(container);
    clausesL->setSpacing(0);

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i] = new QCheckBox(ChallengeInfo::clause(i));
        clauses[i]->setFont(treb);
        clauses[i]->setStyleSheet("color: #414141");
        clauses[i]->setToolTip(ChallengeInfo::description(i));
        clausesL->addWidget(clauses[i]);
    }

    QImageButton *goback;
    goback = new QImageButton("db/Challenge Window/Buttons/" + buttonNo + "ButtonNormal.png", "db/Challenge Window/Buttons/" + buttonNo + "ButtonGlow.png");
    goback->setParent(this);
    goback->move(182,330);

    challenge_b = new QImageButton("db/Challenge Window/Buttons/" + buttonOk + "ButtonNormal.png", "db/Challenge Window/Buttons/" + buttonOk + "ButtonGlow.png");
    challenge_b->setParent(this);
    challenge_b->move(25,330);

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
        : BaseChallengeWindow(p, tr("%1's Info"), "Chall", "GoBack", parent)
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

void ChallengeWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        event->accept();
        close();
    }
    else
    {
        BaseChallengeWindow::keyPressEvent(event);
    }
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
