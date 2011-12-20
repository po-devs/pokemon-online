#include "battleserverlog.h"
#include "battlelogs.h"
#include <QtGui>
#include "../BattleManager/battleinput.h"
#include "../BattleManager/battleclientlog.h"
#include "../BattleManager/battledatatypes.h"

ServerPlugin * createPluginClass(ServerInterface*) {
    return new BattleLogs();
}

BattleLogs::BattleLogs()
{
    QSettings s("config_battleLogs", QSettings::IniFormat);
    saveMixedTiers = s.value("save_mixed_tiers", true).toBool();
    saveRawFiles = s.value("save_raw_files", true).toBool();
    saveTextFiles = s.value("save_text_files", false).toBool();

    tiers = s.value("tiers", QStringList()).toStringList().toSet();
}

QString BattleLogs::pluginName() const
{
    return "Battle Logs";
}

BattlePlugin* BattleLogs::getBattlePlugin(BattleInterface* b)
{
    if (b->tier().isEmpty()) {
        if (!saveMixedTiers)
            return NULL;
    } else {
        if (!tiers.contains(b->tier()) && tiers.size() > 0)
            return NULL;
        if (!saveRawFiles && !saveTextFiles)
            return NULL;
    }

    return new BattleLogsPlugin(b, saveRawFiles, saveTextFiles);
}

bool BattleLogs::hasConfigurationWidget () const
{
    return true;
}

QWidget *BattleLogs::getConfigurationWidget()
{
    return new BattleLogsWidget(this);
}

/************************/
/************************/
/************************/

BattleLogsWidget::BattleLogsWidget(BattleLogs *master)
{
    this->master = master;

    QFormLayout *f = new QFormLayout(this);

    f->addRow("Tiers to be logged (input nothing to log all tiers)", tiers = new QTextEdit());
    f->addWidget(mixedTiers = new QCheckBox("Save battles between different tiers"));
    f->addWidget(rawFile = new QCheckBox("Save raw binary logs"));
    f->addWidget(textFile = new QCheckBox("Save html logs"));

    mixedTiers->setChecked(master->saveMixedTiers);
    rawFile->setChecked(master->saveRawFiles);
    textFile->setChecked(master->saveTextFiles);

    tiers->setText(QStringList(master->tiers.toList()).join(", "));

    QPushButton *button = new QPushButton("Done");
    f->addRow(NULL, button);

    connect(button, SIGNAL(clicked()), SLOT(done()));
}

void BattleLogsWidget::done()
{
    QStringList tiers = this->tiers->toPlainText().split(",", QString::SkipEmptyParts);
    for(int i = 0; i < tiers.size(); i++) {
        tiers[i] = tiers[i].trimmed();
    }

    master->tiers = tiers.toSet();
    master->saveMixedTiers = mixedTiers->isChecked();
    master->saveRawFiles = rawFile->isChecked();
    master->saveTextFiles = textFile->isChecked();

    QSettings s("config_battleLogs", QSettings::IniFormat);
    s.setValue("save_mixed_tiers", mixedTiers->isChecked());
    s.setValue("save_raw_files", rawFile->isChecked());
    s.setValue("save_text_files", textFile->isChecked());
    s.setValue("tiers", tiers);

    close();
}

/************************/
/************************/
/************************/

BattleLogsPlugin::BattleLogsPlugin(BattleInterface *b, bool raw, bool plain) : commands(&toSend, QIODevice::WriteOnly), raw(raw), text(plain)
{
    input = NULL;

    if (text) {
        conf = b->configuration();
        conf.receivingMode[0] = conf.receivingMode[1] = BattleConfiguration::Player;
        conf.teams[0] = &b->team(0);
        conf.teams[1] = &b->team(1);

        //log->data()->reloadTeam(0);
        //log->data()->reloadTeam(1);

        input = new BattleInput(&conf);
        data = new battledata_basic(&conf);
        log = new BattleServerLog(data, &theme);
        input->addOutput(data);
        input->addOutput(log);
    }

    started = false;
    commands.setVersion(QDataStream::Qt_4_5);
    t.start();
}

BattleLogsPlugin::~BattleLogsPlugin()
{
    if (!started)
        return;

    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    QString time = QTime::currentTime().toString("hh'h'mm'm'ss's'");
    QString id0 = QString::number(id1);
    QString id1 = QString::number(id2);

    QDir d("");
    if(!d.exists("logs/battles/" + date)) {
        d.mkpath("logs/battles/" + date);
    }

    if (raw) {
        QFile out;
        out.setFileName(QString("logs/battles/%1/%2-%3-%4.raw").arg(date, time, id0, id1));
        out.open(QIODevice::WriteOnly);
        out.write("battle_logs_v0 0\n");
        out.write(teams);
        out.write(toSend);
        out.close();
    }

    if (text) {
        QFile out;
        out.setFileName(QString("logs/battles/%1/%2-%3-%4.html").arg(date, time, id0, id1));
        out.open(QIODevice::WriteOnly);
        out.write(log->getLog().join("").toUtf8());
        out.close();

        if (input) {
            input->deleteTree();
        }

        delete input;
    }
}

QHash<QString, BattlePlugin::Hook> BattleLogsPlugin::getHooks()
{
    QHash<QString, Hook> ret;

    ret.insert("battleStarting(BattleInterface&)", (Hook)(&BattleLogsPlugin::battleStarting));
    ret.insert("emitCommand(BattleInterface&,int,int,QByteArray)", (Hook)(&BattleLogsPlugin::emitCommand));

    return ret;
}

int BattleLogsPlugin::battleStarting(BattleInterface &b)
{
    if (raw) {
        QDataStream dteams(&teams, QIODevice::WriteOnly);

        QByteArray team1;
        QDataStream d1(&team1, QIODevice::WriteOnly);
        d1.setVersion(QDataStream::Qt_4_5);
        d1 << b.team(0);
        dteams << team1;

        QByteArray team2;
        QDataStream d2(&team2, QIODevice::WriteOnly);
        d2.setVersion(QDataStream::Qt_4_5);
        d2 << b.team(1);
        dteams << team2;
    }

    //team may have been reordered with wifi clause?
    if (text) {
        data->reloadTeam(0);
        data->reloadTeam(1);
    }

    id1 = b.id(0);
    id2 = b.id(1);
    started = true;

    return 0;
}

int BattleLogsPlugin::emitCommand(BattleInterface &, int, int players, QByteArray b)
{
    /* Those, are not logged */
    if (char(b[0]) == BattleInterface::CancelMove || char(b[0]) == BattleInterface::OfferChoice || char(b[0]) == BattleInterface::RearrangeTeam)
        return 0;

    if (players != BattleInterface::AllButPlayer) {
        if (raw) {
            commands << qint32(t.elapsed()) << b;
        }
        if (text) {
            input->receiveData(b);
        }
    }

    return 0;
}
