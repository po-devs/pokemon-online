#ifndef POKELISTE_H
#define POKELISTE_H

#include <QListView>

class pokeListe : public QListView
{
    Q_OBJECT

public:
    pokeListe(QWidget * parent = 0);
    ~pokeListe();

protected:
    void mousePressEvent(QMouseEvent * event);
    //void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    //void dragEnterEvent(QDragEnterEvent * event);
    //void dragMoveEvent(QDragMoveEvent * event);
    //void dropEvent(QDropEvent * event);

private:
    void startDrag();

    QPoint startPos;
};

#endif // POKELISTE_H
