#include "fullscreensupport.h"
#include <Foundation/NSString.h>
#include <Appkit/NSView.h>
#include <Appkit/NSWindow.h>

// Allow the code to be compiled on older macs and still allow
// Fullscreen on Lion
// TODO: will this work on Mountain Lion too?
#if !defined(MAC_OS_X_VERSION_10_7) || \
    MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7
enum {
    NSWindowCollectionBehaviorFullScreenPrimary = (1 << 7),
    NSWindowCollectionBehaviorFullScreenAuxiliary = (1 << 8)
};
#endif

void MacSupport::setupFullScreen(QWidget *widget)
{
    NSView *view = (NSView*) widget->winId();
    NSWindow *window = [view window];
    NSUInteger collectionBehavior = [window collectionBehavior];
    collectionBehavior |= NSWindowCollectionBehaviorFullScreenPrimary;
    [window setCollectionBehavior:collectionBehavior];
}
