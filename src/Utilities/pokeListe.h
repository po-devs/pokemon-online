#ifndef POKELISTE_H
#define POKELISTE_H

#include <QTableWidget>

class QTableWidgetItem;
class pokeListe : public QTableWidget
{
    Q_OBJECT

public:
    pokeListe(int row,int col,QWidget * parent = 0);
    ~pokeListe();

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    //void dragEnterEvent(QDragEnterEvent * event);
    //void dragMoveEvent(QDragMoveEvent * event);
    //void dropEvent(QDropEvent * event);

private:
    void startDrag();

    QPoint startPos;
    QTableWidgetItem * itemForDrag;
};

#endif // POKELISTE_H
