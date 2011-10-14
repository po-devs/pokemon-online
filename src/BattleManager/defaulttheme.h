#ifndef DEFAULTTHEME_H
#define DEFAULTTHEME_H

#include <QColor>

class BattleDefaultTheme {
public:
    virtual QColor TypeColor(int){return QColor();}
    virtual QColor CategoryColor(int){return QColor();}
    virtual QColor StatusColor(int){return QColor();}
};

#endif // DEFAULTTHEME_H
