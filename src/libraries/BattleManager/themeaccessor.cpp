#include "themeaccessor.h"
#include "TeambuilderLibrary/theme.h"

QPixmap ThemeAccessor::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    (void) requestedSize;

    QPixmap ret;

    if (id.startsWith("avatar/")) {
        return m_theme->trainerSprite(id.section("/", 1).toInt());
    }

    *size = ret.size();

    return ret;
}
