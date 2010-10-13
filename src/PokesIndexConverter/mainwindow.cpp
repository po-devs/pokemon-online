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

QHash<int, int> Associations;

inline QByteArray getFileContent(const QString &path) {
    QFile f(path);
    f.open(QIODevice::ReadOnly);

    return f.readAll();
}

void makeAssociations(QByteArray a1, QByteArray a2, QHash<int, int> & hash)
{
    QList<QByteArray> s1 = a1.split('\n');
    QList<QByteArray> s2 = a2.split('\n');

    for (int i = 0; i < s2.size(); i++) {
        int j;
        for (j = 0; j < s1.size(); j++) {
            if (s1[j].toLower() == s2[i].toLower()) {
                hash[i] = j;
                break;
            }
        }

        if (j == s1.size()) {
            QMessageBox::information(NULL, "Error", QString("%1 is not associated to anything (%2, %3, %4)").arg(QString::fromUtf8(s2[i])).arg(i)
                                     .arg(s2.size()).arg(s1.size()));
            exit(-1);
        }
    }
}

void reorderFile(const QString &name, const QHash<int, int> &order)
{
    QStringList content = QString::fromUtf8(getFileContent(name)).split('\n');
    QVector<QString> content2;
    content2.resize(content.size());
    QStringList content3 = content2.toList();

    for (int i = 0; i < content.size(); i++) {
        if (!order.contains(i)) {
            QMessageBox::information(NULL, "Error", QString("File %1 has extra line %2").arg(name).arg(i));
            exit(-2);
        }
        content3[order[i]] = content[i];
    }

    QFile out(name);
    out.open(QIODevice::WriteOnly);
    out.write(content3.join("\n").toUtf8());
}

void replaceDataInFile(const QString &name, const QHash<int, int> &data)
{
    QStringList content = QString::fromUtf8(getFileContent(name)).split('\n');
    QStringList ret;

    foreach(QString s, content) {
        QStringList spl = s.split(' ');

        for (int i = 1; i < spl.size(); i++)  {
            if (data.contains(spl[i].toInt())) {
                spl[i] = QString::number(data[spl[i].toInt()]);
            }
        }
        ret.push_back(spl.join(" "));
    }

    QFile out(name);
    out.open(QIODevice::WriteOnly);
    out.write(ret.join("\n").toUtf8());
}

void MainWindow::processFiles()
{
//    QStringList files = QFileDialog::getOpenFileNames(this, "Files to procecss", "db/pokes");
    QStringList files = QFileDialog::getOpenFileNames(this, "Files to procecss", "db/abilities/");

    QByteArray a1(getFileContent("db/abilities/oldabilities.txt")), a2(getFileContent("db/abilities/abilities.txt"));
    makeAssociations(a1, a2, Associations);

    foreach(QString file, files) {
        processFile(file);
    }

    QMessageBox::information(this, "Files processed", "The files were successfully processed.");
}

static QString makeLine(int index, int forme, const QString &content) {
    if (forme != -1)
        return QString("%1:%2 %3").arg(index).arg(forme).arg(content);
    else
        return QString("%1 %2").arg(index).arg(content);
}

void MainWindow::processFile(const QString &filename)
{
//    bool translationFile = filename.indexOf("pokemons") != -1;
//    bool descriptionFile = filename.indexOf("description") != -1 || filename.indexOf("classification") != -1 ;

//    QFile in(filename);
//    in.open(QIODevice::ReadOnly);

//    QString file = QString::fromUtf8(in.readAll());
//    QStringList lines = file.split('\n');

//    /* Pokemon naming files had a first line telling the number of pokemons without formes */
//    if (translationFile) {
//        lines.erase(lines.begin(), lines.begin() + 1);
//    }

//    QStringList output;

//    for (int i = 0; i < lines.size(); i++) {
//        if (i <= 493 || descriptionFile) {
//            output.push_back(makeLine(i, descriptionFile ? -1: 0, lines[i]));
//        } else {
//            int indexes[] = {
//                479,479,479,479,479,386,386,386,413,413,492,487
//            };
//            int formes[] = {
//                1,2,3,4,5,1,2,3,1,2,1,1
//            };
//            int relNum = i-494;
//            output.push_back(makeLine(indexes[relNum], formes[relNum], lines[i]));
//        }
//    }

//    in.close();
//    QFile out(filename);
//    out.open(QIODevice::WriteOnly);
//    out.write(output.join("\n").toUtf8());
    if (!filename.contains("/pokes/"))
        reorderFile(filename, Associations);
    else {
        replaceDataInFile(filename, Associations);
    }
}
