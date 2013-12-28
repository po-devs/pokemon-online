#include "thememanagerwidget.h"
#include "ui_thememanagerwidget.h"

#include "themewidget.h"
#include <QtXml/QDomDocument>
#include <QDebug>

ThemeManagerWidget::ThemeManagerWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ThemeManagerWidget)
{
    ui->setupUi(this);

    connect(ui->getListingButton, SIGNAL(clicked()), SLOT(updateListing()));

    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));

    setWindowTitle("Theme Manager");
}

ThemeManagerWidget::~ThemeManagerWidget()
{
    delete ui;
}

void ThemeManagerWidget::updateListing()
{
    QString url = "http://valssi.fixme.fi/~lamperi/pokemononline/themes.xml";

    ui->getListingButton->setDisabled(true);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "Pokemon-Online Theme Manager");
    QNetworkReply* reply = manager.get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)));
}

void ThemeManagerWidget::downloadFinished(QNetworkReply *reply)
{
    //ui->progressBar->setValue(100);
    ui->getListingButton->setDisabled(false);

    QString data = reply->readAll();
    QDomDocument document;
    document.setContent(data);
    qDebug() << data;
    qDebug() << document.firstChild().nodeName();
    QDomElement themeNode = document.firstChild().firstChildElement("Themes/Current");
    while (!themeNode.isNull()) {

        QString name = themeNode.attribute("name");
        QString author = themeNode.attribute("author");
        QString version = themeNode.attribute("version");
        QString downloadUrl = themeNode.attribute("downloadurl");
        QString forumId = themeNode.attribute("forumthread");
        bool directDownload = (themeNode.attribute("direct") == "yes");

        QStringList images;
        QDomElement imageNode = themeNode.firstChildElement("preview");
        while (!imageNode.isNull()) {
            qDebug() << "imagaNode" << &imageNode;
            images << imageNode.firstChild().toText().data();
            imageNode = imageNode.nextSiblingElement("preview");
        }
        themeNode = themeNode.nextSiblingElement("Themes/Current");

        qDebug() << "before creating themewidget";
        ThemeWidget *widget = new ThemeWidget(name, author, version, downloadUrl, forumId, directDownload);
        qDebug() << "after creating themewidget";
        widget->setImages(images);
        ui->themeWidgets->addWidget(widget);
    }
    ui->scrollAreaWidgetContents->adjustSize();
}


void ThemeManagerWidget::downloadProgress(qint64 done, qint64 total)
{
    (void) done;
    (void) total;
    //double percent = total <= 0 ? 0 : static_cast<double>(done)/total;
    //ui->progressBar->setValue(percent);
}

