#ifndef BATTLEPLUGINSTRUCT_H
#define BATTLEPLUGINSTRUCT_H

class BattlePlugin;
class BattleInterface;

struct BattlePStorage
{
    typedef void (BattlePlugin::*Hook) ();

    enum Functions {
        battleStarting = 0,
        lastEnum
    };

    BattlePStorage(BattlePlugin *p);
    ~BattlePStorage();

    void call(int f, BattleInterface *b);

    Hook calls[lastEnum];
    BattlePlugin *plugin;
};

#endif // BATTLEPLUGINSTRUCT_H
