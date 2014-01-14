#ifndef TESTCOLOR_H
#define TESTCOLOR_H

#include "testplayer.h"
class TestColor : public TestPlayer
{
    Q_OBJECT
public slots:
    void onPlayerConnected();
    void onChannelMessage(const QString &message, int chanid, bool html);
};

#endif // TESTCOLOR_H
