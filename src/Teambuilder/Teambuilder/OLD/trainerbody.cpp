#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QPlainTextEdit>
#include "trainerbody.h"
#include "theme.h"
#include "avatarbox.h"
#include "teambuilder_old.h"
#include "pokeballed.h"
#include "teamholder.h"

TB_TrainerBody::TB_TrainerBody(TeamHolder *team) : m_team(team)
{
    QHBoxLayout *ml = new QHBoxLayout(this);
    ml->setMargin(0);

    //////////////// First Column  ///////////////////
    QVBoxLayout *col1 = new QVBoxLayout();
    ml->addLayout(col1);

    /* Avatar */
    col1->addWidget(new TitledWidget(tr("Avatar"), m_avatar=new AvatarBox()));

    /* Avatar Selection */
    col1->addWidget(m_avatarSelection = new QSpinBox(), 5, Qt::AlignTop);
    m_avatarSelection->setRange(1,263);
    m_avatarSelection->setAccessibleName(tr("Avatar selector", "TB accessible name"));
    m_avatarSelection->setAccessibleDescription(tr("In this field you can select your avatar by its number.", "TB accessible description"));

    //////////////// Second Column ///////////////////
    QVBoxLayout *col2 = new QVBoxLayout();
    ml->addLayout(col2,100);

    /* Trainer nickname */
    col2->addWidget(new TitledWidget(tr("Trainer &Name"),m_nick = new QLineEdit()));
    m_nick->setMaximumWidth(150);
    m_nick->setValidator(new QNickValidator(m_nick));
    m_nick->setAccessibleName(tr("Trainer name", "TB accessible name"));

    QHBoxLayout *colorTier = new QHBoxLayout();
    colorTier->setMargin(0);
    col2->addLayout(colorTier);
    /* Trainer name color */
    colorTier->addWidget(new TitledWidget(tr("Name Color"), m_colorButton = new QPushButton(tr("Change &Color"))));
    QSettings s;
    if (s.value("trainer_color").value<QColor>().name() != "#000000")
        m_colorButton->setStyleSheet(QString("background: %1;color:white").arg(s.value("trainer_color").value<QColor>().name()));
    m_colorButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    colorTier->addWidget(new TitledWidget(tr("Team Tier"), m_tier = new QLineEdit()));
    m_tier->setText(trainerTeam()->team().defaultTier());
    m_tier->setAccessibleName(tr("Team tier", "TB accessible name"));

    /* Trainer information */
    col2->addWidget(new TitledWidget(tr("Trainer I&nformation"), m_trainerInfo = new QPlainTextEdit()));
    m_trainerInfo->setAccessibleName(tr("Trainer information", "TB accessible name"));

    /* Trainer win, lose message */
    col2->addWidget(new TitledWidget(tr("&Winning Message"), m_winMessage = new QPlainTextEdit()));
    m_winMessage->setAccessibleName(tr("Winning message", "TB accessible name"));
    col2->addWidget(new TitledWidget(tr("L&osing Message"), m_loseMessage = new QPlainTextEdit()));
    m_loseMessage->setAccessibleName(tr("Losing message", "TB accessible name"));

    //////////////// Third  Column ///////////////////
    QVBoxLayout *col3 = new QVBoxLayout();
    ml->addLayout(col3,0);

    QLabel *ash = new QLabel();
    ash->setPixmap(Theme::Sprite("ash"));

    col3->addWidget(ash,0,Qt::AlignBottom);

    m_winMessage->setTabChangesFocus(true);
    m_loseMessage->setTabChangesFocus(true);
    m_trainerInfo->setTabChangesFocus(true);

    connect (m_colorButton, SIGNAL(clicked()), SLOT(changeTrainerColor()));
    connect (m_nick, SIGNAL(textEdited(QString)), SLOT(setTrainerNick(QString)));
    connect (m_tier, SIGNAL(textEdited(QString)), SLOT(changeTier(QString)));
    connect (m_winMessage, SIGNAL(textChanged()), SLOT(changeTrainerWin()));
    connect (m_loseMessage, SIGNAL(textChanged()), SLOT(changeTrainerLose()));
    connect (m_trainerInfo, SIGNAL(textChanged()), SLOT(changeTrainerInfo()));
    connect (m_avatarSelection, SIGNAL(valueChanged(int)), SLOT(changeTrainerAvatar(int)));
}

void TB_TrainerBody::changeTrainerColor()
{
    QSettings s;
    QColor c = QColorDialog::getColor(s.value("trainer_color").value<QColor>().name());

    s.setValue("trainer_color", c);

    if (c.isValid() && c.lightness() <= 140 && c.green() <= 180)
        m_colorButton->setStyleSheet(QString("background: %1; color: white").arg(c.name()));
    else {
        s.setValue("trainer_color", "");
        m_colorButton->setStyleSheet("");
    }
}

void TB_TrainerBody::setTierList(const QStringList &tiers) {
    QCompleter *completer = new QCompleter(tiers, m_tier);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    m_tier->setCompleter(completer);
}

TeamHolder * TB_TrainerBody::trainerTeam()
{
    return m_team;
}

void TB_TrainerBody::updateTrainer()
{
    m_trainerInfo->setPlainText(trainerTeam()->info().info);
    m_nick->setText(trainerTeam()->name());
    m_winMessage->setPlainText(trainerTeam()->info().winning);
    m_loseMessage->setPlainText(trainerTeam()->info().losing);
    m_tier->setText(trainerTeam()->team().defaultTier());
    changeTrainerAvatar(trainerTeam()->info().avatar);
}

void TB_TrainerBody::changeTrainerInfo()
{
    trainerTeam()->info().info = m_trainerInfo->toPlainText();
}

void TB_TrainerBody::setTrainerNick(const QString &newnick)
{
    trainerTeam()->name() = newnick;
}

void TB_TrainerBody::changeTier(const QString &tier)
{
    trainerTeam()->team().defaultTier() = tier;
}

void TB_TrainerBody::changeTrainerWin()
{
    trainerTeam()->info().winning = m_winMessage->toPlainText();
}

void TB_TrainerBody::changeTrainerLose()
{
    trainerTeam()->info().losing = m_loseMessage->toPlainText();
}

void TB_TrainerBody::changeTrainerAvatar(int newavatar)
{
    if (newavatar==0)
        newavatar=1;
    m_avatarSelection->setValue(newavatar);
    trainerTeam()->info().avatar = newavatar;
    m_avatar->changePic(Theme::TrainerSprite(newavatar));
}
