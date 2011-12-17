#ifndef DEFAULTTHEME_H
#define DEFAULTTHEME_H

#include <QColor>
#include <QPixmap>

class BattleDefaultTheme {
public:
    virtual QColor TypeColor(int){return QColor();}
    virtual QColor CategoryColor(int){return QColor();}
    virtual QColor StatusColor(int){return QColor();}

    virtual QPixmap TrainerSprite(int){return QPixmap();}
    virtual QPixmap Sprite(const QString&){return QPixmap();}
    virtual QPixmap StatusIcon(int){return QPixmap();}
    virtual QPixmap Pic(const QString&){return QPixmap();}
    virtual QPixmap BattleStatusIcon(int){return QPixmap();}
    virtual QPixmap BattleGenderPicture(int){return QPixmap();}
};

#endif // DEFAULTTHEME_H
