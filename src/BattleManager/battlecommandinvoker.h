#ifndef BATTLECOMMANDINVOKER_H
#define BATTLECOMMANDINVOKER_H

#include <cstdint>
#include <functional>
#include "test.h"
#include "battleenum.h"

template <class Underling>
class BattleCommandInvoker
{
public:
    typedef battle::BattleEnum enumClass;
    typedef Underling workerClass;

    template <battle::BattleEnum val>
    struct Param
    {

    };

    template <enumClass val, typename ...Params>
    void invoke(Params&&... params) {
        /* Since no function partial specialisation,using overload */
        invoke2(Param<val>(), std::forward<Params>(params)...);
    }

protected:

    template<class Y=int>
    typename test<decltype(sizeof(Y)+sizeof(decltype(&workerClass::onKo)))>::type invoke2(Param<battle::Ko>, uint8_t spot) {
        wc()->onKo(spot);
    }

    template<enumClass val,typename...Params>
    void invoke2(Param<val>, Params&&...params) {
        wc()->template mInvoke<val, Params...>(std::forward<Params>(params)...);
    }

    inline workerClass* wc() {
        return (workerClass*)(this);
    }
};

#endif // BATTLECOMMANDINVOKER_H
