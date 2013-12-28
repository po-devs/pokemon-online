#ifndef THEMEMANAGERWIDGET_H
#define THEMEMANAGERWIDGET_H

#include <QDialog>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Ui {
    class ThemeManagerWidget;
}

class ThemeManagerWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ThemeManagerWidget(QWidget *parent = 0);
    ~ThemeManagerWidget();

private slots:
    void updateListing();

    void downloadFinished(QNetworkReply*);

    void downloadProgress(qint64 done, qint64 progress);

private:
    QNetworkAccessManager manager;

    Ui::ThemeManagerWidget *ui;
};

#endif // THEMEMANAGERWIDGET_H
