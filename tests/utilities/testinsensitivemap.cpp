#include <cassert>

#include "testinsensitivemap.h"
#include "Utilities/coreclasses.h"

TestInsensitiveMap::TestInsensitiveMap()
{
}


void TestInsensitiveMap::run() const
{
    istringmap<int> map;

    map["Hello"] = 2;
    map["world"] = 4;
    map["Ketchup"] = 3;
    map["ketChup"] = 1;

    assert(map["heLLo"] == 2);
    assert(map["world"] == 4);
    assert(map["World"] == 4);
    assert(map["Ketchup"] == 1);
}
