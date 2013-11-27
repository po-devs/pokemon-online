#include <cassert>
#include <Utilities/functions.h>
#include "testfunctions.h"

void TestFunctions::run()
{
    assert(intlog2(8) == 3);
    assert(intpow2(3) == 8);
    assert(intlog2(intpow2(5)) == 5);
}
