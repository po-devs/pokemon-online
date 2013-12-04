#ifndef REPLAYVIEWER_H
#define REPLAYVIEWER_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"

class SpectatorWindow;
class QPushButton;
class QHBoxLayout;

class ReplayBar : public QObject
{
    Q_OBJECT
public:
    void init();
    QHBoxLayout *getLayout();
public slots:
    void changePause();
signals:
    void paused();
    void play();
    void speedChange(bool fast);
    void seekNext();
public:
    QPushButton *pause, *fastforward, *next;
};

class ReplayViewer : public QObject
{
    Q_OBJECT
public:
    ReplayViewer(const QString &file);
    ~ReplayViewer();

    void setFile();
public slots:
    void read(bool forced=false);

    void play();
    void pause();
    void changeSpeed(bool fast);
    void seekNext();
private:
    SpectatorWindow *window;
    QFile *in;
    FullBattleConfiguration conf;

    QElapsedTimer t;
    bool finished;
    bool paused;
    bool speeding;
    int timerDiff; //Used in case we skip forward, since we can't edit the time in the timer

    quint32 nextRead;
    QByteArray lastData;
    ReplayBar bar;
    int version;
};

#endif // REPLAYVIEWER_H
