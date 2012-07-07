#include <cerrno>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

#include "ziputils.h"

Zip::Zip() : archive(NULL)
{

}

Zip::~Zip()
{
    close();
}

void Zip::close()
{
    if (archive) {
        zip_close(archive);
        archive = NULL;
        errorStr.clear();
    }
}

Zip& Zip::create(const QString &path)
{
    close();

    QFile rm(path);
    rm.remove();
    rm.close();

    int err=0;
    archive = zip_open(path.toStdString().c_str(), ZIP_CREATE | ZIP_EXCL, &err);

    if (!archive) {
        char buffer[1024];
        zip_error_to_str(buffer, sizeof(buffer), err, errno);
        errorStr = QString(buffer);
    }

    return *this;
}

void Zip::writeArchive()
{
    close();
}

void Zip::addFile(const QString &path, const QString &zipPath)
{
    if (!archive) {
        return;
    }

    QString zpath = zipPath.isEmpty() ? path : zpath;

    zip_source *source = zip_source_file(archive, path.toStdString().c_str(), 0, 0);

    if (!source) {
        errorStr = zip_strerror(archive);
        return;
    }

    if (zip_add(archive, zpath.toStdString().c_str(), source) < 0) {
        errorStr = zip_strerror(archive);
        return;
    }
}

void Zip::addMemoryFile(const QByteArray &data, const QString &zpath)
{
    if (!archive) {
        return;
    }

    char *buf = (char*)malloc(data.length()*sizeof(char) + 1);
    memcpy(buf, data.constData(), data.length()*sizeof(char));

    zip_source *source = zip_source_buffer(archive, buf, data.length(), 1);

    if (!source) {
        errorStr = zip_strerror(archive);
        qDebug() << "impossible to source zip buffer " << errorStr;
        return;
    }

    int res = zip_add(archive, zpath.toStdString().c_str(), source);
    if (res < 0) {
        errorStr = zip_strerror(archive);
        qDebug() << "Failed to add source : " << errorStr;

        zip_source_free(source);
        return;
    }
}

Zip::operator bool() const
{
    return archive != NULL && errorStr.length() == 0;
}
