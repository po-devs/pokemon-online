#include "remove_direction_override.h"

QString removeDirectionOverride(const QString& s)
{
    QChar const rtloverride(0x202e);
    if (s.contains(rtloverride)) {
        QString ret = s;
        return ret.replace(rtloverride, "");
    }
    return s;
}

