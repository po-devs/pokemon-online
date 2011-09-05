#ifndef BATTLECLIENTLOG_H
#define BATTLECLIENTLOG_H

#include "battlecommandmanager.h"

class BattleData;

class BattleClientLog : public QObject, public BattleCommandManager<BattleClientLog>
{
    Q_OBJECT
public:
    BattleClientLog(BattleData *data);

    void onKo(int spot);
    void onSendOut(int spot, int player, std::shared_ptr<ShallowBattlePoke> pokemon, bool silent);

    QString nick(int spot);
    QString rnick(int spot);

    /* Logs, but doesnt request a print */
    void pushHtml(const QString&);
    /* Requests a print, unless silent, in which case it calls pushHtml and add comments */
    void printHtml(const QString&, bool silent = false);
    void printLine(const QString&, bool silent = false);
signals:
    void lineToBePrinted(const QString &);
protected:
    BattleData *mData;
    BattleData *data();

    QList<QString> log;
    bool blankMessage;
};

#endif // BATTLECLIENTLOG_H
