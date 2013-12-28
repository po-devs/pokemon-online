#ifndef POKEMONEDITORDIALOG_H
#define POKEMONEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class PokemonEditorDialog;
}

class PokemonEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PokemonEditorDialog(QWidget *parent = 0);
    ~PokemonEditorDialog();

private:
    Ui::PokemonEditorDialog *ui;
};

#endif // POKEMONEDITORDIALOG_H
