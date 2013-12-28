#include "confighelper.h"
#include "../Utilities/otherwidgets.h"

/* I hate stupid GCC that forces to specify the base class
   of attributes when templates are involved, even though
   it can check if there's a problem or not and shut up
   when there's not.

   What the hell is undeclared when templates are compile-time?
   Do they know what lazy evaluation is?

   You can always bypass that using -fpermissive, but it's going to
   be deprecated -.-

   Visual C++ > GCC on this point */

ConfigForm::ConfigForm(const QString &button1, const QString &button2, QObject *parent)
    : QObject(parent), button1S(button1), button2S(button2)
{

}

ConfigForm::~ConfigForm()
{
    foreach(AbstractConfigHelper *help, helpers) {
        delete help;
    }

    helpers.clear();
}

void ConfigForm::addConfigHelper(AbstractConfigHelper *helper)
{
    helpers.push_back(helper);
}

QWidget* ConfigForm::generateConfigWidget()
{
    QWidget *ret = new QWidget();

    QVBoxLayout *v = new QVBoxLayout(ret);
    v->setMargin(0);

    foreach(AbstractConfigHelper *helper, helpers) {
        v->addWidget(helper->generateConfigWidget());
    }

    if (button1S.isEmpty() && button2S.isEmpty())
        return ret;
    if (button1S.isEmpty()) {
        QPushButton *p = new QPushButton(button2S);
        v->addWidget(p);

        connect(p, SIGNAL(clicked()), SIGNAL(button2()));
        return ret;
    }
    if (button2S.isEmpty()) {
        QPushButton *p = new QPushButton(button1S);
        v->addWidget(p);

        connect(p, SIGNAL(clicked()), SIGNAL(button1()));
        return ret;
    }
    QPushButton *p1 = new QPushButton(button1S);
    QPushButton *p2 = new QPushButton(button2S);

    connect(p1, SIGNAL(clicked()), SIGNAL(button1()));
    connect(p2, SIGNAL(clicked()), SIGNAL(button2()));

    v->addLayout(new QSideBySide(p1, p2));

    return ret;
}

void ConfigForm::applyVals()
{
    foreach(AbstractConfigHelper *helper, helpers) {
        helper->updateVal();
    }
}

AbstractConfigHelper::AbstractConfigHelper(const QString &desc) : description(desc) {

}

QWidget *AbstractConfigHelper::generateConfigWidget() {
    internalWidget = getInternalWidget();
    if (description.isEmpty()) {
        return internalWidget;
    } else if (!internalWidget) {
        QLabel *ret = new QLabel(description);
        ret->setOpenExternalLinks(true);
        ret->setWordWrap(true);
        return ret;
    } else {
        QWidget *w = new QWidget();
        w->setLayout(new QSideBySide(new QLabel(description), internalWidget));
        w->layout()->setMargin(5);
        return w;
    }
}

ConfigSpin::ConfigSpin(const QString &desc, int &var, int min, int max)
    : ConfigHelper<int> (desc, var), min(min), max(max)
{

}

QWidget * ConfigSpin::getInternalWidget()
{
    QSpinBox *ret = new QSpinBox();

    ret->setRange(min, max);
    ret->setValue(var);

    return ret;
}

void ConfigSpin::updateVal()
{
    var = ((QSpinBox*)(internalWidget))->value();
}

ConfigLine::ConfigLine(const QString &desc, QString &var)
    : ConfigHelper<QString> (desc, var)
{

}

void ConfigLine::updateVal()
{
    var = ((QLineEdit*)(internalWidget))->text();
}

QWidget *ConfigLine::getInternalWidget()
{
    QLineEdit *ret = new QLineEdit();
    ret->setText(var);

    return ret;
}

ConfigText::ConfigText(const QString &desc, QString &var)
    : ConfigHelper<QString> (desc, var)
{

}

void ConfigText::updateVal()
{
    var = ((QPlainTextEdit*)(internalWidget))->toPlainText();
}

QWidget *ConfigText::getInternalWidget()
{
    QPlainTextEdit *ret = new QPlainTextEdit();
    ret->setPlainText(var);

    return ret;
}

ConfigCheck::ConfigCheck(const QString &desc, bool &var)
    : ConfigHelper<bool> ("", var), checkBoxText(desc)
{

}

void ConfigCheck::updateVal()
{
    var = ((QCheckBox*)(internalWidget))->isChecked();
}

QWidget *ConfigCheck::getInternalWidget()
{
    QCheckBox *ret = new QCheckBox(checkBoxText);
    ret->setChecked(var);

    return ret;
}

ConfigFile::ConfigFile(const QString &desc, QString &path)
                       : ConfigHelper<QString> (desc, path)
{
    this->path = path;
}

QWidget *ConfigFile::getInternalWidget()
{
    QWidget *w = new QWidget();
    QToolButton *dots;

    w->setLayout(new QSideBySide(edit=new QLineEdit(path), dots=new QToolButton()));
    dots->setText("...");

    w->layout()->setSpacing(1);

    connect(dots, SIGNAL(clicked()), SLOT(findPath()));

    return w;
}

void ConfigFile::findPath()
{
    QString dir = QFileDialog::getExistingDirectory(internalWidget, tr("Find Directory"), path);

    if (dir != "") {
        this->path = dir + "/";
        edit->setText(path);
    }
}

void ConfigFile::updateVal()
{
    QDir d;
    if (d.exists(edit->text()) && edit->text().length() > 0) {
        var = edit->text();
    }
}

ConfigSlider::ConfigSlider(const QString &desc, int &var, int min, int max)
    : ConfigHelper<int>(desc, var), min(min), max(max)
{

}

void ConfigSlider::updateVal()
{
    var = ((QSlider*)(internalWidget))->value();
}

QWidget * ConfigSlider::getInternalWidget()
{
    QSlider *ret = new QSlider();
    ret->setOrientation(Qt::Horizontal);
    ret->setRange(min, max);
    ret->setTickInterval((max-min)/2);
    ret->setTickPosition(QSlider::TicksAbove);
    ret->setValue(var);

    return ret;
}
