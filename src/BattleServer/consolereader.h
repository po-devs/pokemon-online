#ifndef CONSOLEREADER_H
#define CONSOLEREADER_H

#include <QObject>
#include <QTextStream>

class BattleServer;

class ConsoleReader : public QObject
{
    Q_OBJECT
public:
    ConsoleReader(BattleServer* server);

public slots:
    void read(int);
private:
    BattleServer* m_Server;
    QTextStream m_TextStream;
};

#endif // CONSOLEREADER_H
