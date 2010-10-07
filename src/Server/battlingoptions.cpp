#include "battlingoptions.h"
#include "../Utilities/otherwidgets.h"

BattlingOptionsWindow::BattlingOptionsWindow()
{
    setAttribute(Qt::WA_DeleteOnClose, true);

    QVBoxLayout *fl = new QVBoxLayout(this);

    QPushButton *apply;

    fl->addWidget(allowCRated = new QCheckBox(tr("Allow rated battles through regular challenges. (not recommended)")));
    fl->addWidget(sameIp = new QCheckBox(tr("Don't allow rated battles between players with the same IP")));
    fl->addLayout(new QSideBySide(new QLabel(tr("Number of battles between rated battles against the same IP")), diffIps = new QSpinBox(this)));
    fl->addWidget(apply = new QPushButton(tr("&Apply")));

    QSettings s("config", QSettings::IniFormat);
    sameIp->setChecked(s.value("battles_with_same_ip_unrated").toBool());
    diffIps->setValue(s.value("rated_battles_memory_number").toInt());
    allowCRated->setChecked(s.value("rated_battle_through_challenge").toBool());

    connect(apply, SIGNAL(clicked()), SLOT(applyChanges()));
}

void BattlingOptionsWindow::applyChanges()
{
    QSettings s("config", QSettings::IniFormat);

    s.setValue("battles_with_same_ip_unrated", sameIp->isChecked());
    s.setValue("rated_battles_memory_number", diffIps->value());
    s.setValue("rated_battle_through_challenge", allowCRated->isChecked());

    emit settingsChanged();

    close();
}
