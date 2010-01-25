#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QtCore>

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
void merge(QHash<T,U> &map1, const QHash<T,U> &map2)
{
    typename QHash<T, U>::const_iterator it;

    for (it = map2.begin(); it != map2.end(); ++it) {
	map1.insert(it.key(), it.value());
    }
}

inline void inc(QVariant &v, int change=1)
{
    v = v.toInt() + change;
}

struct PokeFraction
{
    int up, down;

    PokeFraction (int up, int down) : up(up), down(down) {}
};

inline int operator *(int num, const PokeFraction &p)
{
    return num * p.up / p.down;
}

/* just a little convenience */
inline QString tu(QString &in)
{
    if (!in[0].isUpper())
	in[0] = in[0].toUpper();
    return in;
}

inline QString tu(const QString &in) {
    if (in[0].isUpper())
	return in;
    else {
	QString str2 = in;
	str2[0] = in[0].toUpper();
	return str2;
    }
}

QString md5_hash(const QString &toHash);

#endif // FUNCTIONS_H
