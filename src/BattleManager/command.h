#ifndef ABSTRACTCOMMAND_H
#define ABSTRACTCOMMAND_H

#include <tuple>
#include <functional>

struct AbstractCommand {
    virtual void apply() = 0;
};

template<class T, typename...Params>
struct Command {
    typedef std::tuple<Params...> tupleType;
    typedef T boundType;

    Command(boundType*cl, Params&&... params) : m_tuple(params...), m_assoc(cl){
    }

    void apply() {
        apply<(void(boundType::*)(Params...))T::receiveCommand>();
    }
    template<void(boundType::*func)(Params...)>
    void apply() {
        /* Easy way: use a function. TODO: do it directly (more efficient) */
        std::function<void(boundType*, Params...)> f = func;
        (void)f;
        //::apply(func, tuple) /* TODO: execute f with the params, using the tuple */
    }

    tupleType m_tuple;
    boundType *m_assoc;
};

#endif // ABSTRACTCOMMAND_H
