#include "battlingoptions.h"
#include "../Utilities/otherwidgets.h"
#include "tiermachine.h"

BattlingOptionsWindow::BattlingOptionsWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *fl = new QVBoxLayout(this);

    QPushButton *apply;

    fl->addWidget(allowCRated = new QCheckBox(tr("Allow rated battles through regular challenges. (not recommended)")));
    fl->addWidget(sameIp = new QCheckBox(tr("Don't allow rated battles between players with the same IP")));
    fl->addLayout(new QSideBySide(new QLabel(tr("Number of battles between rated battles against the same IP")), diffIps = new QSpinBox(this)));
    fl->addSpacing(10);
    fl->addWidget(desc = new QLabel());
    desc->setWordWrap(true);

    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget(percent = new QSpinBox());
    hl->addWidget(hours = new QSpinBox());
    hl->addWidget(max_decay = new QSpinBox());
    hl->addWidget(periods = new QSpinBox());
    hl->addWidget(months = new QSpinBox());
    fl->addLayout(hl);
    fl->addWidget(processOnStartUp = new QCheckBox(tr("Process Ratings on Server Startup")));
    percent->setSuffix(" %");
    hours->setSuffix(" hours");
    max_decay->setSuffix(" %");
    periods->setSuffix(" periods");
    months->setSuffix(" months");
    connect(percent, SIGNAL(valueChanged(int)), SLOT(updateLabel()));
    connect(hours, SIGNAL(valueChanged(int)), SLOT(updateLabel()));
    connect(max_decay, SIGNAL(valueChanged(int)), SLOT(updateLabel()));
    connect(periods, SIGNAL(valueChanged(int)), SLOT(updateLabel()));
    connect(months, SIGNAL(valueChanged(int)), SLOT(updateLabel()));

    percent->setRange(1, 10);
    hours->setRange(1, 10000);
    max_decay->setRange(0, 100);
    periods->setRange(1, 1000);
    months->setRange(1, 240);

    percent->setValue(TierMachine::obj()->percent_per_period);
    hours->setValue(TierMachine::obj()->hours_per_period);
    max_decay->setValue(TierMachine::obj()->max_percent_decay);
    periods->setValue(TierMachine::obj()->max_saved_periods);
    months->setValue(TierMachine::obj()->alt_expiration);

    fl->addWidget(apply = new QPushButton(tr("&Apply")));

    QSettings s("config", QSettings::IniFormat);
    sameIp->setChecked(s.value("battles_with_same_ip_unrated").toBool());
    diffIps->setValue(s.value("rated_battles_memory_number").toInt());
    allowCRated->setChecked(s.value("rated_battle_through_challenge").toBool());
    processOnStartUp->setChecked(s.value("process_ratings_on_startup", true).toBool());

    updateLabel();

    connect(apply, SIGNAL(clicked()), SLOT(applyChanges()));
}

void BattlingOptionsWindow::updateLabel()
{
    desc->setText(QString("Decrease a player's displayed rating by <b>%1%</b> for each period of <b>%2 hours</b> they're not playing. "
                          "Let the maximum total decay on the displayed rating be <b>%3%</b>. Decay can be erased by battles. "
                          "<br/><br/>"
                          "As a bonus for the players, if they fight enough they can save up to <b>%4 periods</b> in which their displayed rating won't be decayed by inactivity. "
                          "Finally, if an alt doesn't play for <b>%5 months</b> they're erased from the ladder. ")
                  .arg(percent->value()).arg(hours->value()).arg(max_decay->value()).arg(periods->value()).arg(months->value()));
}

void BattlingOptionsWindow::applyChanges()
{
    QSettings s("config", QSettings::IniFormat);

    s.setValue("battles_with_same_ip_unrated", sameIp->isChecked());
    s.setValue("rated_battles_memory_number", diffIps->value());
    s.setValue("rated_battle_through_challenge", allowCRated->isChecked());

    s.setValue("ladder_months_expiration", months->value());
    s.setValue("ladder_period_duration", hours->value());
    s.setValue("ladder_percent_per_period", percent->value());
    s.setValue("ladder_bonus_time", periods->value());
    s.setValue("ladder_max_decay", max_decay->value());
    s.setValue("process_ratings_on_startup", processOnStartUp->isChecked());

    emit settingsChanged();

    close();
}
