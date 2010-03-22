#ifndef DOCKINTERFACE_H
#define DOCKINTERFACE_H

#include <QtGui>
#include "otherwidgets.h"

class QStackedWidget;
class TB_TeamBody;
class DockAdvanced : public QDockWidget
{
    Q_OBJECT

 public:
    DockAdvanced(TB_TeamBody * builder);
    ~DockAdvanced();

 public slots:
    void setCurrentPokemon(int index);
    void setPokemonNum(int indexStack,int pokeNum);

 private:
    QStackedWidget * AdvStack;
    TB_TeamBody * m_builder;

};

#endif // DOCKINTERFACE_H
