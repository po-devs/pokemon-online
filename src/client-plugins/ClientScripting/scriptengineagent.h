#ifndef SCRIPTENGINEAGENT_H
#define SCRIPTENGINEAGENT_H

#include <QtScript>
#include <QScriptEngineAgent>

class ScriptEngineAgent: public QScriptEngineAgent
{
    // Q_OBJECT
public:
    ScriptEngineAgent(QScriptEngine *e);
    void exceptionThrow(qint64, const QScriptValue &err, bool);
};

#endif // SCRIPTENGINEAGENT_H
