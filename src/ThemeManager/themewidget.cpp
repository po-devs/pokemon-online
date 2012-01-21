#include "themewidget.h"
#include "ui_themewidget.h"
#include "imageviewerlabel.h"

#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QDebug>
#include <QDialog>

ThemeWidget::ThemeWidget(QString name, QString author, QString version, QString _downloadUrl, QString _forumId, bool direct) :
    QWidget(0),
    downloadUrl(_downloadUrl),
    forumId(_forumId),
    bigImage(0),
    ui(new Ui::ThemeWidget)
{
    ui->setupUi(this);

    ui->themeName->setText(name);
    ui->author->setText(author);
    ui->version->setText(version);

    ui->themeName->adjustSize();
    ui->author->adjustSize();
    ui->version->adjustSize();

    if (downloadUrl.isEmpty()) {
        ui->downloadButton->setDisabled(true);
    }
    if (forumId.isEmpty()) {
        ui->forumButton->setDisabled(true);
    }

    if (direct) {
        ui->downloadButton->setText(tr("Install"));
        connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadAndInstall()));
    } else {
        connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadInBrowser()));
    }

    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));

    connect(ui->forumButton, SIGNAL(clicked()), SLOT(forumThreadButtonClicked()));
    connect(ui->image, SIGNAL(clicked()), SLOT(imageClicked()));
}

ThemeWidget::~ThemeWidget()
{
    delete ui;
}

void ThemeWidget::setImages(QStringList imlist)
{
    imUrls = imlist.toVector();

    if (!imUrls.empty()) {
        current = 0;
        downloadImage();
    }
}

void ThemeWidget::downloadImage()
{
    QNetworkRequest request;
    request.setUrl(QUrl(imUrls[current]));
    request.setRawHeader("User-Agent", "Pokemon-Online Theme Manager");
    downloads[manager.get(request)] = imUrls[current];
}

void ThemeWidget::downloadFinished(QNetworkReply *reply)
{
    QString url = downloads[reply];
    downloads.remove(reply);

    QPixmap im;
    im.loadFromData(reply->readAll());

    // scale down if it is too big
    QRect r = QApplication::desktop()->availableGeometry();
    if (im.width() > 3*r.width()/4)
        im=im.scaledToWidth(3*r.width()/4);
    if (im.height() > 3*r.height()/4)
        im=im.scaledToHeight(3*r.height()/4);

    images[url] = im;

    if (bigImage) {
        bigImage->setPixmap(im);
    }

    if (im.width() > im.height()) {
        ui->image->setPixmap(im.scaledToWidth(120));
    } else {
        ui->image->setPixmap(im.scaledToHeight(120));
    }
}

void ThemeWidget::downloadInBrowser()
{
    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void ThemeWidget::downloadAndInstall()
{
    QNetworkRequest request;
    request.setUrl(QUrl(downloadUrl));
    request.setRawHeader("User-Agent", "Pokemon-Online Theme Manager");
    manager.get(request);
}

void ThemeWidget::forumThreadButtonClicked()
{
    QUrl url = "http://pokemon-online.eu/forums/showthread.php?" + forumId;
    QDesktopServices::openUrl(url);
}

void ThemeWidget::imageClicked()
{
    bigImage = new ImageViewerLabel;
    bigImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    connect(bigImage, SIGNAL(leftPressed()), SLOT(setPrevious()));
    connect(bigImage, SIGNAL(rightPressed()), SLOT(setNext()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(bigImage);
    bigImage->setPixmap(images[imUrls[current]]);

    QDialog* d = new QDialog(this);
    d->setLayout(layout);
    d->setModal(false);
    d->show();
    d->setAttribute(Qt::WA_DeleteOnClose);
    bigImage->setFocus();
    //bigImage = NULL;
}

void ThemeWidget::setPrevious()
{
    if (current > 0) {
        --current;
        if (images.find(imUrls[current]) != images.end()) {
            QPixmap& im = images[imUrls[current]];
            bigImage->setPixmap(im);
            if (im.width() > im.height()) {
                ui->image->setPixmap(im.scaledToWidth(120));
            } else {
                ui->image->setPixmap(im.scaledToHeight(120));
            }

        } else {
            downloadImage();
        }
    }
}

void ThemeWidget::setNext()
{
    if (current+1 < imUrls.size()) {
        ++current;
        if (images.find(imUrls[current]) != images.end()) {
            QPixmap& im = images[imUrls[current]];
            bigImage->setPixmap(im);
            if (im.width() > im.height()) {
                ui->image->setPixmap(im.scaledToWidth(120));
            } else {
                ui->image->setPixmap(im.scaledToHeight(120));
            }

        } else {
            downloadImage();
        }
    }
}
