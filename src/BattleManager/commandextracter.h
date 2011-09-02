#ifndef COMMANDEXTRACTER_H
#define COMMANDEXTRACTER_H

#include <cstdarg>

template <class T, class Underling>
class CommandExtracter
{
public:
    typedef T enumClass;
    typedef Underling workerClass;

    void entryPoint(enumClass val, va_list args){workerClass::unknownEntryPoint(val, args);}
};

#endif // COMMANDEXTRACTER_H
