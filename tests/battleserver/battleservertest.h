#ifndef BATTLESERVERTEST_H
#define BATTLESERVERTEST_H

#include <Utilities/baseanalyzer.h>
#include "../common/test.h"

class BattleServerTest : public Test
{
    Q_OBJECT
public:
    BattleServerTest();

    void run();
public slots:
    /* Override this method to test what you want */
    virtual void onBattleServerConnected();
private:
    BaseAnalyzer *analyzer;
};

#endif // BATTLESERVERTEST_H
