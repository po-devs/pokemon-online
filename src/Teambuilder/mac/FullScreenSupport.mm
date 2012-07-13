#include "fullscreensupport.h"
#include <QDebug>

bool MacSupport::isLion()
{
    NSString *string = [NSString string];

    return [string respondsToSelector:@selector(linguisticTagsInRange:scheme:options:orthography:tokenRanges:)];
}

void MacSupport::setupFullScreen(QWidget *window)
{
    if (isLion()) {
        NSView *view = (NSView*) window->winId();
        NSWindow *window = [view window];
        [window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    }
}
