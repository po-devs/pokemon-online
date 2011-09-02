#ifndef TEST_H
#define TEST_H

template <class T, class U = void>
struct test {
    typedef U type;
};

#endif // TEST_H
