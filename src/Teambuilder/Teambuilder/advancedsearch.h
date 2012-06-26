#ifndef ADVANCEDSEARCH_H
#define ADVANCEDSEARCH_H

#include <QWidget>

namespace Ui {
class AdvancedSearch;
}

class AdvancedSearch : public QWidget
{
    Q_OBJECT
    
public:
    explicit AdvancedSearch(QWidget *parent = 0);
    ~AdvancedSearch();
    
    void setResultsWidth(int px);
private:
    Ui::AdvancedSearch *ui;
};

#endif // ADVANCEDSEARCH_H
