#ifndef RELAYMANAGER_H
#define RELAYMANAGER_H

#include <QObject>
#include <QSet>
#include <QPointer>

class Analyzer;

/**
  Manages relays of players. Since they shouldn't be deleted at the same time as the players
  (if there is remnant data to send, like kick/disconnect message, let's at least send it),
  this is a sort of a garbage collector.
 */
class RelayManager : public QObject
{
    Q_OBJECT
public:
    RelayManager();
    ~RelayManager();

    static void init();
    static void destroy();
    void addTrash(Analyzer *relay, QObject *thisObject=NULL);

    static RelayManager *obj();
    Analyzer *dummyRelay();
    const Analyzer *dummyRelay() const;
public slots:
    void cleanTrash();
private:
    static RelayManager *inst;

    QSet< Analyzer* > trash;
    QSet< Analyzer* > recentTrash;

    Analyzer *dummy;
};

#endif // RELAYMANAGER_H
