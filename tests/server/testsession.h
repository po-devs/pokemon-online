#ifndef TESTSESSION_H
#define TESTSESSION_H

#include "testplayer.h"
class TestSESSION : public TestPlayer
{
    Q_OBJECT
public slots:
    void onPlayerConnected();
    void onChannelMessage(const QString &message, int chan, bool html);
};

#endif // TESTSESSION_H
