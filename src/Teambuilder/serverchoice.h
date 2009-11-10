#ifndef SERVERCHOICE_H
#define SERVERCHOICE_H

#include <QtGui>

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
