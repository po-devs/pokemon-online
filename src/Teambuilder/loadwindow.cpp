#include "loadwindow.h"
#include "ui_loadwindow.h"

LoadWindow::LoadWindow(QWidget *parent, const QStringList &tierList, const QString &name) :
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

    /* Hide profile & color from TeamBuilder, since there's a few bugs and you can edit it all in TeamBuilder anyway... */
    if (name == "") {
        ui->profileList->hide();
        ui->colorButton->hide();
    }

    /* To reduce excessive code, this will make the color the same color as your current profile by default */
    on_profileList_currentIndexChanged(name);

    QSettings s;

    QStringList profiles = holder.profile().getProfileList(s.value("Profile/Path").toString());
    QComboBox *profileList = ui->profileList;
    profileList->blockSignals(true);
    profileList->addItems(profiles);
    profileList->blockSignals(false);
    for (int i = 0; i < profileList->count(); ++i) {
        if (profileList->itemText(i) == name)
            profileList->setCurrentIndex(i);
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

    QSettings s;

    QString profileSelected = ui->profileList->currentText();
    QString path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(profileSelected) + ".xml";
    s.setValue("Profile/Current", path);
    holder.save();

    emit teamLoaded(holder);
}

LoadWindow::~LoadWindow()
{
    delete ui;
}

void LoadWindow::on_colorButton_clicked()
{
    QColor c = QColorDialog::getColor(holder.profile().color());

    if (c.isValid() && (c.lightness() > 140 || c.green() > 180)) {
        return;
    }

    holder.profile().color() = c;
    setColor(c);
}

void LoadWindow::setColor(QColor c) {
    if (c.isValid()) {
        ui->colorButton->setStyleSheet(QString("background: %1; color: white").arg(c.name()));
    } else {
        ui->colorButton->setStyleSheet("");
    }
}

void LoadWindow::on_profileList_currentIndexChanged(const QString &arg1)
{
    QSettings s;
    /* When we change profiles, we'll change color too */
    QString path = s.value("Profile/Path").toString() + "/" + QUrl::toPercentEncoding(arg1) + ".xml";
    holder.profile().loadFromFile(path);
    QColor newColor = holder.profile().color();
    if (newColor.isValid())
    setColor(newColor);
}
