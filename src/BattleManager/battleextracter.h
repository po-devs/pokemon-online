#ifndef BATTLEEXTRACTER_H
#define BATTLEEXTRACTER_H

#include <cstdarg>
#include <stdint.h>
#include <unordered_map>
#include "battleenum.h"

template <class Current>
class BattleExtracter
{
public:
    typedef Current workerClass;
    typedef battle::BattleEnum enumClass;

    typedef void (BattleExtracter<Current>::*extrac_func)(va_list);

    BattleExtracter();
    void entryPoint(enumClass, va_list);

    template <enumClass val, typename ...Params>
    void forwardCommand(Params&&...params) {
        wc()->template receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    void forwardUnknownCommand(enumClass val, va_list args) {
        wc()->unknownEntryPoint(val, args);
    }

protected:
    std::unordered_map<enumClass, extrac_func> callbacks;

    /* C++0x doesn't introduce function specialisation, so here goes with
      structures */
    template <enumClass val>
    struct extracter {
        extracter (BattleExtracter<Current> *pointer) : pointer(pointer) {}
        void operator ()(va_list);
        BattleExtracter<Current> *pointer;
    };

    void extractKo(va_list);

    inline workerClass* wc() {
        return (workerClass*)this;
    }
};

template <class C>
void BattleExtracter<C>::extractKo(va_list args)
{
    uint8_t spot = va_arg(args, uint8_t);

    forwardCommand<battle::Ko>(spot);
}

template<class C>
BattleExtracter<C>::BattleExtracter()
{
    callbacks.insert(battle::Ko, &BattleExtracter<workerClass>::extractKo);
}

template <class C>
void BattleExtracter<C>::entryPoint(enumClass val, va_list args)
{
    if (callbacks.find(val) == callbacks.end()) {
        forwardUnknownCommand(val, args);
        return;
    }
    (this->*callbacks[val])(args);
}


#endif // BATTLEEXTRACTER_H
