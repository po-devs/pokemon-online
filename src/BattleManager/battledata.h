#ifndef BATTLEDATA_H
#define BATTLEDATA_H

#include "battlecommandmanager.h"

class BattleData : public BattleCommandManager<BattleData>
{
public:
    void onKo(int spot);

private:

};

#endif // BATTLEDATA_H
