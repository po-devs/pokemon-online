#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QPixmap>
#include <QIcon>

class QImageButton;
class QImageButtonP;
class QImageButtonLR;
class BattleDefaultTheme;
class ThemeAccessor;

#define DEFAULT_PO_THEME "Classic"

class Theme {
public:
    enum GenderMode {
        TeamBuilderM,
        BattleM,
        PokedexM,
        IngameM
    };

    static void init(const QString &dir="Themes/" DEFAULT_PO_THEME "/");
    static void Reload(const QString &dir="Themes/" DEFAULT_PO_THEME);

    static QStringList SearchPath();
    static QString FindTheme(const QString& theme);

    static QColor Color(const QString &code);
    static QColor TypeColor(int typenum);
    static QColor CategoryColor(int typenum);
    static QColor StatusColor(int status);
    static QColor ChatColor(int num);
    static QPixmap StatusIcon(int status);
    static QPixmap BattleStatusIcon(int status);
    static QPixmap TypePicture(int type);
    static QPixmap GenderPicture(int gender, GenderMode mode = TeamBuilderM);
    static QString path(const QString &filename, bool defaultP = false);

    static QImageButton *Button(const QString &code);
    static QImageButtonP *PressedButton(const QString &code);
    static QImageButtonLR *LRButton(const QString &code);
    static void ChangePics(QImageButton *b, const QString &code);
    static QPixmap Pic(const QString &way);
    static QPixmap Sprite(const QString &key);
    static QIcon Icon(const QString &key);
    static QPixmap BlueBall();
    static QPixmap WhiteBall();
    static QPixmap GreyBall();
    static QPixmap BlackBall();
    static QPixmap OrangeBall();
    static QPixmap FrameBall();
    static QPixmap TrainerSprite(int num);
    static BattleDefaultTheme* getBattleTheme();
    static ThemeAccessor* getAccessor();
private:
    static QString m_Directory;
    static QList<QColor> m_TColors;
    static QList<QColor> m_CColors;
    static QList<QColor> m_ChatColors;
    static QList<QPixmap> m_TPics;
    static QHash<int, QPixmap> m_statusIcons;
    static QHash<int, QPixmap> m_battleIcons;
    static QHash<QString, QColor> m_Colors;
    static void loadColors();
    static void loadPixmaps();

    static QVariant value(const QString &key, bool *def);
    static BattleDefaultTheme *m_battleTheme;
    static ThemeAccessor *m_accessor;
};

#endif // THEME_H
