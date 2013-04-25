#ifndef ABSTRACTCOMMAND_H
#define ABSTRACTCOMMAND_H

#include <tuple>
#include <functional>

#include "test.h"

/**
  * File for the Command structure.
  *
  * With the commandflow protocol, commands
  * are propagated from object to object, each
  * doing what it wants with the commands in an
  * easy way. They are propagated using fully unstacked
  * argument list, so the need for the structure Command
  * doesn't arise.
  *
  * Command is used when you want to store a command for
  * later reuse. Basically it stores the list of arguments
  * in a tuple and applies it back when needed.
  *
  * The tricky part is with data in pointers. Everything that
  * relies on dynamically allowed data - i.e. every non-POD
  * structure - is in a shared_ptr<T>* in the base level of the
  * commandflow protocol (it can be converted to a reference
  * in upper levels for better ease of use). When storing stuff
  * in a command, you need to instantiate a copy of the shared
  * pointer to make sure the data isn't dangling.
  *
  * So basically the templates below about reference are used
  * to convert data from the protocol to shared_ptr instantiation
  * and convert them back to shared_ptr*.
  */

#include <memory>
/*
namespace std {
    template<class T> class shared_ptr;
}
*/

template <class T>
struct remove_ptr {
    typedef T type;
};

template <class T>
const T& remove_ptr_f (const T &item) {return item;}

template <class T>
struct remove_ptr<std::shared_ptr<T>*> {
    typedef std::shared_ptr<T> type;
};

template <class T>
std::shared_ptr<T>& remove_ptr_f (std::shared_ptr<T> * const & item) {return *item;}

template <class T>
struct remove_ptr_ref {
    typedef T type;
};

template <class T>
struct remove_ptr_ref<T*&> {
    typedef T* type;
};

template <class T>
struct add_ptr {
    typedef T type;
};

template <class T>
T& add_ptr_f (T &item) {return item;}

template <class T>
struct add_ptr<std::shared_ptr<T> > {
    typedef std::shared_ptr<T>* type;
};

template <class T>
std::shared_ptr<T>* add_ptr_f (std::shared_ptr<T> &item) {return &item;}

struct AbstractCommand {
    virtual void apply() = 0;
    virtual int val() const = 0;
    virtual ~AbstractCommand(){}
};

template<class T, class enumC, enumC Val, typename...Params>
struct Command : public AbstractCommand
{
    typedef std::tuple<typename remove_ptr<typename std::remove_reference<Params>::type>::type...> tupleType;
    typedef T boundType;
    typedef enumC enumClass;

    Command(boundType* cl, Params&&... params) : m_tuple(remove_ptr_f(params)...), m_assoc(cl){
    }

    ~Command() {
    }

    void apply() {
        /* Apply boundType::template replayCommand on tuple */
        apply_inn();
    }

    /* De-tuples the parameters to apply them to the function
        boundType::template replayCommand */
    template <typename... MethodParams>
    typename enable_if_c<sizeof...(Params) != sizeof...(MethodParams)>::type
    apply_inn(MethodParams... params) {
        apply_inn(params..., add_ptr_f(std::get<sizeof...(MethodParams)>(m_tuple)));
    }

    void apply_inn(typename remove_ptr_ref<Params>::type... params) {
        m_assoc->template replayCommand<Val>(params...);
    }

    int val() const {
        return Val;
    }

    tupleType m_tuple;
    boundType *m_assoc;
};

#endif // ABSTRACTCOMMAND_H
