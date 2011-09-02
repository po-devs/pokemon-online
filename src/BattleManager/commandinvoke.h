#ifndef COMMANDINVOKE_H
#define COMMANDINVOKE_H

template <class T, class Underling>
class CommandInvoker
{
public:
    typedef T enumClass;
    typedef Underling workerClass;

    template <enumClass val, typename ...Params>
    void invoke(Params&&... params) {
        workerClass::template mInvoker<val, Params...>(std::forward<Params>(params)...);
    }
};

#endif // COMMANDINVOKE_H
