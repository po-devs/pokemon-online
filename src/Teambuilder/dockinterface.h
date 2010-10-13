#ifndef DOCKINTERFACE_H
#define DOCKINTERFACE_H

#include <QtGui>
#include "../PokemonInfo/pokemonstructs.h"

class QStackedWidget;
class TB_TeamBody;

class DockAdvanced : public QStackedWidget
{
    Q_OBJECT

 public:
    DockAdvanced(TB_TeamBody * builder);
    ~DockAdvanced();

 public slots:
    void setCurrentPokemon(int index);
    void setPokemonNum(int indexStack, Pokemon::uniqueId);

 private:
    TB_TeamBody * m_builder;

};

#endif // DOCKINTERFACE_H
