#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QtGui>
#include <QProgressBar>

// This is needed for animated sprites. Possible future use for download client! :) - Latios

class Downloader : public QDialog
{
    Q_OBJECT

public:
    Downloader(QWidget *parent = 0, QUrl url = QUrl(""), QString customText = "");
    void initRequest(QUrl url);

private slots:
    void downloadFile();
    void downloadFinished();
    void readyRead();
    void updateProgressRead(qint64 bytesRead, qint64 totalBytes);

private:
    QLabel *statusLabel;
    QProgressBar *downloadProgressBar;
    QPushButton *downloadButton;
    QPushButton *quitCancelButton;
    QDialogButtonBox *buttonBox;
    QUrl downloadUrl;
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QFile *downloadedFile;
    QString customText;
    QTime downloadTime;
    bool requestAborted;
};

#endif // DOWNLOADER_H
