#ifndef PO_ENABLEIF_H
#define PO_ENABLEIF_H

template <class X, class U = void>
struct test {
    typedef U type;
};

template <bool B, class T = void>
struct enable_if_c {
  typedef T type;
};

template <class T>
struct enable_if_c<false, T> {};

template <class Cond, class T = void>
struct enable_if : public enable_if_c<Cond::value, T> {};


#endif // PO_ENABLEIF
