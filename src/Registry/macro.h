#ifndef MACRO_H
#define MACRO_H

/* Macro to write less code */
#define PROPERTY(type, name) \
public: \
    inline type& name() { return m_prop_##name;}\
    inline type name() const { return m_prop_##name;} \
private: \
    type m_prop_##name;\
public:

#endif // MACRO_H
