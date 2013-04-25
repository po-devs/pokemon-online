#include <ctime>
#include <QNetworkReply>
#include <QDomDocument>
#include <QMessageBox>

#include "downloadmanager.h"
#include "../Utilities/functions.h"
#include "../Shared/config.h"
#include "../Utilities/ziputils.h"
#ifdef QT5
#include <QtConcurrent>
#endif

#ifdef __WIN32
#include <windows.h>
#include <windef.h>
#include <Shellapi.h>
#endif

inline QString getXmlFileName()
{
    int major = CLIENT_VERSION_NUMBER/1000;
    int minor = (CLIENT_VERSION_NUMBER%1000)/100;

    return QString("%1.%2.xml").arg(major).arg(minor);
}

DownloadManager::DownloadManager(QObject *parent) :
    QObject(parent), currentUpdateId(-1), downloading(false)
{
}

void DownloadManager::loadUpdatesAvailable()
{
    QSettings settings;

    int time = settings.value("Updates/LastUpdateCheck").toInt();

    /* If the update information is older than 3 days... */
    if (::time(NULL) - time > 24*3600*3) {
        download(QString("https://raw.github.com/coyotte508/pokemon-online/master/updates/%1").arg(getXmlFileName()),this,SLOT(onUpdateFileDownloaded()));
    } else {
        readAvailableUpdatesFromFile();
    }
}

void DownloadManager::loadChangelog()
{
    QSettings settings;

    if (settings.value("Updates/ChangeLogDownloaded") != changeLogUrl) {
        download(changeLogUrl, this, SLOT(onChangeLogDownloaded()));
    } else {
        readChangeLogFromFile();
    }
}

bool DownloadManager::updateReady() const
{
    QSettings settings;
    return settings.value("Updates/Ready").toBool();
}

void DownloadManager::download(const QString &url, QObject *target, const char *slot)
{
    qDebug() << "Downloading " << url;
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "Pokemon-Online Updater");

    QNetworkReply* reply = manager.get(request);
    connect(reply, SIGNAL(finished()), target, slot);
}

void DownloadManager::onUpdateFileDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender());

    if (!reply) {
        return;
    }

    reply->deleteLater();

    if (reply->error()) {
        emit updatesAvailable(reply->errorString(), true);
        return;
    }

    QFile out(appDataPath("Updates/", true) + getXmlFileName());
    out.open(QIODevice::WriteOnly);

    if (!out.isOpen()) {
        /* Error while downloading */
        emit updatesAvailable(tr("Impossible to see available updates: impossible to write to %1.").arg(appDataPath("Updates/")+getXmlFileName()), true);
        return;
    }
    QByteArray response = reply->readAll();

    out.write(response);
    out.close();

    QSettings settings;
    settings.setValue("Updates/LastUpdateCheck", QString::number(::time(NULL)));

    readAvailableUpdatesFromFile();
}

void DownloadManager::downloadUpdate()
{
    qDebug() << "Download update requested";
    if (targetDownload.isNull()) {
        QMessageBox::warning(NULL, tr("No download link found"), tr("The update data doesn't contain any valid download link!"));
        return;
    }

    if (updateReady()) {
        qDebug() << "Update already ready";
        return; // No downloading the same update twice
    }

    if (downloading) {
        qDebug() << "Update already downloading";
        return;
    }

    download(targetDownload, this, SLOT(updateDownloaded()));
}

void DownloadManager::updateDownloaded()
{
    qDebug() << "Update finished downloading";
    QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender());

    if (!reply) {
        return;
    }

    reply->deleteLater();

    if (updateReady()) {
        return; //sad but necessary
    }

    if (reply->error()) {
        QMessageBox::critical(NULL, tr("Update download failed"), reply->errorString());
        return;
    }

    QFileInfo info(QUrl(targetDownload).toString());

    qDebug() << "File: " << info.fileName();

    QString path = appDataPath("Updates/", true) + info.fileName();

    QFile out(path);
    out.open(QIODevice::WriteOnly);

    if (!out.isOpen()) {
        qDebug() << "Error when writing to file " << path << "for updates.";
        return;
    }

    QByteArray data = reply->readAll();

    qDebug() << "Downloaded data size: " << data.length();

    out.write(data);

    if (out.error() == QFile::NoError) {
        loadCurrentUpdateId();

        QSettings settings;
        settings.setValue("Updates/ZipDownloadedFor", currentUpdateId);
        settings.setValue("Updates/ZipPath", path);
        settings.setValue("Updates/ZipDownloaded", true);

        QtConcurrent::run(this,&DownloadManager::extractZip, path);
    }
}

