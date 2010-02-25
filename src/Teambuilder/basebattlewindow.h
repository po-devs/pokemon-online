#ifndef BASEBATTLEWINDOW_H
#define BASEBATTLEWINDOW_H

#include <QtGui>
#include "../PokemonInfo/battlestructs.h"

class BaseBattleDisplay;
class QScrollDownTextEdit;

enum {
    Myself,
    Opponent
};

struct BaseBattleInfo
{
    BaseBattleInfo(const QString & me, const QString &opp);
    /* name [0] = mine, name[1] = other */
    QString name[2];
    bool sub[2];

    /* Opponent pokemon */
    ShallowBattlePoke pokes[2];
    bool pokeAlive[2];
    /* Stat boosts & team status */
    BattleDynamicInfo statChanges[2];
};

/* The battle window called by the client, online */
class Client;

class BaseBattleWindow : public QWidget
{
    Q_OBJECT

    PROPERTY(int, battleId);
    PROPERTY(Client *, client);
public:
    BaseBattleInfo *myInfo;
    const BaseBattleInfo &info() const {
        return *myInfo;
    }
    BaseBattleInfo &info() {
        return *myInfo;
    }

    BaseBattleWindow(const QString &me, const QString &opponent);

    enum BattleCommand
    {
        SendOut,
        SendBack,
        UseAttack,
        OfferChoice,
        BeginTurn,
        ChangePP,
        ChangeHp,
        Ko,
        Effective, /* to tell how a move is effective */
        Miss,
        CriticalHit,
        Hit, /* for moves like fury double kick etc. */
        StatChange,
        StatusChange,
        StatusMessage,
        Failed,
        BattleChat,
        MoveMessage,
        ItemMessage,
        NoOpponent,
        Flinch,
        Recoil,
        WeatherMessage,
        StraightDamage,
        AbilityMessage,
        AbsStatusChange,
        Substitute,
        BattleEnd,
        BlankMessage,
        CancelMove,
        Clause,
        DynamicInfo,
        DynamicStats,
        Spectating,
        SpectatorChat
    };

    enum WeatherM
    {
        StartWeather,
        ContinueWeather,
        EndWeather,
        HurtWeather
    };

    enum Weather
    {
        NormalWeather = 0,
        Hail = 1,
        Rain = 2,
        SandStorm = 3,
        Sunny = 4
    };

    enum StatusFeeling
    {
        FeelConfusion,
        HurtConfusion,
        FreeConfusion,
        PrevParalysed,
        PrevFrozen,
        FreeFrozen,
        FeelAsleep,
        FreeAsleep,
        HurtBurn,
        HurtPoison
    };

    virtual void switchToNaught(int spot);

    void printLine(const QString &str);
    void printHtml(const QString &str);
    QString name(int spot) const;
    virtual QString nick(int spot) const;
    QString rnick(int spot) const;

public slots:
    void receiveInfo(QByteArray);
    void sendMessage();
    void clickClose();
signals:
    void battleCommand(const BattleChoice &);
    void battleMessage(const QString &str, int);
    void closedBW(int);
protected:
    QGridLayout *mylayout;
    QScrollDownTextEdit *mychat;
    QLineEdit *myline;
    BaseBattleDisplay *mydisplay;
    QPushButton *myclose, *mysend;

    bool blankMessage;
    bool battleEnded;

    BaseBattleWindow(){}
    void init();

    void closeEvent(QCloseEvent *);
    virtual void dealWithCommandInfo(QDataStream &s, int command, int spot);
};

class BaseGraphicsZone;

class BaseBattleDisplay : public QWidget
{
    Q_OBJECT
public:
    BaseBattleInfo* myInfo;
    BaseBattleInfo &info() const {
        return *myInfo;
    }

    BaseBattleDisplay(BaseBattleInfo &i);

    virtual void updatePoke(int spot);
    virtual void updateToolTip(int spot);
    void changeStatus(int spot, int poke, int status);

protected:
    QString health(int lifePercent);

    BaseGraphicsZone *zone;
    QLabel *nick[2];
    QLabel *status[2];
    QProgressBar *bars[2];
    QLabel *gender[2];
    /* The pokeballs to indicate how well a team is doing */
    QLabel *advpokeballs[6];
    QLabel *mypokeballs[6];
};

/* The graphics zone, where both pokes are displayed */
class BaseGraphicsZone : public QGraphicsView
{
    Q_OBJECT
public:
    BaseGraphicsZone();
    /* displays that poke */
    template <class T>
    void switchTo(const T &poke, int spot, bool sub);
    /* Display blank */
    void switchToNaught(int spot);
    /* For tool tips */
    bool event(QEvent *event);

    /* Loads a pixmap if not loaded otherwise go see graphics */
    QPixmap loadPixmap(quint16 num, bool shiny, bool back, quint8 gender, bool sub);
    /* We are using a qmap to store the graphics already loaded. So the key of the pixmap
        is a combination of 2 bools, 1 quin8; and one quint16 */
    qint32 key(quint16 num, bool shiny, bool back, quint8 gender, bool sub) const;
    QHash<qint32, QPixmap> graphics;
    /* Current pixmaps displayed */
    QGraphicsPixmapItem *mine, *foe;
    QGraphicsScene scene;

    QString tooltips[2];
};

/* Yeepee, at last templates */
template <class T>
void BaseGraphicsZone::switchTo(const T &poke, int spot, bool sub)
{
    if (spot == Myself)
        mine->setPixmap(loadPixmap(poke.num(), poke.shiny(), true, poke.gender(), sub));
    else
        foe->setPixmap(loadPixmap(poke.num(), poke.shiny(), false, poke.gender(), sub));
}

#endif // BASEBATTLEWINDOW_H
