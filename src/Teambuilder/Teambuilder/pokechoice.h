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
    TB_PokeChoice(QAbstractItemModel *model, bool missingno);
    QSize sizeHint() const;
signals:
    void pokemonActivated(const Pokemon::uniqueId &num);
public slots:
    void activatedCell(const QModelIndex &index);
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    void startDrag();

    QPoint startPos;
    QModelIndex dragIndex;
};

#endif // POKECHOICE_H