void DownloadManager::extractZip(const QString &path)
{
    Zip zip;
    if (!zip.open(path)) {
        return;
    }

    /* The zip path, without the '.zip' at the end */
    QString targetDir = QFileInfo(path).path() + "/" + QFileInfo(path).baseName();

    removeFolder(targetDir);

    if (!zip.extractTo(targetDir)) {
        /* Error while extracting, remove the file etc. */
        zip.close();

        QSettings settings;
        settings.setValue("Updates/ZipDownloaded", false);
        QFile f(path);
        f.remove();

        return;
    }

    qDebug() << "Download extracted, ready to restart!";

    QDir target(targetDir);
    /* updating auto updaters from PO, because it can't update itself :o */
    QStringList autoUpdaters = target.entryList(QStringList() << "*maintenance*", QDir::Files);

    if (autoUpdaters.length() > 0) {
        qDebug() << "Found " << autoUpdaters.length() << " auto updaters";
        /* Todo: check if an auto updater is currently running */
        if (testWritable(target.relativeFilePath(autoUpdaters.front()))) {
            //QMessageBox::information(NULL, "test", QString("%1 is writable!").arg(target.relativeFilePath(autoUpdaters.front())));

            foreach(QString autoUpdater, autoUpdaters) {
                QString rel = target.relativeFilePath(autoUpdater);
                /* Todo: check if those 3 lines are necessary ? */
                QFile s (rel);
                s.remove();
                s.close();

                QFile f (target.absoluteFilePath(autoUpdater));
                if (!f.rename(QDir().absoluteFilePath(rel))) {
                    QMessageBox::critical(NULL, tr("Error during PO update"), tr("Couldn't update file %1.").arg(rel));
                    return;
                }
            }
        } else {
            QString params;
            foreach(QString autoUpdater, autoUpdaters) {
                params += "-update '" + target.relativeFilePath(autoUpdater) + "' '"
                        + target.absoluteFilePath(autoUpdater) + "' ";
            }

#ifdef __WIN32
            QString current = qApp->arguments().front();

            QMessageBox::information(NULL, "PO Update", QString("Pokemon Online will need administrative permissions to continue the update process.").arg(current));

            /* Spawn a new process as admin to move the files */
            bool error = int(::ShellExecute(0, // owner window
                           L"runas",
                           (LPCWSTR)current.utf16(), //Current exe
                           (LPCWSTR)params.utf16(), // params
                           0, // directory
                           SW_SHOWNORMAL)) <= 32;

            if (error) {
                QMessageBox::information(NULL, "Error updating the maintenance tool", "Error with shell execute");
            }
#else
            //Todo: what do on mac / linux? :o
            QMessageBox::critical(NULL, tr("Error during PO update"), tr("Couldn't update file %1.").arg(target.relativeFilePath(autoUpdaters.front())));
            return;
#endif
        }
    }

    QSettings settings;

    settings.setValue("Updates/ReadyFor", currentUpdateId);
    settings.setValue("Updates/ReadyTarget", targetDir);
    settings.setValue("Updates/Ready", true);

    qDebug() << "Update preinstalled, ready to restart.";

    emit readyToRestart();
}

void DownloadManager::onChangeLogDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender());

    if (!reply) {
        return;
    }

    reply->deleteLater();

    if (reply->error()) {
        emit changeLogAvailable(reply->errorString(), true);
        return;
    }

    QFile out(appDataPath("Updates/", true) + "changelog.txt");
    out.open(QIODevice::WriteOnly);

    if (!out.isOpen()) {
        /* Error while downloading */
        emit updatesAvailable(tr("Impossible to load changelog: impossible to write to %1.").arg(appDataPath("Updates/")+"changelog.txt"), true);
        return;
    }

    QByteArray response = reply->readAll();

    out.write(response);
    out.close();

    QSettings settings;
    settings.setValue("Updates/ChangeLogDownloaded", changeLogUrl);

    readChangeLogFromFile();
}


void DownloadManager::loadCurrentUpdateId()
{
    if (currentUpdateId != -1) {
        return;
    }

    QSettings in("version.ini", QSettings::IniFormat);
    currentUpdateId = std::max(in.value("updateId").toInt(), UPDATE_ID);
}

void DownloadManager::readAvailableUpdatesFromFile()
{
    QFile in(appDataPath("Updates/") + getXmlFileName());

    if (!in.exists()) {
        return;
    }

    in.open(QIODevice::ReadOnly);

    QString data = QString::fromUtf8(in.readAll());

    QDomDocument doc;

    if (!doc.setContent(data)) {
        return;
    }

    QDomElement el = doc.firstChildElement("updates");

    el = el.firstChildElement("update");

    while (!el.isNull() && !isValidUpdateElement(el)) {
        el = el.nextSiblingElement("update");
    }

    if (!el.isNull()) {
        targetDownload = el.attribute("download");
        changeLogUrl = el.attribute("changelog");

        QString versionTo = el.attribute("versionTo");

        if (versionTo.length() > 0 &&  versionTo != VERSION) {
            emit updatesAvailable(tr("An update to version %1 is available!").arg(versionTo), false);
        } else {
            emit updatesAvailable(tr("An update is available!"), false);
        }
    }
}

void DownloadManager::readChangeLogFromFile()
{
    QFile in(appDataPath("Updates/")+"changelog.txt");

    if (!in.exists()) {
        return;
    }

    in.open(QIODevice::ReadOnly);

    emit changeLogAvailable(QString::fromUtf8(in.readAll()), false);
}

bool DownloadManager::isValidUpdateElement(const QDomElement &el)
{
    if (!el.hasAttribute("updateIdFrom") || !el.hasAttribute("os")) {
        return false;
    }

    loadCurrentUpdateId();

    int minId, maxId;

    minId = el.attribute("updateIdFrom").section(",", 0, 0).trimmed().toInt();
    maxId = el.attribute("updateIdFrom").section(",", -1, -1).trimmed().toInt();

    if (currentUpdateId < minId || currentUpdateId > maxId) {
        return false;
    }

    QString os = el.attribute("os");

    if (os != "all" && os != OS) {
        return false;
    }

    return true;
}
