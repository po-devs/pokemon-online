#ifndef POKECHOICE_H
#define POKECHOICE_H

#include <QTableView>

namespace Pokemon {
    class uniqueId;
}

/* The list of pokemons */
class TB_PokeChoice : public QTableView
{
    Q_OBJECT

public:
    TB_PokeChoice(QWidget *parent=NULL);
    TB_PokeChoice(QAbstractItemModel *model, bool missingno);
    QSize sizeHint() const;

    void setModel(QAbstractItemModel *model);
signals:
    void pokemonSelected(const Pokemon::uniqueId &num);
    void pokemonActivated(const Pokemon::uniqueId &num);
public slots:
    void activatedCell(const QModelIndex &index);
    void selectedCell(const QModelIndex &index);
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void keyPressEvent(QKeyEvent * event);

private:
    using QAbstractItemView::startDrag;
    void startDrag();
    void init();

    QPoint startPos;
    QModelIndex dragIndex;
};

#endif // POKECHOICE_H
