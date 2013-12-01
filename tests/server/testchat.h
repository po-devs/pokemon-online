#ifndef TESTCHAT_H
#define TESTCHAT_H

#include "testplayer.h"
class TestChat : public TestPlayer
{
    Q_OBJECT
public slots:
    void onPlayerConnected();
    void onChannelMessage(const QString &message, int chanid, bool html);
};

#endif // TESTCHAT_H
