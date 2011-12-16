/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Dec 7 18:06:57 2011
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QRadioButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionCtrl_S;
    QAction *save;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QLineEdit *pokemonName;
    QListWidget *moveList;
    QListWidget *pokemonList;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QRadioButton *gen1;
    QRadioButton *gen2;
    QRadioButton *gen3;
    QRadioButton *gen4;
    QRadioButton *gen5;
    QTabWidget *pokeMoves;
    QWidget *tab;
    QListWidget *levelMoves;
    QWidget *tab_2;
    QListWidget *tutorMoves;
    QWidget *tab_3;
    QListWidget *eggMoves;
    QWidget *tab_4;
    QListWidget *specialMoves;
    QWidget *tab_5;
    QListWidget *tmMoves;
    QWidget *tab_7;
    QListWidget *preMoves;
    QWidget *tab_6;
    QListWidget *dwMoves;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(746, 733);
        actionCtrl_S = new QAction(MainWindow);
        actionCtrl_S->setObjectName(QString::fromUtf8("actionCtrl_S"));
        save = new QAction(MainWindow);
        save->setObjectName(QString::fromUtf8("save"));
        save->setMenuRole(QAction::TextHeuristicRole);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        pokemonName = new QLineEdit(centralWidget);
        pokemonName->setObjectName(QString::fromUtf8("pokemonName"));

        gridLayout->addWidget(pokemonName, 0, 0, 1, 1);

        moveList = new QListWidget(centralWidget);
        moveList->setObjectName(QString::fromUtf8("moveList"));

        gridLayout->addWidget(moveList, 0, 1, 4, 1);

        pokemonList = new QListWidget(centralWidget);
        pokemonList->setObjectName(QString::fromUtf8("pokemonList"));

        gridLayout->addWidget(pokemonList, 1, 0, 1, 1);

        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        gen1 = new QRadioButton(groupBox);
        gen1->setObjectName(QString::fromUtf8("gen1"));

        horizontalLayout->addWidget(gen1);

        gen2 = new QRadioButton(groupBox);
        gen2->setObjectName(QString::fromUtf8("gen2"));

        horizontalLayout->addWidget(gen2);

        gen3 = new QRadioButton(groupBox);
        gen3->setObjectName(QString::fromUtf8("gen3"));

        horizontalLayout->addWidget(gen3);

        gen4 = new QRadioButton(groupBox);
        gen4->setObjectName(QString::fromUtf8("gen4"));

        horizontalLayout->addWidget(gen4);

        gen5 = new QRadioButton(groupBox);
        gen5->setObjectName(QString::fromUtf8("gen5"));

        horizontalLayout->addWidget(gen5);


        gridLayout->addWidget(groupBox, 2, 0, 1, 1);

        pokeMoves = new QTabWidget(centralWidget);
        pokeMoves->setObjectName(QString::fromUtf8("pokeMoves"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        levelMoves = new QListWidget(tab);
        levelMoves->setObjectName(QString::fromUtf8("levelMoves"));
        levelMoves->setGeometry(QRect(0, 1, 371, 321));
        pokeMoves->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        tutorMoves = new QListWidget(tab_2);
        tutorMoves->setObjectName(QString::fromUtf8("tutorMoves"));
        tutorMoves->setGeometry(QRect(0, 0, 371, 321));
        pokeMoves->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        eggMoves = new QListWidget(tab_3);
        eggMoves->setObjectName(QString::fromUtf8("eggMoves"));
        eggMoves->setGeometry(QRect(0, 0, 371, 321));
        pokeMoves->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QString::fromUtf8("tab_4"));
        specialMoves = new QListWidget(tab_4);
        specialMoves->setObjectName(QString::fromUtf8("specialMoves"));
        specialMoves->setGeometry(QRect(0, 0, 371, 321));
        pokeMoves->addTab(tab_4, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QString::fromUtf8("tab_5"));
        tmMoves = new QListWidget(tab_5);
        tmMoves->setObjectName(QString::fromUtf8("tmMoves"));
        tmMoves->setGeometry(QRect(0, 0, 371, 321));
        pokeMoves->addTab(tab_5, QString());
        tab_7 = new QWidget();
        tab_7->setObjectName(QString::fromUtf8("tab_7"));
        preMoves = new QListWidget(tab_7);
        preMoves->setObjectName(QString::fromUtf8("preMoves"));
        preMoves->setGeometry(QRect(0, 0, 371, 321));
        pokeMoves->addTab(tab_7, QString());
        tab_6 = new QWidget();
        tab_6->setObjectName(QString::fromUtf8("tab_6"));
        dwMoves = new QListWidget(tab_6);
        dwMoves->setObjectName(QString::fromUtf8("dwMoves"));
        dwMoves->setGeometry(QRect(0, 0, 371, 321));
        pokeMoves->addTab(tab_6, QString());

        gridLayout->addWidget(pokeMoves, 3, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 746, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(save);

        retranslateUi(MainWindow);

        pokeMoves->setCurrentIndex(5);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionCtrl_S->setText(QApplication::translate("MainWindow", "Ctrl + S", 0, QApplication::UnicodeUTF8));
        save->setText(QApplication::translate("MainWindow", "Save", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_WHATSTHIS
        save->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        groupBox->setTitle(QApplication::translate("MainWindow", "GroupBox", 0, QApplication::UnicodeUTF8));
        gen1->setText(QApplication::translate("MainWindow", "Gen 1", 0, QApplication::UnicodeUTF8));
        gen2->setText(QApplication::translate("MainWindow", "Gen 2", 0, QApplication::UnicodeUTF8));
        gen3->setText(QApplication::translate("MainWindow", "Gen 3", 0, QApplication::UnicodeUTF8));
        gen4->setText(QApplication::translate("MainWindow", "Gen 4", 0, QApplication::UnicodeUTF8));
        gen5->setText(QApplication::translate("MainWindow", "Gen 5", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab), QApplication::translate("MainWindow", "Level Moves", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab_2), QApplication::translate("MainWindow", "Tutor Moves", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab_3), QApplication::translate("MainWindow", "Egg Moves", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab_4), QApplication::translate("MainWindow", "Special", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab_5), QApplication::translate("MainWindow", "TM && HM", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab_7), QApplication::translate("MainWindow", "Pre Evo", 0, QApplication::UnicodeUTF8));
        pokeMoves->setTabText(pokeMoves->indexOf(tab_6), QApplication::translate("MainWindow", "Dream World", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
