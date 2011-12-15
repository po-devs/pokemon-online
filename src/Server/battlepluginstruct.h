#ifndef BATTLEPLUGINSTRUCT_H
#define BATTLEPLUGINSTRUCT_H

class BattlePlugin;
class BattleInterface;

struct BattlePStorage
{
    typedef int (BattlePlugin::*Hook) ();

    enum Functions {
        battleStarting = 0,
        emitCommand = 1,
        lastEnum
    };

    BattlePStorage(BattlePlugin *p);
    ~BattlePStorage();

    int call(int f, BattleInterface *b);
    template<class T1, class T2, class T3>
    int call(int f, BattleInterface *b, T1 arg1, T2 arg2, T3 arg3);

    Hook calls[lastEnum];
    BattlePlugin *plugin;
};

template<class T1, class T2, class T3>
int BattlePStorage::call(int f, BattleInterface *b, T1 arg1, T2 arg2, T3 arg3)
{
    if (calls[f]) {
        //qDebug() << "Plugin of " << this << " is " << plugin << "(Battle " << b << ")";
        /* Calls the plugin member function, from the correct class, with the appropriate parameters */
        return (*plugin.*(reinterpret_cast<int (BattlePlugin::*)(BattleInterface &, T1, T2, T3)>(calls[f])))(*b, arg1, arg2, arg3);
    }
    return 0;
}

class BattlePluginException;

#endif // BATTLEPLUGINSTRUCT_H
