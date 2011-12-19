#include "themewidget.h"
#include "ui_themewidget.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

ThemeWidget::ThemeWidget(QString name, QString author, QString version, QString _downloadUrl, QString _forumId) :
    QWidget(0),
    ui(new Ui::ThemeWidget),
    downloadUrl(_downloadUrl),
    forumId(_forumId)
{
    ui->setupUi(this);

    ui->themeName->setText(name);
    ui->author->setText(author);
    ui->version->setText(version);

    if (downloadUrl.isEmpty()) {
        ui->downloadButton->setDisabled(true);
    }
    if (forumId.isEmpty()) {
        ui->forumButton->setDisabled(true);
    }

    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));

    connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadButtonClicked()));
    connect(ui->forumButton, SIGNAL(clicked()), SLOT(forumThreadButtonClicked()));
}

ThemeWidget::~ThemeWidget()
{
    delete ui;
}

void ThemeWidget::setThemeImage(QString image)
{

    QNetworkRequest request;
    request.setUrl(QUrl(image));
    request.setRawHeader("User-Agent", "Pokemon-Online Theme Manager");
    manager.get(request);
}

void ThemeWidget::downloadFinished(QNetworkReply *reply)
{
    QPixmap im;
    im.loadFromData(reply->readAll());
    if (im.width() > im.height()) {
        ui->image->setPixmap(im.scaledToWidth(120));
    } else {
        ui->image->setPixmap(im.scaledToHeight(120));
    }
}

void ThemeWidget::downloadButtonClicked()
{
    qDebug() << "Opening url" << downloadUrl;
    QDesktopServices::openUrl(QUrl(downloadUrl));
}

void ThemeWidget::forumThreadButtonClicked()
{
    QUrl url = "http://pokemon-online.eu/forums/showthread.php?" + forumId;
    QDesktopServices::openUrl(url);
}

