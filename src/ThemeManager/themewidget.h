#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
    class ThemeWidget;
}

class ThemeWidget : public QWidget
{
    Q_OBJECT

public:
    ThemeWidget(QString name, QString author, QString version, QString downloadUrl, QString forumId);

    ~ThemeWidget();

    void setThemeImage(QString image);

private slots:
    void forumThreadButtonClicked();

    void downloadButtonClicked();

    void downloadFinished(QNetworkReply*);

    void imageClicked();

private:
    QNetworkAccessManager manager;

    QPixmap im;

    QString downloadUrl;
    QString forumId;

    Ui::ThemeWidget *ui;
};

#endif // THEMEWIDGET_H
