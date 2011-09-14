#ifndef AUXPOKEDATAPROXY_H
#define AUXPOKEDATAPROXY_H

#include "../PokemonInfo/pokemonstructs.h"

class AuxPokeDataProxy : public QObject
{
    Q_OBJECT
public:
    AuxPokeDataProxy();

    void onSendOut();
    void onSendBack();

    Q_PROPERTY(bool onTheField READ isOnTheField NOTIFY onTheFieldChanged)
    Q_PROPERTY(bool subsitute READ hasSubstitute NOTIFY substituteChanged)
    Q_PROPERTY(bool showing READ isShowing NOTIFY showingChanged)
    Q_PROPERTY(int alternateSprite READ alternateSpriteRef NOTIFY alternateSpriteChanged)

    bool isOnTheField() {
        return onTheField;
    }

    bool hasSubstitute() {
        return substitute;
    }

    bool isShowing() {
        return showing;
    }

    int alternateSpriteRef() {
        return alternateSprite.toPokeRef();
    }

    /* Macro to generate setters... */
#define helper_macro(setter, type, argument) \
    void setter(type argument) {\
        if (this->argument == argument) return;\
        this->argument = argument;\
        emit argument##Changed();\
    }

    helper_macro(setOnTheField, bool, onTheField)
    helper_macro(setSubstitute, bool, substitute)
    helper_macro(setShowing, bool, showing)
    helper_macro(setAlternateSprite, Pokemon::uniqueId, alternateSprite)
#undef helper_macro

    void vanish() {setShowing(false);}
    void reappear() {setShowing(true);}
    void changeSprite(Pokemon::uniqueId sprite) {setAlternateSprite(sprite);}
    void changeForme(int subnum) {setAlternateSprite(Pokemon::uniqueId(alternateSprite.pokenum, subnum));}

signals:
    void onTheFieldChanged();
    void substituteChanged();
    void showingChanged();
    void alternateSpriteChanged();

public:
    bool onTheField;
    bool substitute;
    bool showing;
    Pokemon::uniqueId alternateSprite;
};

class FieldProxy : public QObject {
    Q_OBJECT
public:
    FieldProxy() {
        /* Resizes for triple. Later, when loaded with battle configuration, will get
          more accurate loading */
        for (int i = 0; i < 6; i++) {
            auxdata[i] = new AuxPokeDataProxy();
        }
    }

    ~FieldProxy() {
        for (int i = 0; i < 6; i++) {
            delete auxdata[i];
        }
    }

    Q_INVOKABLE AuxPokeDataProxy *poke(int num) {
        return auxdata[num];
    }

    void shiftSpots(int spot1, int spot2) {
        std::swap(auxdata[spot1], auxdata[spot2]);
    }

    std::vector<AuxPokeDataProxy*> auxdata;
};

#endif // AUXPOKEDATAPROXY_H
