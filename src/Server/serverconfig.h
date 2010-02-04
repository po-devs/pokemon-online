#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QtGui>
#include "security.h"

class ServerWindow : public QWidget
{
    Q_OBJECT
public:
    ServerWindow(QWidget *parent = 0);
signals:
    void nameChanged(const QString &name);
    void descChanged(const QString &desc);
private slots:
    void apply();
private:
    QLineEdit *serverName;
    QTextEdit *serverDesc;
};

#endif // SERVERCONFIG_H
