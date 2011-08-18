#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QtCore>
#include <QColor>
#include <ctime>

class QWidget;

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
    inline const type &name() const { return m_prop_##name;} \
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

inline QString toColor(const QString &mess, const QColor &col)
{
    if (!col.isValid()) {
        return mess;
    }
    return QString("<span style='color:%1'>%2</span>").arg(col.name(), mess);
}

inline QString toBoldColor(const QString &mess, const QColor &col)
{
    return QString("<b><span style='color:%1'>%2</span></b>").arg(col.name(), mess);
}

QString cleanStringForFiles(const QString &title);

inline long int true_rand() {
    return (rand() + unsigned(clock()))%RAND_MAX;
}

inline int intlog2(unsigned x) {
    int i;
    for (i = 0; x > 1; i++) {
        x/= 2;
    }
    return i;
}

inline QByteArray getFileContent(const QString &path) {
    QFile f(path);
    f.open(QIODevice::ReadOnly);

    return f.readAll();
}

void cropImage(QImage &img);

QByteArray md5_hash(const QByteArray &toHash);

void createIntMapper(QObject *src, const char *signal, QObject *dest, const char *slot, int id);

void writeSettings(QWidget *w);
void loadSettings(QWidget *w, const QSize &defaultSize = QSize());

QString slug(const QString &s);

template<class T>
void setDefaultValue(const QString &key, T value)
{
    QSettings s;
    if (s.value(key).isNull())
        s.setValue(key, value);
}

template<class T>
void setDefaultValue(QSettings &s, const QString &key, T value)
{
    if (s.value(key).isNull())
        s.setValue(key, value);
}

#endif // FUNCTIONS_H
