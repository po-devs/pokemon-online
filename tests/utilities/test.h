#ifndef TEST_H
#define TEST_H

class Test
{
public:
    Test();
    virtual ~Test(){}

    virtual void run() = 0;
};

#endif // TEST_H
