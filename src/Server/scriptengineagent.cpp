#include "scriptengineagent.h"

ScriptEngineAgent::ScriptEngineAgent(QScriptEngine *e) : QScriptEngineAgent(e)
{
}

void ScriptEngineAgent::exceptionThrow ( qint64, const QScriptValue & err, bool )
{
    if (!const_cast<QScriptValue &>(err).property("backtracetext").isValid()) {
        const_cast<QScriptValue &>(err).setProperty("backtracetext",  err.engine()->currentContext()->backtrace().join("\n"));
    }
}
