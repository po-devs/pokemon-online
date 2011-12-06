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
    typedef std::tuple<typename std::remove_reference<Params>::type...> tupleType;
    typedef T boundType;
    typedef enumC enumClass;

    Command(boundType*cl, Params... params) : m_tuple(params...), m_assoc(cl){
    }

    ~Command() {
    }

    void apply() {
        /* Apply boundType::template replayCommand on tuple */
        apply_inn();
    }

    /* De-tuples the parameters to apply them to the function
        boundType::template replayCommand */
    template <typename... MethodParams>
    void apply_inn(MethodParams... params) {
        apply_inn(params..., std::get<sizeof...(MethodParams)>(m_tuple));
    }

    void apply_inn(Params... params) {
        m_assoc->template replayCommand<val, Params...>(params...);
    }

    tupleType m_tuple;
    boundType *m_assoc;
};

#endif // ABSTRACTCOMMAND_H
