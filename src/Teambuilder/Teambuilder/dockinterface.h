#ifndef DOCKINTERFACE_H
#define DOCKINTERFACE_H

#include <QStackedWidget>
#include "../PokemonInfo/pokemonstructs.h"

class TB_TeamBody;

class DockAdvanced : public QStackedWidget
{
    Q_OBJECT

 public:
    DockAdvanced(TB_TeamBody * builder);
    ~DockAdvanced();

    void changeGeneration(int gen);
    void createAdvanced(int index);

 public slots:
    void setCurrentPokemon(int index);
    void updatePokemonNum(int indexStack);

 private:
    TB_TeamBody * m_builder;

    QWidget *advanced[6];

};

#endif // DOCKINTERFACE_H
