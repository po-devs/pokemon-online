#ifndef AUXPOKEDATAPROXY_H
#define AUXPOKEDATAPROXY_H

#include "../PokemonInfo/pokemonstructs.h"

class PokeProxy;

class AuxPokeDataProxy : public QObject
{
    Q_OBJECT
public:
    AuxPokeDataProxy();

    void onSendOut(PokeProxy *poke);
    void onSendBack();

    Q_INVOKABLE int statBoost(int stat);
    Q_INVOKABLE int stat(int stat);

    Q_PROPERTY(bool onTheField READ isOnTheField NOTIFY onTheFieldChanged)
    Q_PROPERTY(bool substitute READ hasSubstitute NOTIFY substituteChanged)
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

    void statUp(int stat, int level);
    void statDown(int stat, int level);

public:
    bool onTheField;
    bool substitute;
    bool showing;
    Pokemon::uniqueId alternateSprite;
    PokeProxy *poke;

    int statboosts[8];
    int basestats[8];
    int boostedstats[8];

    void updateBoostedStat(int stat);
};

class ZoneProxy : public QObject {
    Q_OBJECT
public:
    ZoneProxy();
    ~ZoneProxy();

    enum Hazards {
        Spikes=1,
        SpikesLV2=2,
        SpikesLV3=4,
        StealthRock=8,
        ToxicSpikes=16,
        ToxicSpikesLV2=32
    };


    Q_PROPERTY(int spikes READ spikesLevel NOTIFY spikesChanged)
    Q_PROPERTY(int toxicSpikes READ tspikesLevel NOTIFY tspikesChanged)
    Q_PROPERTY(bool stealthRocks READ stealthRocks NOTIFY rocksChanged)

    int spikesLevel() const {return mSpikes;}
    int tspikesLevel() const {return mTSpikes;}
    bool stealthRocks() const {return mRocks;}
    void setSpikesLevel(int level);
    void setToxicSpikesLevel(int level);
    void setStealthRocks(bool stealthRocks);

    void setHazards(quint8 hazards);
signals:
    void spikesChanged();
    void tspikesChanged();
    void rocksChanged();
private:
    int mSpikes;
    int mTSpikes;
    bool mRocks;
};

class FieldProxy : public QObject {
    Q_OBJECT
public:
    FieldProxy();
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
