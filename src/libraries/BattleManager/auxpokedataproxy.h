#ifndef AUXPOKEDATAPROXY_H
#define AUXPOKEDATAPROXY_H

#include <PokemonInfo/pokemonstructs.h>

class PokeProxy;
class ShallowBattlePoke;
class ZoneProxy;

class AuxPokeDataProxy : public QObject
{
    Q_OBJECT
public:
    AuxPokeDataProxy();
    ~AuxPokeDataProxy();

    void setPoke(PokeProxy *poke);
    void onSendOut(ShallowBattlePoke *shallow);
    void onSendBack();

    void setPlayerPoke(bool p);

    Q_INVOKABLE int statBoost(int stat);
    Q_INVOKABLE int stat(int stat);
    Q_INVOKABLE int minStat(int stat);
    Q_INVOKABLE int maxStat(int stat);
    Q_INVOKABLE int type1();
    Q_INVOKABLE int type2();

    Q_PROPERTY(bool onTheField READ isOnTheField NOTIFY onTheFieldChanged)
    Q_PROPERTY(bool substitute READ hasSubstitute NOTIFY substituteChanged)
    Q_PROPERTY(bool showing READ isShowing NOTIFY showingChanged)
    Q_PROPERTY(int alternateSprite READ alternateSpriteRef NOTIFY alternateSpriteChanged)
    Q_PROPERTY(PokeProxy *pokemon READ pokemon NOTIFY pokemonChanged)

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

    PokeProxy *pokemon() const {
        return poke;
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
    void changeForme(int subnum);
    void boostStat(int stat, int level);
    void setBoost(int stat, int level);
    void setStat(int stat, int value);
    void resetStatBoosts();
    void resetStats();

signals:
    void onTheFieldChanged();
    void substituteChanged();
    void showingChanged();
    void alternateSpriteChanged();
    void pokemonChanged();

    void statUp(int stat, int level);
    void statDown(int stat, int level);

public:
    bool onTheField;
    bool substitute;
    bool showing;
    bool playerPoke;
    Pokemon::uniqueId alternateSprite;
    PokeProxy *poke;

    int statboosts[8];
    int basestats[8];
    int boostedstats[8];

    void updateBoostedStat(int stat);
};

class FieldProxy : public QObject {
    Q_OBJECT
public:
    FieldProxy(int numOfSlots=2);
    ~FieldProxy();

    Q_INVOKABLE AuxPokeDataProxy *poke(int num) {
        return auxdata[num];
    }

    Q_INVOKABLE ZoneProxy *zone(int player) {
        return zonedata[player];
    }

    Q_PROPERTY(int weather READ weather NOTIFY weatherChanged)

    void shiftSpots(int spot1, int spot2) {
        std::swap(auxdata[spot1], auxdata[spot2]);
    }

    enum Weather {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    Q_ENUMS(Weather)

    int weather() {return mWeather;}
    void setWeather(int newWeather);
signals:
    void weatherChanged();
private:
    std::vector<AuxPokeDataProxy*> auxdata;
    std::vector<ZoneProxy*> zonedata;

    int mWeather;
};

#endif // AUXPOKEDATAPROXY_H
