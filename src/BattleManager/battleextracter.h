#ifndef BATTLEEXTRACTER_H
#define BATTLEEXTRACTER_H

#include <QHash>
#include <cstdarg>
#include <stdint.h>
#include <unordered_map>
#include "battleenum.h"

namespace std {
    template<class T> class shared_ptr;
}

class ShallowBattlePoke;

template <class Current>
class BattleExtracter
{
public:
    typedef Current workerClass;
    typedef BattleEnum enumClass;

    typedef void (BattleExtracter<Current>::*extrac_func)(va_list);

    BattleExtracter();
    void entryPoint_v(enumClass, va_list);

    template <enumClass val, typename ...Params>
    void forwardCommand(Params&&...params) {
        wc()->template receiveCommand<val, Params...>(std::forward<Params>(params)...);
    }

    void forwardUnknownCommand(enumClass val, va_list args) {
        wc()->unknownEntryPoint(val, args);
    }

protected:
    QHash<enumClass, extrac_func> callbacks;

    /* C++0x doesn't introduce function specialisation, so here goes with
      structures */
    template <enumClass val>
    struct extracter {
        extracter (BattleExtracter<Current> *pointer) : pointer(pointer) {}
        void operator ()(va_list);
        BattleExtracter<Current> *pointer;
    };

    void extractKo(va_list);
    void extractSendOut(va_list);

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }
};

template <class C>
void BattleExtracter<C>::extractKo(va_list args)
{
    uint8_t spot = va_arg(args, int);

    forwardCommand<BattleEnum::Ko>(spot);
}

template <class C>
void BattleExtracter<C>::extractSendOut(va_list args)
{
    uint8_t spot = va_arg(args, int);
    uint8_t prevIndex = va_arg(args, int);
    std::shared_ptr<ShallowBattlePoke> *poke = va_arg(args, std::shared_ptr<ShallowBattlePoke> *);
    bool silent = va_arg(args, int);

    forwardCommand<BattleEnum::SendOut>(spot, prevIndex, poke, silent);
}

template<class C>
BattleExtracter<C>::BattleExtracter()
{
    callbacks.insert(BattleEnum::Ko, &BattleExtracter<workerClass>::extractKo);
    callbacks.insert(BattleEnum::SendOut, &BattleExtracter<workerClass>::extractSendOut);
}

template <class C>
void BattleExtracter<C>::entryPoint_v(enumClass val, va_list args)
{
    if (callbacks.find(val) == callbacks.end()) {
        forwardUnknownCommand(val, args);
        return;
    }
    (this->*callbacks[val])(args);
}


#endif // BATTLEEXTRACTER_H
