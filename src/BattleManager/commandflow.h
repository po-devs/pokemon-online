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
        if (workerClass::template shouldInvoke<val, Params...>(params...)) {
            workerClass::template invoke<val, Params...>(params...);
        }

        workerClass::template output<val, Params...>(params...);

        if (workerClass::template shouldStore<val, Params...>(params...)) {
            workerClass::template store(workerClass::template createCommand<val, Params...>(params...));
        }
    }
};

#endif // COMMANDFLOW_H
