#ifndef THEME_H
#define THEME_H

#include <QtCore>

class Theme {
public:
    static QColor TypeColor(int typenum);
    static QColor CategoryColor(int typenum);
    static QColor StatusColor(int status);
    static QColor ChatColor(int num);

private:
    static QList<QColor> m_TColors;
    static QList<QColor> m_CColors;
    static QList<QColor> m_ChatColors;
};

#endif // THEME_H
