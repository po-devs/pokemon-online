#include "downloader.h"

#include <QtGui>
#include <QtNetwork>

Downloader::Downloader(QWidget *parent, QUrl url, QString cusText)
    : QDialog(parent)
{
    setWindowTitle(tr("Downloader"));

    this->resize(350, this->height());
    this->setMaximumSize(this->width(), this->height());

    customText = cusText;
    downloadUrl = url;
    statusLabel = new QLabel(tr("Grabbing the file to download..."));

    downloadButton = new QPushButton(tr("Download"));
    downloadButton->setDefault(true);
    quitCancelButton = new QPushButton(tr("Quit"));
    quitCancelButton->setDefault(false);

    downloadProgressBar = new QProgressBar(this);
    downloadProgressBar->setTextVisible(true);
    downloadProgressBar->setFixedSize(350, this->height());

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(downloadButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitCancelButton, QDialogButtonBox::RejectRole);

    connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(quitCancelButton, SIGNAL(clicked()), this, SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(statusLabel, 0, Qt::AlignTop);
    layout->addWidget(downloadProgressBar, 0, Qt::AlignLeft);
    layout->addWidget(buttonBox, 0, Qt::AlignBottom);
    setLayout(layout);
}

void Downloader::downloadFile()
{
    QFileInfo fileInfo(downloadUrl.path());
    QString fileName = QString("%1%2").arg("db/pokes/", fileInfo.fileName());
    qDebug() << fileName;
    downloadedFile = new QFile(fileName);
    if(!downloadedFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Downloader"), tr("Unable to save the file %1: %2").arg(fileInfo.fileName()).arg(downloadedFile->errorString()));
        delete downloadedFile;
        downloadedFile = 0;
        return;
    }

    statusLabel->setText(tr("Downloading %1...").arg(fileInfo.fileName()));
    downloadButton->setEnabled(false);
    requestAborted = false;
    downloadTime.start();
    initRequest(downloadUrl);
}

void Downloader::downloadFinished()
{
    if(requestAborted) {
        if(downloadedFile) {
            downloadedFile->close();
            downloadedFile->remove();
            delete downloadedFile;
            downloadedFile = 0;
        }
        reply->deleteLater();
        return;
    }
    downloadedFile->flush();
    downloadedFile->close();
    statusLabel->setText(tr("Downloaded:%1 \n%2").arg(downloadedFile->fileName(), customText));
    downloadTime.restart();
}

void Downloader::initRequest(QUrl url)
{
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgressRead(qint64,qint64)));
}

void Downloader::readyRead()
{
    if(downloadedFile) {
        downloadedFile->write(reply->readAll());
    }
}

void Downloader::updateProgressRead(qint64 bytesRead, qint64 totalBytes)
{
    if(requestAborted) {
        return;
    }
    downloadProgressBar->setMaximum(totalBytes);
    downloadProgressBar->setValue(bytesRead);
    double speed = bytesRead * 1000.0 / downloadTime.elapsed();
    QString to;
    if(speed < 1024) {
        to = "Bytes/s";
    } else {
        if(speed < 1024 * 1024) {
            speed /= 1024;
            to = "KB/s";
        } else {
            speed /= 1024 * 1024;
            to = "MB/s";
        }
    }
    downloadProgressBar->setFormat(QString("%p% (%1 %2)").arg(QString::number(speed), to));
}
