#ifndef COMMANDFLOW_H
#define COMMANDFLOW_H

template <class T, class Underling>
class CommandFlow
{
public:
    typedef T enumClass;
    typedef Underling workerClass;

    template <enumClass val, typename ...Params>
    void receiveCommand(Params... params) {
        if (wc()->template shouldInvoke<val, Params...>(params...)) {
            wc()->template invoke<val, Params...>(params...);
        }

        if (wc()->template shouldStore<val, Params...>(params...)) {
            wc()->template store(wc()->template createCommand<val, Params...>(params...));
        } else {
            wc()->template output<val, Params...>(params...);
        }
    }

    inline workerClass* wc() {
        return static_cast<workerClass*>(this);
    }
};

#endif // COMMANDFLOW_H
