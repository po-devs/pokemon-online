#ifndef SCRIPTUTILS_H
#define SCRIPTUTILS_H

#include <QString>

struct ScriptUtils {
    enum ScriptType {
        ClientScripts,
        BattleScripts
    };

    static QString loadScripts(ScriptType type=ClientScripts);
};

#endif // SCRIPTUTILS_H
