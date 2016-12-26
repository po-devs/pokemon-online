#ifndef BATTLEWINDOWATTACKBUTTON_H
#define BATTLEWINDOWATTACKBUTTON_H

#include <PokemonInfo/battlestructs.h>
#include <QPushButton>
#include <Utilities/otherwidgets.h>

class QLabel;
class PokeProxy;

class AbstractAttackButton
{
public:
    void init();

    void updateAttack(const BattleMove& b, const PokeProxy &p, Pokemon::gen gen, bool zmove);

    virtual void updateGui();

    QAbstractButton *pointer() {
        return dynamic_cast<QAbstractButton *> (this);
    }

    QString power() const;
    int type() const;
    int num() const;
    QString tooltip() const;
    QString moveName() const;

    QLabel *name;
    QLabel *pp;

protected:
    bool zmove, validZmove;
    const BattleMove *b;
    const PokeProxy *p;
    Pokemon::gen gen;
};

class ImageAttackButton : public QImageButton, public AbstractAttackButton
{
    Q_OBJECT
public:
    ImageAttackButton(const BattleMove& b, const PokeProxy &p, Pokemon::gen gen);
    virtual void updateGui();
};

class OldAttackButton : public QPushButton, public AbstractAttackButton
{
    Q_OBJECT
public:
    OldAttackButton(const BattleMove& b, const PokeProxy &p, Pokemon::gen gen);
    virtual void updateGui();
};

#endif // BATTLEWINDOWATTACKBUTTON_H
