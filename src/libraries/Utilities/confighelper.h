#ifndef CONFIGHELPER_H
#define CONFIGHELPER_H

#include <QtCore>
#include <QtGui>
#ifdef QT5
#include <QtWidgets>
#endif

/* This file allows to avoid writing bothersome code in order to make GUI config windows.

   Just generate a ConfigForm and add the corresponding ConfigHelper, and the variables
   will be modified internally automatically when the user plays with the GUI.
*/

class AbstractConfigHelper;

class ConfigForm : public QObject
{
    Q_OBJECT
public:
    ConfigForm(const QString &button1, const QString &button2, QObject*parent=NULL);
    ~ConfigForm();

    QWidget* generateConfigWidget();
    void addConfigHelper(AbstractConfigHelper *helper);
public slots:
    void applyVals();
signals:
    void button1();
    void button2();
private:
    QList<AbstractConfigHelper *> helpers;

    QString button1S, button2S;
};

class AbstractConfigHelper
{
public:
    AbstractConfigHelper(const QString &desc = "");
    virtual ~AbstractConfigHelper(){}

    QWidget * generateConfigWidget();
    virtual void updateVal(){}
protected:
    QString description;
    QWidget* internalWidget;
private:
    virtual QWidget *getInternalWidget() {return NULL;}
};

template <class T>
class ConfigHelper : public AbstractConfigHelper {
public:
    ConfigHelper(const QString &desc, T &var);
protected:
    T &var;
};

/* Creates a combobox.
   You must also specify the values, of type T, that correspond
   to the different labels in the combobox */
template <class T>
class ConfigCombo : public ConfigHelper<T> {
public:
    ConfigCombo(const QString &desc, T &var, const QStringList &labels, const QList<T> &values);
    void updateVal();
private:
    QStringList labels;
    QList<T> values;

    virtual QWidget *getInternalWidget();
};

/* Creates a SpinBox */
class ConfigSpin : public ConfigHelper<int> {
public:
    ConfigSpin(const QString &desc, int &var, int min, int max);
    void updateVal();
private:
    int min, max;

    virtual QWidget *getInternalWidget();
};

/* Creates a line edit */
class ConfigLine : public ConfigHelper<QString> {
public:
    ConfigLine(const QString &desc, QString &var);
    void updateVal();
private:
    virtual QWidget *getInternalWidget();
};

/* Creates a text edit */
class ConfigText : public ConfigHelper<QString> {
public:
    ConfigText(const QString &desc, QString &var);
    void updateVal();
private:
    virtual QWidget *getInternalWidget();
};

/* Creates a QCheckBox */
class ConfigCheck : public ConfigHelper<bool> {
public:
    ConfigCheck(const QString &desc, bool &var);
    void updateVal();
private:
    QString checkBoxText;
    virtual QWidget *getInternalWidget();
};

/* Finds a file path */
class ConfigFile : public QObject, public ConfigHelper<QString> {
    Q_OBJECT
public:
    ConfigFile(const QString &desc, QString &path);
    void updateVal();
public slots:
    void findPath();
private:
    QString path;
    virtual QWidget *getInternalWidget();

    QLineEdit *edit;
};

/* Slider */
class ConfigSlider : public ConfigHelper<int> {
public:
    ConfigSlider(const QString &desc, int &var, int min=0, int max=100);
    void updateVal();
private:
    int min, max;
    virtual QWidget *getInternalWidget();
};

template<class T>
ConfigHelper<T>::ConfigHelper(const QString &desc, T &var) : AbstractConfigHelper(desc), var(var) {

}

template<class T>
ConfigCombo<T>::ConfigCombo(const QString &desc, T &var, const QStringList &labels, const QList<T> &values)
    : ConfigHelper<T>(desc, var), labels(labels), values(values)
{

}

template<class T>
QWidget *ConfigCombo<T>::getInternalWidget()
{
    QComboBox *ret = new QComboBox();

    ret->addItems(labels);

    for (int i = 0; i < values.size(); i++) {
        if (values[i] == ConfigHelper<T>::var) {
            ret->setCurrentIndex(i);
            break;
        }
    }

    return ret;
}

template<class T>
void ConfigCombo<T>::updateVal()
{
    int index = ((QComboBox*)(ConfigHelper<T>::internalWidget))->currentIndex();

    if (index >= 0 && index < values.size()) {
        ConfigHelper<T>::var = values[index];
    }
}


#endif // CONFIGHELPER_H
