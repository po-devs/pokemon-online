#ifndef CONSOLEREADER_H
#define CONSOLEREADER_H

#include <QtCore>

class Server;

class ConsoleReader : public QObject
{
    Q_OBJECT
public:
    ConsoleReader(Server* server);

public slots:
    void read(int);
private:
    Server* m_Server;
    QTextStream m_TextStream;
};

#endif
