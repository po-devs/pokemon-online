#include "loadline.h"
#include "ui_loadline.h"

#include <PokemonInfo/pokemoninfo.h>

#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QCompleter>

LoadLine::LoadLine(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoadLine)
{
    ui->setupUi(this);
}

void LoadLine::setUi(QCheckBox *name, QComboBox *gen, QLineEdit *tier, QToolButton *browser, const QStringList &tierList)
{
    ui2.gen =gen;
    ui2.name = name;
    ui2.tier = tier;
    ui2.browse = browser;

    QLabel *pokes[6] = {ui->poke1, ui->poke2, ui->poke3, ui->poke4, ui->poke5, ui->poke6};
    memcpy(ui2.pokes, pokes, sizeof(pokes));

    for (int i = GenInfo::GenMin(); i <= GenInfo::GenMax(); i++) {
        for (int j = 0; j < GenInfo::NumberOfSubgens(i); j++) {
            Pokemon::gen g(i,j);

            ui2.gen->addItem(GenInfo::Version(g), QVariant::fromValue(g));
        }
    }

    QCompleter *m_completer = new QCompleter(tierList);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);

    ui2.tier->setCompleter(m_completer);

    connect(ui2.gen, SIGNAL(currentIndexChanged(int)), SLOT(genChanged()));
    connect(ui2.tier, SIGNAL(textEdited(QString)), SLOT(tierEdited(QString)));
    connect(m_completer, SIGNAL(activated(QString)), SLOT(tierEdited(QString)));
    connect(ui2.browse, SIGNAL(clicked()), SLOT(browseTeam()));
}

void LoadLine::activateCheck()
{
    setChecked(true);
    updateAll();
}

void LoadLine::setTeam(const Team &t)
{
    team = t;

    updateAll();
}

const Team &LoadLine::getTeam() const
{
    return team;
}

void LoadLine::browseTeam()
{
    loadTTeamDialog(team, this, SLOT(activateCheck()));
}

void LoadLine::genChanged()
{
    team.setGen(ui2.gen->itemData(ui2.gen->currentIndex()).value<Pokemon::gen>());
}

void LoadLine::tierEdited(const QString &l)
{
    team.defaultTier() = l;
}

void LoadLine::updateAll()
{
    ui2.tier->setText(team.defaultTier());

    for (int i = 0; i < ui2.gen->count(); i++) {
        Pokemon::gen g = ui2.gen->itemData(i).value<Pokemon::gen>();
        if (g == team.gen()) {
            ui2.gen->setCurrentIndex(i);
            break;
        }
    }

    for (int i = 0; i < 6; i++) {
        ui2.pokes[i]->setPixmap(PokemonInfo::Icon(team.poke(i).num()));
    }

    ui2.name->setText(team.name());
}

bool LoadLine::isChecked() const
{
    return ui2.name->isChecked();
}

void LoadLine::setChecked(bool checked)
{
    ui2.name->setChecked(checked);
}

LoadLine::~LoadLine()
{
    delete ui;
}
