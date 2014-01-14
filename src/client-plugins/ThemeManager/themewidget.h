#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "imageviewerlabel.h"

namespace Ui {
    class ThemeWidget;
}

class ThemeWidget : public QWidget
{
    Q_OBJECT

public:
    ThemeWidget(QString name, QString author, QString version, QString downloadUrl, QString forumId, bool direct);

    ~ThemeWidget();

    void setImages(QStringList image);

private slots:
    void forumThreadButtonClicked();

    void downloadInBrowser();
    void downloadAndInstall();

    void downloadImage();
    void downloadFinished(QNetworkReply*);

    void imageClicked();

    void setPrevious();
    void setNext();

private:
    QNetworkAccessManager manager;

    QHash<QString, QPixmap> images;
    QHash<QNetworkReply*, QString> downloads;

    QString downloadUrl;
    QString forumId;

    QVector<QString> imUrls;
    int current;

    ImageViewerLabel* bigImage;

    Ui::ThemeWidget *ui;
};

#endif // THEMEWIDGET_H
