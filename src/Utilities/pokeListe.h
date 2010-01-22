#ifndef POKELISTE_H
#define POKELISTE_H

#include <QListView>

class pokeListe : public QListView
{
    Q_OBJECT

public:
    pokeListe(QWidget * parent = 0);
    ~pokeListe();
};

#endif // POKELISTE_H
