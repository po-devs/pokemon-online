#include "scriptengineagent.h"

ScriptEngineAgent::ScriptEngineAgent(QScriptEngine *e) : QScriptEngineAgent(e)
{
}

void ScriptEngineAgent::exceptionThrow ( qint64, const QScriptValue & err, bool )
{
    QScriptValue exception = const_cast<QScriptValue &>(err);
    QStringList backtrace = err.engine()->currentContext()->backtrace();
    if (!exception.property("backtrace").isValid()) {
        exception.setProperty("backtrace", qScriptValueFromSequence(err.engine(), backtrace));
    }
    if (!exception.property("backtracetext").isValid()) {
        exception.setProperty("backtracetext", backtrace.join("\n"));
    }
}
