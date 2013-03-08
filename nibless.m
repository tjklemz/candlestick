#import <Cocoa/Cocoa.h>
#include "opengl.h"
#include "app.h"

@interface SysView : NSOpenGLView
{
}
@end

@interface SysDelegate : NSObject <NSApplicationDelegate>
{
}
+(void)populateMainMenu;
+(void)populateApplicationMenu:(NSMenu *)aMenu;
+(void)populateWindowMenu:(NSMenu *)aMenu;
@end

static int isfullscreen = 0;
static NSWindow *window;
static SysView *view;

@implementation SysView

- (id)initWithFrame:(NSRect)frameRect
{
    NSOpenGLPixelFormatAttribute attr[] = 
	{
        NSOpenGLPFADoubleBuffer,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute) 32,
		NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute) 23,
		(NSOpenGLPixelFormatAttribute) 0
	};
	NSOpenGLPixelFormat *nsglFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attr] autorelease];
	
    self = [super initWithFrame:frameRect pixelFormat:nsglFormat];
	
	return self;
}

- (void)prepareOpenGL
{
	// Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	
	App_OnInit();
}

- (void)reshape
{
	int w = [[[self window] contentView] bounds].size.width;
	int h = [[[self window] contentView] bounds].size.height;
	
	App_OnResize(w, h);
	//[super setNeedsDisplay:YES];
	//[[self openGLContext] update];
	puts("reshape");
}

- (void)drawRect:(NSRect)rect
{
	//App_OnResize(rect.size.width, rect.size.height);
	App_OnRender();
	
	//swaps the buffers (double buffering) and calls glFlush()
	[[self openGLContext] flushBuffer];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)mouseDown:(NSEvent *)anEvent
{
	puts("mousedown!");
}

- (void)mouseDragged:(NSEvent *)anEvent
{
	puts("mousemoved!");
}

- (void)keyDown:(NSEvent *)anEvent
{
	NSString * str = [anEvent characters];
	const char * chars = [str UTF8String];
    unsigned char character = chars[0];
	
	switch(character) {
		//esc
		case 27:
			puts("ESCAPE!");
			break;
		default:
		NSLog(@"%d", character);
			App_OnKeyDown(character);
			break;
	}
	
	[self setNeedsDisplay:YES];
}

@end

@implementation SysDelegate

+(void)populateMainMenu
{
	NSMenu *mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
	NSMenuItem *menuItem;
	NSMenu *submenu;

	menuItem = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:@"Apple"];
	[NSApp performSelector:@selector(setAppleMenu:) withObject:submenu];
	[self populateApplicationMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];

	menuItem = [mainMenu addItemWithTitle:@"Window" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", @"The Window menu")];
	[self populateWindowMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	[NSApp setWindowsMenu:submenu];

	[NSApp setMainMenu:mainMenu];
}

+(void)populateApplicationMenu:(NSMenu *)aMenu
{
	NSString *applicationName = [[NSProcessInfo processInfo] processName];
	NSMenuItem *menuItem;

	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", nil), applicationName]
		action:@selector(orderFrontStandardAboutPanel:)
		keyEquivalent:@""];
	[menuItem setTarget:NSApp];

	[aMenu addItem:[NSMenuItem separatorItem]];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Preferences...", nil)
		action:NULL
		keyEquivalent:@","];

	[aMenu addItem:[NSMenuItem separatorItem]];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Services", nil)
		action:NULL
		keyEquivalent:@""];
	NSMenu * servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
	[aMenu setSubmenu:servicesMenu forItem:menuItem];
	[NSApp setServicesMenu:servicesMenu];

	[aMenu addItem:[NSMenuItem separatorItem]];

	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", nil), applicationName]
		action:@selector(hide:)
		keyEquivalent:@"h"];
	[menuItem setTarget:NSApp];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)
		action:@selector(hideOtherApplications:)
		keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	[menuItem setTarget:NSApp];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)
		action:@selector(hideOtherApplications:)
		keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	[menuItem setTarget:NSApp];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Show All", nil)
		action:@selector(unhideAllApplications:)
		keyEquivalent:@""];
	[menuItem setTarget:NSApp];

	[aMenu addItem:[NSMenuItem separatorItem]];

	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), applicationName]
		action:@selector(terminate:)
		keyEquivalent:@"q"];
	[menuItem setTarget:NSApp];
}

+(void)populateWindowMenu:(NSMenu *)aMenu
{
	NSMenuItem *menuItem;

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Minimize", nil)
		action:@selector(miniaturize:)
		keyEquivalent:@"m"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Zoom", nil)
		action:@selector(zoom:)
		keyEquivalent:@""];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Fullscreen", nil)
		action:@selector(fullscreen:)
		keyEquivalent:@"f"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
	[menuItem setTarget:[NSApp delegate]];

	[aMenu addItem:[NSMenuItem separatorItem]];
}

- (void) applicationWillFinishLaunching: (NSNotification *)notification
{
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	CGRect cgr;
	NSRect rect;

	[SysDelegate populateMainMenu];

	cgr = CGDisplayBounds(CGMainDisplayID());
	rect = NSMakeRect(50, 100, 640, 480);

	window = [[NSWindow alloc] initWithContentRect: rect
		styleMask: NSClosableWindowMask | NSTitledWindowMask |
			NSMiniaturizableWindowMask | NSResizableWindowMask
		backing: NSBackingStoreBuffered
		defer: YES
		screen: [NSScreen mainScreen]];
	[window setTitle: [[NSProcessInfo processInfo] processName]];
	[window setAcceptsMouseMovedEvents: YES];
	[window setDelegate: [NSApp delegate]];

	view = [[SysView alloc] initWithFrame: rect];
	[window setContentView: view];
	[view release];

	// event_resize
	// seticon

	// [[NSApp dockTile] setShowsApplicationBadge: YES];
	// [[NSApp dockTile] display];

	[window makeKeyAndOrderFront: NULL];
}

- (void)fullscreen:(NSNotification *)notification
{
	if (isfullscreen) {
		[view exitFullScreenModeWithOptions:nil];
		isfullscreen = 0;
	} else {
		[view enterFullScreenMode:[window screen] withOptions:nil];
		isfullscreen = 1;
	}
}

- (void)windowWillClose:(NSNotification *)notification
{
	puts("closed!");
	[NSApp stop:self];
}

- (void)windowDidResize:(NSNotification *)notification
{
	puts("resized!");
}

@end

int main(int argc, char **argv)
{
	NSAutoreleasePool *pool;
	NSApplication *app;
	SysDelegate *del;

	pool = [[NSAutoreleasePool alloc] init];
	app = [NSApplication sharedApplication];
	del = [[SysDelegate alloc] init];

	/* carbon voodoo to get icon and menu without bundle */
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);
	SetFrontProcess(&psn);

	[app setDelegate: del];
	[app run];
	[app setDelegate: NULL];
	[del release];
	[pool release];
	return 0;
}
