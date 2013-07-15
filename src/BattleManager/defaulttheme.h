#ifndef DEFAULTTHEME_H
#define DEFAULTTHEME_H

#include <QColor>
#include <QPixmap>
#include <QObject>

class BattleDefaultTheme : public QObject {
    Q_OBJECT
public:
    BattleDefaultTheme(QObject *parent=0) : QObject(parent){}

    Q_INVOKABLE virtual QColor typeColor(int){return QColor();}
    virtual QColor categoryColor(int){return QColor();}
    virtual QColor statusColor(int){return QColor();}

    virtual QPixmap trainerSprite(int){return QPixmap();}
    virtual QPixmap sprite(const QString&){return QPixmap();}
    virtual QPixmap statusIcon(int){return QPixmap();}
    virtual QPixmap pic(const QString&){return QPixmap();}
    virtual QPixmap battleStatusIcon(int){return QPixmap();}
    virtual QPixmap battleGenderPicture(int){return QPixmap();}
    Q_INVOKABLE virtual QString trainerSpritePath(int) {return QString();}
};

#endif // DEFAULTTHEME_H
