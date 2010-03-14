#ifndef THREAD_SAFE_CONTAINERS_H
#define THREAD_SAFE_CONTAINERS_H

#include <QtCore>

#ifdef MULTI_THREADED_ACCESS
# define MAKE_THREAD_SAFE mutable QMutex m_global_mutex
# define CONCURRENT_FUNCTION QMutexLocker m_global_locker(&m_global_mutex)
#else
# define MAKE_THREAD_SAFE
# define CONCURRENT_FUNCTION
#endif

template <class T>
class QTSList : private QList<T>
{
    MAKE_THREAD_SAFE;
public:
    typedef typename QList<T>::value_type value_type;
    typedef typename QList<T>::iterator iterator;
    typedef typename QList<T>::const_iterator const_iterator;

    QTSList () {

    }

    QTSList (const QTSList &other) {
        * ((QList<T>*)(this)) = *((QList<T> *)(&other));
        QList<T>::detach();
    }

    QTSList<T> (const QList<T> &other) {
        * ((QList<T>*)(this)) = other;
        QList<T>::detach();
    }

    QTSList<T> & operator = (const QList<T> &other) {
        * ((QList<T>*)(this)) = other;
        QList<T>::detach();
        return *this;
    }

    int size() {
        CONCURRENT_FUNCTION;
        return QList<T>::size();
    }

    T operator [] (int i) const {
        CONCURRENT_FUNCTION;
        return QList<T>::operator [](i);
    }

    T & operator [] (int i) {
        CONCURRENT_FUNCTION;
        return QList<T>::operator [](i);
    }

    iterator begin() {
        CONCURRENT_FUNCTION;
        return QList<T>::begin();
    }

    const_iterator begin()  const {
        CONCURRENT_FUNCTION;
        return QList<T>::begin();
    }

    iterator end() {
        CONCURRENT_FUNCTION;
        return QList<T>::end();
    }

    const_iterator end()  const {
        CONCURRENT_FUNCTION;
        return QList<T>::end();
    }

    void push_back(const T& item) {
        CONCURRENT_FUNCTION;
        QList<T>::push_back(item);
    }

    const_iterator constBegin()  const {
        CONCURRENT_FUNCTION;
        return QList<T>::constBegin();
    }

    const_iterator constEnd()  const {
        CONCURRENT_FUNCTION;
        return QList<T>::constEnd();
    }

    bool empty() const {
        CONCURRENT_FUNCTION;
        return QList<T>::empty();
    }

    QTSList &operator << (const T&l) {
        CONCURRENT_FUNCTION;
        QList<T>::operator <<(l);
        return *this;
    }

    QTSList &operator << (const QTSList<T> &l) {
        CONCURRENT_FUNCTION;
        QList<T>::operator <<(l);
        return *this;
    }
};

template <class T, class U>
class QTSHash : private QHash<T, U>
{
    MAKE_THREAD_SAFE;
public:
    typedef typename QHash<T,U>::iterator iterator;
    typedef typename QHash<T,U>::const_iterator const_iterator;

    QTSHash<T,U> () {

    }

    QTSHash<T,U> (const QTSHash<T,U> &other) {
        * ((QHash<T,U>*)(this)) = *((QHash<T,U> *)(&other));
        QHash<T,U>::detach();
    }

    QTSHash<T,U>& operator = (const QTSHash<T,U> &other) {
        * ((QHash<T,U>*)(this)) = *((QHash<T,U> *)(&other));
        QHash<T,U>::detach();
        return *this;
    }

    int size() {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::size();
    }

    U operator [] (const T &i) const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::operator [](i);
    }

    U & operator [] (const T &i) {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::operator [](i);
    }

    iterator begin() {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::begin();
    }

    const_iterator begin()  const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::begin();
    }

    iterator end() {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::end();
    }

    const_iterator end()  const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::end();
    }

    const_iterator constBegin()  const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::constBegin();
    }

    const_iterator constEnd()  const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::constEnd();
    }

    bool empty() const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::empty();
    }

    bool contains (const T &key) const {
        CONCURRENT_FUNCTION;
        return QHash<T,U>::contains(key);
    }

    QTSHash & operator << (const T&l) {
        CONCURRENT_FUNCTION;
        QHash<T,U>::operator <<(l);
        return *this;
    }

    QTSHash & operator << (const QTSList<T> &l) {
        CONCURRENT_FUNCTION;
        QHash<T,U>::operator <<(l);
        return *this;
    }
};

#endif // THREAD_SAFE_CONTAINERS_H
