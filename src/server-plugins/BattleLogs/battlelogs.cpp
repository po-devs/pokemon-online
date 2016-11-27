#include <QFormLayout>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>

#include "battleserverlog.h"
#include "battlelogs.h"

#include <BattleManager/battleinput.h>
#include <BattleManager/battleclientlog.h>
#include <BattleManager/battledatatypes.h>
#include "../Shared/battlecommands.h"


BattleServerPlugin * createBattleServerPlugin() {
    return new BattleLogs();
}

BattleLogs::BattleLogs()
{
    QSettings s("config_battleLogs", QSettings::IniFormat);
    saveMixedTiers = s.value("save_mixed_tiers", true).toBool();
    saveRawFiles = s.value("save_raw_files", true).toBool();
    saveTextFiles = s.value("save_text_files", false).toBool();

    tiers = s.value("tiers", QStringList()).toStringList().toSet();

    QSettings server("config", QSettings::IniFormat);
    webUrl = server.value("Server/Web", "http://web.pkmn.co").toString();
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

    return new BattleLogsPlugin(b, saveRawFiles, saveTextFiles, webUrl);
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

BattleLogsPlugin::BattleLogsPlugin(BattleInterface *b, bool raw, bool plain, const QString &url) : commands(&toSend, QIODevice::WriteOnly), raw(raw), text(plain), url(url), m(QMutex::Recursive)
{
    //qDebug() << "plugin start";
    input = NULL;

    conf = b->configuration();

    if (text) {
        input = new BattleInput(&conf, conf.protocolVersion);
        data = new battledata_basic(&conf);
        log = new BattleServerLog(data, &theme);
        input->addOutput(data);
        input->addOutput(log);
    }

    started = false;
    logging = true;
    t.start();
}

BattleLogsPlugin::~BattleLogsPlugin()
{
    //qDebug() << "plugin deleted";
    if (input) {
        input->deleteTree();
    }

    delete input, input = NULL;
}

QHash<QString, BattlePlugin::Hook> BattleLogsPlugin::getHooks()
{
    QHash<QString, Hook> ret;

    ret.insert("battleStarting(BattleInterface&)", (Hook)(&BattleLogsPlugin::battleStarting));
    ret.insert("battleEnded(BattleInterface&)", (Hook)(&BattleLogsPlugin::battleEnded));
    ret.insert("emitCommand(BattleInterface&,int,int,QByteArray)", (Hook)(&BattleLogsPlugin::emitCommand));

    return ret;
}

int BattleLogsPlugin::battleStarting(BattleInterface &b)
{
    //qDebug() << "battle started";
    QMutexLocker l(&m);
    if (raw) {
        team1 = b.team(0);
        team2 = b.team(1);
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

int BattleLogsPlugin::battleEnded(BattleInterface &b)
{
    //qDebug() << "battle ended";
    QMutexLocker l(&m);

    (void) b;
    logging = false;

    //if (started) {
        QString date = QDate::currentDate().toString("yyMMdd");
        QString time = QTime::currentTime().toString("hh'h'mm'm'ss's'");
        QString id0 = QString::number(id1);
        QString id1 = QString::number(id2);

        QString hash = QString::number(qHash(QString("%2-%3-%4").arg(time,id0,id1)));

        QDir d("");
        if(!d.exists("logs/battles/" + date)) {
            d.mkpath("logs/battles/" + date);
        }

        if (raw) {
            QFile out;
            out.setFileName(QString("logs/battles/%1/%2.poreplay").arg(date, hash));
            out.open(QIODevice::WriteOnly);
            out.write("battle_logs_v3\n");

            /* Writing configuration */
            DataStream outd(&out);
            conf.teams[0] = &team1;
            conf.teams[1] = &team2;
            outd << conf;

            out.write(toSend);
            out.close();

            /* Privacy concerns will be dealt with if they arise */
            b.sendMessage(BattleInterface::All, "Replay", url + "/replay/" + date + "-" + hash);
        }

        if (text) {
            QFile out;
            out.setFileName(QString("logs/battles/%1/%2-%3-%4.html").arg(date, hash));
            out.open(QIODevice::WriteOnly);
            out.write(log->getLog().join("").toUtf8());
            out.close();
        }
    //}
    return 0;
}

int BattleLogsPlugin::emitCommand(BattleInterface &, int, int players, QByteArray b)
{
    QMutexLocker l(&m);
    if (!logging) {
        return 0;
    }
    /* Those, are not logged */
    if (char(b[0]) == BattleCommands::CancelMove || char(b[0]) == BattleCommands::OfferChoice || char(b[0]) == BattleCommands::RearrangeTeam)
        return 0;

    /* Check if it's not a command given to a player/spectator in particular */
    //if (players != BattleInterface::AllButPlayer && players < 10000) {
    /* Only spectator side */
    if (players == BattleInterface::AllButPlayer || players == BattleInterface::All) {
        if (raw) {
            commands << qint32(t.elapsed()) << b;
        }
        if (text) {
            input->receiveData(b);
        }
    }

    return 0;
}
