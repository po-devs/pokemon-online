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
signals:
    void updatesAvailable(const QString, bool error);
    void changeLogAvailable(const QString, bool error);
private slots:
    void onUpdateFileDownloaded();
private:
    QNetworkAccessManager manager;
    QHash<QNetworkReply*, QPair<QObject*, const char*> > replies;

    void download(const QString &url, QObject *target, const char *slot);
    void readAvailableUpdatesFromFile();
    void loadCurrentUpdateId();
    bool isValidUpdateElement(const QDomElement &el);

    QString targetDownload;
    QString changeLogUrl;
    int currentUpdateId;
};

#endif // DOWNLOADMANAGER_H
