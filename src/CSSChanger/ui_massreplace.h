/********************************************************************************
** Form generated from reading UI file 'massreplace.ui'
**
** Created: Sat 24. Dec 02:17:47 2011
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MASSREPLACE_H
#define UI_MASSREPLACE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_MassReplaceDialog
{
public:
    QVBoxLayout *verticalLayout;
    QPushButton *pushButton;
    QGroupBox *groupBox;
    QVBoxLayout *findList;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *MassReplaceDialog)
    {
        if (MassReplaceDialog->objectName().isEmpty())
            MassReplaceDialog->setObjectName(QString::fromUtf8("MassReplaceDialog"));
        MassReplaceDialog->resize(614, 399);
        verticalLayout = new QVBoxLayout(MassReplaceDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        pushButton = new QPushButton(MassReplaceDialog);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        verticalLayout->addWidget(pushButton);

        groupBox = new QGroupBox(MassReplaceDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        findList = new QVBoxLayout(groupBox);
        findList->setObjectName(QString::fromUtf8("findList"));

        verticalLayout->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(MassReplaceDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(MassReplaceDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), MassReplaceDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), MassReplaceDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(MassReplaceDialog);
    } // setupUi

    void retranslateUi(QDialog *MassReplaceDialog)
    {
        MassReplaceDialog->setWindowTitle(QApplication::translate("MassReplaceDialog", "Multiple color changer", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QString());
        groupBox->setTitle(QApplication::translate("MassReplaceDialog", "Different instances", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MassReplaceDialog: public Ui_MassReplaceDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MASSREPLACE_H
