#ifndef THEMEACCESSOR_H
#define THEMEACCESSOR_H

#include <QtDeclarative/QDeclarativeImageProvider>
#include "defaulttheme.h"

class ThemeAccessor : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT
public:
    ThemeAccessor(BattleDefaultTheme *theme) : QDeclarativeImageProvider(QDeclarativeImageProvider::Pixmap) {
        m_theme = theme;
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

private:
    BattleDefaultTheme *m_theme;
};

#endif // THEMEACCESSOR_H
