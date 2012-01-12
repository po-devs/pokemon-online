#include "evmanager.h"
#include "../../PokemonInfo/pokemonstructs.h"
#include <QGridLayout>
#include "theme.h"
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include "../../Utilities/otherwidgets.h"
#include "../../PokemonInfo/pokemoninfo.h"
#include "theme.h"
#include "../Utilities/qimagebuttonlr.h"

TB_EVManager::TB_EVManager(PokeTeam *_poke)
{
    m_poke = _poke;

    myStatUp = -1;
    myStatDown = -1;
    QGridLayout *l = new QGridLayout(this);
    l->setSpacing(0);

    QString labels[6] = {tr("Hit Points:"), tr("Attack:"), tr("Defense:"), tr("Special Attack:")
                         , tr("Special Defense:"), tr("Speed:")};

    for (int i = 0; i < 6; i++)
    {
        l->addWidget(m_descs[i] = new QLabel(labels[i]), i, 0, Qt::AlignLeft);
        m_descs[i]->setObjectName("SmallText");
        l->addWidget(m_stats[i] = new QLabel(), i, 2, Qt::AlignLeft);
        m_stats[i]->setObjectName("SmallText");
        l->addWidget(m_sliders[i] = new QSlider(Qt::Horizontal), i, 3);
        l->addWidget(m_evs[i] = new QLineEdit("0"), i, 4, Qt::AlignLeft);

        if (!i==0){
            l->addWidget(natureButtons[i-1] = Theme::LRButton("equal"),i,1,Qt::AlignLeft);
            natureButtons[i-1]->setFixedWidth(20);
            natureButtons[i-1]->setFixedHeight(14);
            connect(natureButtons[i-1],SIGNAL(rightClick()),SLOT(checkNButtonR()));
            connect(natureButtons[i-1],SIGNAL(leftClick()),SLOT(checkNButtonL()));
        }
        slider(i)->setTracking(true);
        slider(i)->setRange(0,255);
        slider(i)->setMinimumWidth(150);
        m_evs[i]->setFixedWidth(35);
        connect(slider(i),SIGNAL(valueChanged(int)),SLOT(changeEV(int)));
        connect(m_evs[i], SIGNAL(textChanged(QString)), SLOT(changeEV(QString)));
    }

    l->addWidget(m_mainSlider = new QSlider(Qt::Horizontal), 6, 0, 1, 4);
    l->addWidget(m_mainLabel = new QLabel(), 6, 4);
    m_mainLabel->setObjectName("SmallText");
    m_mainSlider->setEnabled(false);
    m_mainSlider->setRange(0,510);

    changeGen(gen());

    /*Setting the vals */
    updateEVs();
}

void TB_EVManager::setPokemon(PokeTeam *_poke)
{
    m_poke = _poke;
    updateEVs();
}

void TB_EVManager::changeGen(int)
{
    if (gen() <= 1) {
        m_stats[SpDefense]->hide();
        m_sliders[SpDefense]->hide();
        m_evs[SpDefense]->hide();
        m_descs[SpDefense]->hide();
        natureButtons[SpDefense-1]->hide();
        m_descs[SpAttack]->setText(tr("Special:", "Stat"));
    } else {
        m_stats[SpDefense]->show();
        m_sliders[SpDefense]->show();
        m_evs[SpDefense]->show();
        m_descs[SpDefense]->show();
        natureButtons[SpDefense-1]->show();
        m_descs[SpAttack]->setText(tr("Special Attack:"));
    }
    if (gen() <= 2) {
        m_mainLabel->hide();
        m_mainSlider->hide();
    } else {
        m_mainLabel->show();
        m_mainSlider->show();
    }
}

PokeTeam * TB_EVManager::poke()
{
    return m_poke;
}

const PokeTeam * TB_EVManager::poke() const
{
    return m_poke;
}

QSlider * TB_EVManager::slider(int stat)
{
    return m_sliders[stat];
}

const QSlider * TB_EVManager::slider(int stat) const
{
    return m_sliders[stat];
}

QLineEdit * TB_EVManager::evLabel(int stat)
{
    return m_evs[stat];
}

const QLineEdit * TB_EVManager::evLabel(int stat) const
{
    return m_evs[stat];
}

QLabel * TB_EVManager::statLabel(int stat)
{
    return m_stats[stat];
}

