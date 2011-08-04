#ifndef POKEBALLED_H
#define POKEBALLED_H

#include <QWidget>

/* Titles and such with the blue pokeball in front */
class Pokeballed : public QWidget {
public:
    Pokeballed(QWidget *w);
protected:
    Pokeballed();
    void init(QWidget *w);
};

class TitledWidget : public QWidget {
public:
    TitledWidget(const QString &title, QWidget *w);
};


#endif // POKEBALLED_H
