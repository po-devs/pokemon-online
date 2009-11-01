#ifndef DOCKINTERFACE_H
#define DOCKINTERFACE_H

#include <QDockWidget>

//class dockAdvanced

class QStackedWidget;
class TeamBuilder;
class dockAdvanced : public QDockWidget
{
    Q_OBJECT

 public:
    dockAdvanced(TeamBuilder * builder,QWidget * parent);
    ~dockAdvanced();

 signals:
    void updateDataOfBody(int index);

 public slots:
    void setCurrentPokemon(int index);
    void setPokemonNum(int indexStack,int pokeNum);

private slots:
    void stackAdvancedChanged(int stackIndex);

 private:
    QStackedWidget * AdvancedPokemons_gestionnaire;
    TeamBuilder * m_builder;
    int lastStackIndex;

};

#endif // DOCKINTERFACE_H
