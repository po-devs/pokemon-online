#ifndef PLAYERSWINDOW_H
#define PLAYERSWINDOW_H

#include <QtGui>
#include "security.h"

class PlayersWindow : public QWidget
{
    Q_OBJECT
public:
    PlayersWindow(QWidget *parent = 0);
signals:
    void banned(const QString &name);
    void authChanged(const QString &name, int auth);
private slots:
    void ban();
    void unban();
    void user();
    void mod();
    void admin();
    void owner();
    void clpass();
private:
    QTableWidget *mytable;

    QString currentName();
};

#endif // PLAYERSWINDOW_H
