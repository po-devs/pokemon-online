#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include <QtGui>

/* This is the dialog, when you click on "Go Online" from the menu.
   It requests a hostname/IP address to connect to, and then
   the signal textValueSelected or rejected is emitted */

class ServerChoice : public QInputDialog
{
    Q_OBJECT
public:
    ServerChoice();
private slots:
    void textSelected(const QString &text);
    /* SIGNALS:
	textValueSelected: when pressed Ok
	rejected: when pressed Cancel
	    */
};

#endif // SERVERCHOICE_H
