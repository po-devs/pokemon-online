#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include <QtGui>
#include "centralwidget.h"

/* This is the dialog, when you click on "Go Online" from the menu.
   It requests a hostname/IP address to connect to, and then
   the signal textValueSelected or rejected is emitted */

class QCompactTable;
class Analyzer;

class ServerChoice : public QWidget, public CentralWidgetInterface
{
    Q_OBJECT
public:
    ServerChoice(const QString &nick);
    ~ServerChoice();

    QSize defaultSize() {
        return QSize(500, 450);
    }

public slots:
    void addServer(const QString &name, const QString &desc, quint16 num, const QString &ip, quint16 max, quint16 port);
signals:
    void serverChosen(const QString &ip, const quint16 port, const QString &nick);
    void rejected();
private slots:
    void showDescription(int row);
    void regServerChosen(int row);
    void advServerChosen();
    void connectionError(int , const QString &mess);
private:
    QCompactTable *mylist;
    QLineEdit *myAdvServer;
    QLineEdit *myName;
    QTextBrowser *myDesc;
    Analyzer *registry_connection;

    QHash<QString, QString> descriptionsPerIp;
};

#endif // SERVERCHOICE_H
