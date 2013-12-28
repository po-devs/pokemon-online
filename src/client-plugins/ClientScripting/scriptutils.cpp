#include "scriptutils.h"
#include "../Utilities/functions.h"

QString ScriptUtils::loadScripts(ScriptType type)
{
    QDir d(appDataPath("Scripts/", true));
    QFile f(d.absoluteFilePath(type==ClientScripts ? "scripts.js" : "battlescripts.js"));
    f.open(QIODevice::ReadOnly);

    return QString::fromUtf8(f.readAll());
}
