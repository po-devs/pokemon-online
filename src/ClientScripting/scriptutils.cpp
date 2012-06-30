#include "scriptutils.h"
#include "../Utilities/functions.h"

QString ScriptUtils::loadScripts()
{
    ensureDir("script");

    QFile f("script/scripts.js");
    f.open(QIODevice::ReadOnly);

    return QString::fromUtf8(f.readAll());
}
