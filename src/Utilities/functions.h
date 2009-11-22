#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>

/* Changes a string so all what is inside is converted to html.

    This is useful when for example you want to display html code, or when you want to avoid a value entered
    by the user to change the formatting of the content of the html display.

    For example 'Hello <b>world</b> & Cie' is converted into 'Hello &lt;b&rt;world&lt;/b&rt; &amp; Cie'
 */
QString escapeHtml(const QString &toConvert);

/* Macro to write less code */
#define PROPERTY(type, name) \
public: \
    inline type& name() { return m_prop_##name;}\
    inline type name() const { return m_prop_##name;} \
private: \
    type m_prop_##name;\
public:


#endif // FUNCTIONS_H
