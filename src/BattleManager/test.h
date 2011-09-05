#ifndef TEST_H
#define TEST_H

template <int X=0, class U = void>
struct test {
    typedef U type;
};

#endif // TEST_H
