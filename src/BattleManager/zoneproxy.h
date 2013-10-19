#ifndef ZONEPROXY_H
#define ZONEPROXY_H

#include <QObject>

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
        ToxicSpikesLV2=32,
        StickyWeb = 64
    };


    Q_PROPERTY(int spikes READ spikesLevel NOTIFY spikesChanged)
    Q_PROPERTY(int toxicSpikes READ tspikesLevel NOTIFY tspikesChanged)
    Q_PROPERTY(bool stealthRocks READ stealthRocks NOTIFY rocksChanged)
    Q_PROPERTY(bool stickyWeb READ stickyWeb NOTIFY stickyWebChanged)

    int spikesLevel() const {return mSpikes;}
    int tspikesLevel() const {return mTSpikes;}
    bool stealthRocks() const {return mRocks;}
    bool stickyWeb() const {return mStickyWeb;}
    void setSpikesLevel(int level);
    void setToxicSpikesLevel(int level);
    void setStealthRocks(bool stealthRocks);
    void setStickyWeb(bool stickyWeb);

    void setHazards(quint8 hazards);
signals:
    void spikesChanged();
    void tspikesChanged();
    void rocksChanged();
    void stickyWebChanged();
private:
    int mSpikes;
    int mTSpikes;
    bool mRocks;
    bool mStickyWeb;
};

#endif // ZONEPROXY_H
