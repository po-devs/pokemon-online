#include <QtGui>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QPushButton *process = new QPushButton(tr("Convert Files..."));
    setCentralWidget(process);

    connect(process, SIGNAL(clicked()), SLOT(processFiles()));
}

MainWindow::~MainWindow()
{

}

void MainWindow::processFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Files to procecss", "db/pokes");

    foreach(QString file, files) {
        processFile(file);
    }

    QMessageBox::information(this, "Files processed", "The files were successfully processed.");
}

static QString makeLine(int index, int forme, const QString &content) {
    return QString("%1:%2 %3").arg(index).arg(forme).arg(content);
}

void MainWindow::processFile(const QString &filename)
{
    bool translationFile = filename.indexOf("pokemons") != -1;

    QFile in(filename);
    in.open(QIODevice::ReadOnly);

    QString file = QString::fromUtf8(in.readAll());
    QStringList lines = file.split('\n');

    /* Pokemon naming files had a first line telling the number of pokemons without formes */
    if (translationFile) {
        lines.erase(lines.begin(), lines.begin() + 1);
    }

    QStringList output;

    for (int i = 0; i < lines.size(); i++) {
        if (i <= 493) {
            output.push_back(makeLine(i, 0, lines[i]));
        } else {
            int indexes[] = {
                479,479,479,479,479,386,386,386,413,413,487,492
            };
            int formes[] = {
                1,2,3,4,5,1,2,3,1,2,1,1
            };
            int relNum = i-494;
            output.push_back(makeLine(indexes[relNum], formes[relNum], lines[i]));
        }
    }

    in.close();
    QFile out(filename);
    out.open(QIODevice::WriteOnly);
    out.write(output.join("\n").toUtf8());
}
