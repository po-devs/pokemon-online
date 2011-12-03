#ifndef ABSTRACTCOMMAND_H
#define ABSTRACTCOMMAND_H

#include <tuple>
#include <functional>

struct AbstractCommand {
    virtual void apply() = 0;
    virtual ~AbstractCommand(){}
};

template<class T, class enumC, enumC val, typename...Params>
struct Command : public AbstractCommand
{
    typedef std::tuple<Params...> tupleType;
    typedef T boundType;
    typedef enumC enumClass;

    Command(boundType*cl, Params... params) : m_tuple(params...), m_assoc(cl){
    }

    void apply() {
        /* Apply boundType::template replayCommand on tuple */
    }

    tupleType m_tuple;
    boundType *m_assoc;
};

#endif // ABSTRACTCOMMAND_H
