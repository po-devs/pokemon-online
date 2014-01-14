#ifndef PLAYERSWINDOW_H
#define PLAYERSWINDOW_H

#include "security.h"

class PlayersWindow : public QWidget
{
    Q_OBJECT

public:
    PlayersWindow(QWidget *parent = 0, int expireDays = 30);

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

    void enableSorting(bool);
private:
    QTableWidget *mytable;

    QString currentName();
};

#endif // PLAYERSWINDOW_H
