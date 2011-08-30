#ifndef COMMANDEXTRACTER_H
#define COMMANDEXTRACTER_H

template <class T, class Underling>
class CommandExtracter
{
public:
    typedef T enumClass;
    typedef Underling workerClass;

    void entryPoint(enumClass, ...){}
};

#endif // COMMANDEXTRACTER_H
