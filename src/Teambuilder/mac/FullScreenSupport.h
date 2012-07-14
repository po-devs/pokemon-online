#ifndef MAC_FULLSCREENSUPPORT_H
#define MAC_FULLSCREENSUPPORT_H
#include <QWidget>

class MacSupport
{
public:
	static bool isLion();
	static void setupFullScreen(QWidget *window);
};

#endif // MAC_FULLSCREENSUPPORT_H