/* the reverse of slider(int) */
int TB_EVManager::stat(QObject *sender) const
{
    for (int i = 0; i < 6; i++)
        if (sender == slider(i) || sender == evLabel(i))
            return i;
    throw QString("Fatal Error in TB_EVManager, alert the developers");
}

void TB_EVManager::updateEVs()
{
    if (!poke()) {
        return;
    }

    for (int i = 0; i < 6; i++)
        updateEV(i);

    updateMain();
}

void TB_EVManager::changeEV(const QString &newvalue)
{
    int mstat = stat(sender());

    poke()->setEV(mstat, std::max(std::min(newvalue.toInt(), 252), 0));

    slider(mstat)->blockSignals(true);
    updateEV(mstat);

    if (gen() == 2 && (mstat == SpAttack || mstat == SpDefense)) {
        updateEV(SpAttack+SpDefense-mstat);
    }

    updateMain();
    slider(mstat)->blockSignals(false);

    emit EVChanged(mstat);
}

void TB_EVManager::changeEV(int newvalue)
{
    int mstat = stat(sender());

    if (sender() == slider(mstat)) {
        newvalue = newvalue - (newvalue %4);

        if (newvalue == poke()->EV(mstat)) {
            return;
        }
    }
    poke()->setEV(mstat, newvalue);

    updateEV(mstat);
    updateMain();

    emit EVChanged(mstat);
}


void TB_EVManager::checkNButtonL()
{
    if (gen() <= 2)
        return;
    int loc = 0;
    for (int i = 0; i < 5; i++)
        if (sender() == natureButtons[i])
            loc = i;
    if(myStatDown == loc+1){
        int temp = myStatUp;
        myStatUp = loc+1;
        myStatDown = temp;
    }
    else{
        myStatUp = loc+1;
        if (myStatDown == -1) {
            myStatDown = myStatUp == 1 ? 2 : 1;
        }
    }
    if(myStatUp != -1 && myStatDown != -1)
        emit natureChanged(myStatUp,myStatDown);
}

void TB_EVManager::checkNButtonR()
{
    if (gen() <= 2)
        return;
    int loc = 0;
    for (int i = 0; i < 5; i++)
        if (sender() == natureButtons[i])
            loc = i;
    if(myStatUp == loc+1){
        int temp = myStatDown;
        myStatDown = loc+1;
        myStatUp = temp;
    }
    else{
        myStatDown = loc+1;
        if (myStatUp == -1) {
            myStatUp = myStatDown == 1 ? 2 : 1;
        }
    }
    if(myStatUp != -1 && myStatDown != -1)
        emit natureChanged(myStatUp,myStatDown);
}

void TB_EVManager::updateEV(int stat)
{
    slider(stat)->setValue(poke()->EV(stat));

    /* first the color : red if the stat is hindered by the nature, black if normal, blue if the stat is enhanced */
    QColor color;

    switch (poke()->natureBoost(stat)) {
    case -1: color = Theme::Color("Teambuilder/statHindered"); break;
    case 1: color = Theme::Color("Teambuilder/statRaised"); break;
    }

    evLabel(stat)->setText(QString::number(poke()->EV(stat)));
    statLabel(stat)->setText(toColor(QString::number(poke()->stat(stat)), color));
}

int TB_EVManager::gen() const
{
    if (poke()) {
        return poke()->gen().num;
    } else {
        return GEN_MAX;
    }
}

void TB_EVManager::updateMain()
{
    if (gen() <= 2)
        return;

    m_mainSlider->setValue(510 - poke()->EVSum());
    m_mainLabel->setText(QString::number(510 - poke()->EVSum()));
}

void TB_EVManager::updateNatureButtons()
{
    bool upC = false;
    bool downC = false;
    for (int i = 0; i < 5; i++){
        if(NatureInfo::Boost(poke()->nature(),i+1) == 1){
            upC = true;
            myStatUp = i+1;
        }
        else if(NatureInfo::Boost(poke()->nature(),i+1) == -1){
            downC = true;
            myStatDown = i+1;
        }
    }
    if(!upC&&!downC){
        myStatUp = -1;
        myStatDown = -1;
    }
    for (int j = 0; j<5;j++){
        if (j+1 == myStatUp)
            Theme::ChangePics(natureButtons[j], "plus");
        else if(j+1 == myStatDown)
            Theme::ChangePics(natureButtons[j], "minus");
        else
            Theme::ChangePics(natureButtons[j], "equal");
    }
}
