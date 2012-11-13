#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QHash>

class QDomElement;

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = 0);
    
    void loadUpdatesAvailable();
    void loadChangelog();

    /* Calls the slot with the network reply as the sender. Up to you
      to deleteLater it! */
    void download(const QString &url, QObject *target, const char *slot);
    bool updateReady() const;
    bool isDownloading() const { return downloading;}
public slots:
    void downloadUpdate();
signals:
    void updatesAvailable(const QString, bool error);
    void changeLogAvailable(const QString, bool error);
    void readyToRestart();
private slots:
    void onUpdateFileDownloaded();
    void onChangeLogDownloaded();
    void updateDownloaded();
private:
    QNetworkAccessManager manager;
    QHash<QNetworkReply*, QPair<QObject*, const char*> > replies;

    void readAvailableUpdatesFromFile();
    void readChangeLogFromFile();
    void loadCurrentUpdateId();
    bool isValidUpdateElement(const QDomElement &el);

    QString targetDownload;
    QString changeLogUrl;
    int currentUpdateId;

    bool downloading;

    void extractZip(const QString &path);
};

#endif // DOWNLOADMANAGER_H
