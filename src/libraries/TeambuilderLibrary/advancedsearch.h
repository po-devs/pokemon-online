#ifndef ADVANCEDSEARCH_H
#define ADVANCEDSEARCH_H

#include <QWidget>
#include <PokemonInfo/pokemonstructs.h>

namespace Ui {
class AdvancedSearch;
}

class AdvancedSearch : public QWidget
{
    Q_OBJECT
    
public:
    explicit AdvancedSearch(QWidget *parent = 0);
    ~AdvancedSearch();
    
    void setGen(const Pokemon::gen &gen);
    void setResultsWidth(int px);
public slots:
    void search();
    void emitNum(const QModelIndex&);
signals:
    void pokemonSelected(const Pokemon::uniqueId &id);
private:
    Ui::AdvancedSearch *ui;

    Pokemon::gen gen;
};

#endif // ADVANCEDSEARCH_H
