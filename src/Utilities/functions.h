#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <QMap>

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

/* merge maps -- the second one goes into the first one */
template <class T, class U>
void merge(QMap<T,U> &map1, const QMap<T,U> &map2)
{
    typename QMap<T, U>::const_iterator it;

    for (it = map2.begin(); it != map2.end(); ++it) {
	map1.insert(it.key(), it.value());
    }
}
#endif // FUNCTIONS_H
