#include "QtGui/QApplication"
#include "otherwidgets.h"


int main(int argc, char *argv[])
{
        QApplication a(argc, argv);
        QIRCLineEdit *button = new QIRCLineEdit();
        QObject::connect(button, SIGNAL(clicked()), &a, SLOT(quit()));
        button->show();
        return a.exec();
}
