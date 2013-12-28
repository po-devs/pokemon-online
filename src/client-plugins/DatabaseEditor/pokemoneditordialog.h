#ifndef POKEMONEDITORDIALOG_H
#define POKEMONEDITORDIALOG_H

#include <QDialog>

class QAbstractItemModel;

namespace Ui {
class PokemonEditorDialog;
}

class PokemonEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PokemonEditorDialog(QWidget *parent = 0);
    ~PokemonEditorDialog();

private slots:
    void on_pokemonFrame_clicked();
private:
    Ui::PokemonEditorDialog *ui;
    QAbstractItemModel *pokeModel;
};

#endif // POKEMONEDITORDIALOG_H
