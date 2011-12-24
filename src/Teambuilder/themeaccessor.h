#ifndef THEMEACCESSOR_H
#define THEMEACCESSOR_H

#include <QString>

struct ThemeAccessor {
    virtual QString path(const QString& file) {return file;}
};

#endif // THEMEACCESSOR_H
