#ifndef ZIPUTILS_H
#define ZIPUTILS_H

#ifdef __WIN32
#include "../../SpecialIncludes/zip.h"
#else
#include <zip.h>
#endif

#include <QString>
#include <QVector>

class Zip
{
public:
    Zip();
    ~Zip();

    Zip &create(const QString &path);
    Zip &open(const QString &path);
    void addFile(const QString &path, const QString &zipPath=QString());
    void addMemoryFile(const QByteArray &data, const QString &zipPath);
    bool extractTo(const QString &folder);
    void writeArchive();
    void close();

    operator bool() const;
private:
    zip *archive;

    QString errorStr;
};

#endif // ZIPUTILS_H
