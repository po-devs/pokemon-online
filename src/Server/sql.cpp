#include "sql.h"

int SQLCreator::databaseType = SQLCreator::SQLite;
QMutex SQLCreator::mutex;
QString SQLCreator::databaseSchema;
bool SQLCreator::doVacuum;
