#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include <QtGui>

/* This is the dialog, when you click on "Go Online" from the menu.
   It requests a hostname/IP address to connect to, and then
   the signal textValueSelected or rejected is emitted */

class QCompactTable;
class Analyzer;
class MainEngine;

class ServerChoice : public QWidget
{
    Q_OBJECT
public:
    ServerChoice();

    QMenuBar* createMenuBar(MainEngine *) {return NULL;}
public slots:
    void addServer(const QString &name, const QString &desc, quint16 num, const QString &ip);
signals:
    void serverChosen(const QString &ip);
    void rejected();
private slots:
    void showDescription(int row);
    void regServerChosen(int row);
    void advServerChosen();
    void connectionError(int , const QString &mess);
private:
    QCompactTable *mylist;
    QLineEdit *myAdvServer;
    QTextEdit *myDesc;
    Analyzer *registry_connection;

    QHash<QString, QString> descriptionsPerIp;
};

#endif // SERVERCHOICE_H
