#ifndef SHAREDDATAPTR_H
#define SHAREDDATAPTR_H

#include <memory>

template <class T>
class data_ptr : public std::shared_ptr<T>
{
public:
    const T & operator*() const {
        return std::shared_ptr<T>::operator*();
    }
    const T * operator->() const {
        return std::shared_ptr<T>::operator->();
    }

    T & operator*() {
        return std::shared_ptr<T>::operator*();
    }
    T * operator->() {
        return std::shared_ptr<T>::operator->();
    }
};

#endif // SHAREDDATAPTR_H
