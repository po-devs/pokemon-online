#ifndef SHAREDDATAPTR_H
#define SHAREDDATAPTR_H

#include <QSharedDataPointer>
#include <memory>

template <class T>
class data_ptr : protected std::shared_ptr<T>
{
public:
    inline void preprocess() const {
        printf("preprocess called\n");
        if (!std::shared_ptr<T>::get()) {
            data_ptr<T> *base = const_cast<data_ptr<T>*>(this);
            base->reset(new T());
        }
    }

    inline void preprocess2() {
        printf("preprocess2 called\n");
        if (!std::shared_ptr<T>::get()) {
            data_ptr<T> *base = const_cast<data_ptr<T>*>(this);
            base->reset(new T());
        } else if (!std::shared_ptr<T>::unique()) {
            data_ptr<T> *base = const_cast<data_ptr<T>*>(this);
            base->reset(new T(*std::shared_ptr<T>::get()));
        }
    }

    T & operator*() {
        preprocess2();
        return std::shared_ptr<T>::operator*();
    }

    T * operator->() {
        preprocess2();
        return std::shared_ptr<T>::operator->();
    }

    const T & operator*() const {
        preprocess();
        return std::shared_ptr<T>::operator*();
    }
    const T * operator->() const {
        preprocess();
        return std::shared_ptr<T>::operator->();
    }
};

#endif // SHAREDDATAPTR_H
