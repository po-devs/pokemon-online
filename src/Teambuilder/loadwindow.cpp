#include "loadwindow.h"
#include "ui_loadwindow.h"

LoadWindow::LoadWindow(QWidget *parent, const QStringList &tierList) :
    QDialog(parent),
    ui(new Ui::LoadWindow)
{
    ui->setupUi(this);

    holder.load();

    LoadLine *array[] = {ui->line1, ui->line2, ui->line3, ui->line4, ui->line5, ui->line6};
    memcpy(lines, array, sizeof(array));

    lines[0]->setUi(ui->name1, ui->gen1, ui->tier1, ui->browser1, tierList);
    lines[1]->setUi(ui->name2, ui->gen2, ui->tier2, ui->browser2, tierList);
    lines[2]->setUi(ui->name3, ui->gen3, ui->tier3, ui->browser3, tierList);
    lines[3]->setUi(ui->name4, ui->gen4, ui->tier4, ui->browser4, tierList);
    lines[4]->setUi(ui->name5, ui->gen5, ui->tier5, ui->browser5, tierList);
    lines[5]->setUi(ui->name6, ui->gen6, ui->tier6, ui->browser6, tierList);

    for (int i = 0; i < 6; i++) {
        if (i >= holder.count()) {
            lines[i]->setChecked(false);
        } else {
            lines[i]->setTeam(holder.team(i));
            lines[i]->setChecked(true);
        }
    }

    connect(this, SIGNAL(accepted()), SLOT(onAccept()));
}

void LoadWindow::onAccept()
{
    while (holder.count() < 6) {
        holder.addTeam();
    }

    for (int i = 5; i >= 0; i--) {
        holder.setCurrent(i);
        if (lines[i]->isChecked()) {
            holder.team() = lines[i]->getTeam();
        } else {
            holder.removeTeam();
        }
    }

    holder.save();

    emit teamLoaded(holder);
}

LoadWindow::~LoadWindow()
{
    delete ui;
}
