#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

template <class T> class CommandExtracter;

template <class T>
class AbstractCommandManager
{
public:
    typedef T enumClass;

    virtual void entryPoint(enumClass commandId, ...) = 0;
};

template<class T=int, class Current=AbstractCommandManager<T>, class Extracter=CommandExtracter<T, Current> >
class CommandManager : public AbstractCommandManager<T>, public Extracter
{
public:
    typedef T enumClass;
    typedef Current type;
    typedef Extracter extracterType;
private:
    enum {
        /* If triggered, means Current is incorrect type */
        ErrorCurrentShouldBeCastableToBase = static_cast<AbstractCommandManager<enumClass> *>(Current* (NULL))
    };
};


#endif // COMMANDMANAGER_H
