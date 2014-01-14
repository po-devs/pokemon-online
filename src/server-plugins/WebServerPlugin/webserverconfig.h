#ifndef WEBSERVERCONFIG_H
#define WEBSERVERCONFIG_H

#include <QDialog>

namespace Ui {
class WebServerConfig;
}

class WebServerPlugin;

class WebServerConfig : public QDialog
{
    Q_OBJECT
    
public:
    explicit WebServerConfig(WebServerPlugin *parent = 0);
    ~WebServerConfig();
    
    void accept();
private:
    Ui::WebServerConfig *ui;

    WebServerPlugin *master;
};

#endif // WEBSERVERCONFIG_H
