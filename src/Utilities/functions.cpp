#include "functions.h"

QString escapeHtml(QString & toConvert)
{
    toConvert.replace("&", "&amp;");
    toConvert.replace("<", "&lt;");
    toConvert.replace(">", "&gt;");

    return toConvert;
}
