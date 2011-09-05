#ifndef BATTLECOMMANDINVOKER_H
#define BATTLECOMMANDINVOKER_H

#include <cstdint>
#include <functional>
#include <memory>
#include "test.h"
#include "battleenum.h"

class ShallowBattlePoke;

template <class Underling>
class BattleCommandInvoker
{
public:
    typedef BattleEnum enumClass;
    typedef Underling workerClass;

    template <BattleEnum val>
    struct Param
    {

    };

    template <enumClass val, typename ...Params>
    void invoke(Params&&... params) {
        /* Since no function partial specialisation, using overload */
        invoke2(Param<val>(), std::forward<Params>(params)...);
    }

protected:

    /* the template, as well as the sizeof, are artifacts to test if a function is there in the template class */
//    template<class Y=int>
//    typename test<sizeof(Y)+sizeof(decltype(&workerClass::onKo))>::type invoke2(Param<BattleEnum::Ko>, uint8_t spot) {
//        wc()->onKo(spot);
//    }

//    template<class Y=int>
//    typename test<sizeof(Y)+sizeof(decltype(&workerClass::onSendOut))>::type
//    invoke2(Param<BattleEnum::SendOut>, uint8_t spot, uint8_t prevIndex, std::shared_ptr<ShallowBattlePoke> *ptr, bool silent) {
//        wc()->onSendOut(spot, prevIndex, *ptr, silent);
//    }

    template<enumClass val,typename...Params>
    void invoke2(Param<val>, Params&&...params) {
        wc()->template mInvoke<val, Params...>(std::forward<Params>(params)...);
    }

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }
};

#endif // BATTLECOMMANDINVOKER_H
