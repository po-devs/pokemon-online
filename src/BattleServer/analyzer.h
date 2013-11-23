#ifndef ANALYZER_H
#define ANALYZER_H

#include <QObject>

#include "../Utilities/coreclasses.h"
#include "../Utilities/asiosocket.h"

class GenericNetwork;

class Analyzer : public QObject
{
    Q_OBJECT
public:
    Analyzer(GenericSocket sock, int id);
    
    template <typename ...Params>
    void notify(int command, Params&&... params) {
        QByteArray tosend;
        DataStream out(&tosend, QIODevice::WriteOnly);

        out.pack(uchar(command), std::forward<Params>(params)...);

        emitCommand(tosend);
    }

    /* Convenience functions to avoid writing a new one every time */
    inline void emitCommand(const QByteArray &command) {
        emit sendCommand(command);
    }
signals:
    void sendCommand(const QByteArray&);
public slots:
    
private:
    GenericNetwork *socket;
};

#endif // ANALYZER_H
