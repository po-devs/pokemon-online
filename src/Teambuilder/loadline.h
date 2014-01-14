#ifndef LOADLINE_H
#define LOADLINE_H

#include <QWidget>

#include <PokemonInfo/pokemonstructs.h>

namespace Ui {
class LoadLine;
}

class QCheckBox;
class QComboBox;
class QLineEdit;
class QToolButton;
class QLabel;

class LoadLine : public QWidget
{
    Q_OBJECT
    
public:
    explicit LoadLine(QWidget *parent = 0);
    void setUi(QCheckBox *name, QComboBox *gen, QLineEdit *tier, QToolButton *browser, const QStringList &tierList);
    ~LoadLine();

    bool isChecked() const;
    void setChecked(bool checked);
    void setTeam(const Team &t);

    const Team& getTeam() const;

public slots:
    void activateCheck();
    void genChanged();
    void tierEdited(const QString &);
    void browseTeam();
    void updateAll();
private:
    Ui::LoadLine *ui;

    struct Ui2 {
        QCheckBox *name;
        QComboBox *gen;
        QLabel *pokes[6];
        QToolButton *browse;
        QLineEdit *tier;
    };

    Ui2 ui2;

    Team team;
};

#endif // LOADLINE_H
