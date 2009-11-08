#ifndef DOCKINTERFACE_H
#define DOCKINTERFACE_H

#include <QtGui>

//class dockAdvanced

class QStackedWidget;
class TeamBuilder;
class DockAdvanced : public QDockWidget
{
    Q_OBJECT

 public:
    DockAdvanced(TeamBuilder * builder);
    ~DockAdvanced();

 signals:
    void updateDataOfBody(int index);

 public slots:
    void setCurrentPokemon(int index);
    void setPokemonNum(int indexStack,int pokeNum);

private slots:
    void stackAdvancedChanged();

 private:
    QStackedWidget * AdvancedPokemons_gestionnaire;
    TeamBuilder * m_builder;
    int lastStackIndex;

};

#endif // DOCKINTERFACE_H
