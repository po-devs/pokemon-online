#include "test.h"

Test::Test()
{
    connect(this, SIGNAL(failure()), SIGNAL(finished()));
    connect(this, SIGNAL(success()), SIGNAL(finished()));
}
