#ifndef REPLAYVIEWER_H
#define REPLAYVIEWER_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"

class SpectatorWindow;

class ReplayViewer : public QObject
{
    Q_OBJECT
public:
    ReplayViewer(const QString &file);
    ~ReplayViewer();

    void setFile();
public slots:
    void read();
private:
    SpectatorWindow *window;
    QFile *in;
    FullBattleConfiguration conf;

    QElapsedTimer t;

    quint32 nextRead;
    QByteArray lastData;
};

#endif // REPLAYVIEWER_H
