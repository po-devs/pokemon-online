#include <cerrno>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>
#include <QDir>

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

Zip& Zip::open(const QString &path)
{
    close();

    qDebug() << "Zip utils: opening file " << path;

    int err=0;
    archive = zip_open(path.toStdString().c_str(), 0, &err);

    if (!archive) {
        char buffer[1024];
        zip_error_to_str(buffer, sizeof(buffer), err, errno);
        errorStr = QString(buffer);

        qDebug() << "Error when opening the archive: " << errorStr;
    } else {
        //qDebug() << "Num files: " << zip_get_num_entries(archive, 0); // <-- when everyone has libzip2, use that
        qDebug() << "Num files: " << zip_get_num_files(archive);
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

    QString zpath = zipPath.isEmpty() ? path : zipPath;

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

bool Zip::extractTo(const QString &folder)
{
    QDir d(QFileInfo(folder).path());

    QString baseName = QFileInfo(folder).baseName();

    if (!d.mkpath(baseName)) {
        errorStr = QString("Impossible to create path %1").arg(folder);
        return false;
    }

    if (!d.cd(baseName)) {
        errorStr = QString("Impossible to go in folder %1 to extract the archive.").arg(folder);
        return false;
    }

    //int numFiles = zip_get_num_entries(archive, 0); // <-- when everyone has libzip2, use that
    int numFiles = zip_get_num_files(archive);

    qDebug() << "Number of files in the archive: " << numFiles;

    if (numFiles == 0) {
        errorStr = QString("Empty archive.");
        return false;
    }

    for (int i = 0; i < numFiles; i++) {
        QString name = zip_get_name(archive, i, 0);

        qDebug() << "File " << i << ": " << name;

        /* Mod.ini is already open */
        zip_file* file = zip_fopen_index(archive, i, 0);

        if (!file) {
            errorStr = QString("Error when extracting file %1 in the archive: %2.").arg(name, zip_strerror(archive));
            //error
            return false;
        }

        d.mkpath(QFileInfo(name).path());
        QFile out(d.absoluteFilePath(name));
        out.open(QIODevice::WriteOnly);

        int readsize = 0;

        char buffer[4096];

        do
        {
            out.write(buffer, readsize);

            readsize = zip_fread(file, buffer, 4096);
        } while (readsize > 0) ;

        out.close();

        zip_fclose(file), file = NULL;
    }

    return true;
}
