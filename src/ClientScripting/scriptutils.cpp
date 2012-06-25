#include "scriptutils.h"
#include "../Utilities/functions.h"

QString ScriptUtils::loadScripts()
{
    QDir d(appDataPath("Scripts/", true));
    QFile f(d.absoluteFilePath("scripts.js"));
    f.open(QIODevice::ReadOnly);

    return QString::fromUtf8(f.readAll());
}
