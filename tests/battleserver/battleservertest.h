#ifndef BATTLESERVERTEST_H
#define BATTLESERVERTEST_H

#include <Server/battleanalyzer.h>
#include <test.h>

class BattleServerTest : public Test
{
    Q_OBJECT
public:
    BattleServerTest();

    void start(); /* Override to not automatically accept test once run() is over */
    void run();
public slots:
    /* Override this method to test what you want */
    virtual void onBattleServerConnected();
protected:
    BattleAnalyzer *analyzer;
};

#endif // BATTLESERVERTEST_H
