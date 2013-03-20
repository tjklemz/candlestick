#import <Cocoa/Cocoa.h>
#include "opengl.h"
#include "app.h"

/*@interface MyWindow : NSWindow
{
}
@end*/

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
static BOOL hasBeenSaved = NO;

/*@implementation MyWindow

-(BOOL)

@end*/

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
	//puts("reshape");
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
	//puts("mousedown!");
}

- (void)mouseDragged:(NSEvent *)anEvent
{
	//puts("mousemoved!");
}

-(void)saveAs
{
	NSSavePanel *panel = [[NSSavePanel savePanel] retain];
	[panel setLevel:CGShieldingWindowLevel()];
	
	[panel setAllowedFileTypes:[NSArray arrayWithObjects:@"txt", nil]];
	[panel setCanSelectHiddenExtension:YES];
	[panel setAllowsOtherFileTypes:YES];
	[panel setDirectoryURL:[NSURL fileURLWithPath:[@"~/Documents" stringByExpandingTildeInPath]]];
	
	[panel beginWithCompletionHandler:^(NSInteger returnCode) {
		if(returnCode == NSFileHandlingPanelOKButton) {
			const char * filename;
			[panel orderOut:self];
			filename = [[[panel URL] path] UTF8String];
			NSLog(@"Got URL: %s", filename);
			App_SaveAs(filename);
			hasBeenSaved = YES;
		}
		
		[panel release];
	}];
}

-(void)openFile
{
	NSOpenPanel *panel = [[NSOpenPanel openPanel] retain];
	[panel setLevel:CGShieldingWindowLevel()];

	// Configure your panel the way you want it
	[panel setCanChooseFiles:YES];
	[panel setCanChooseDirectories:NO];
	[panel setAllowsMultipleSelection:NO];
	[panel setAllowedFileTypes:[NSArray arrayWithObject:@"txt"]];
	[panel setAllowsOtherFileTypes:YES];
	[panel setDirectoryURL:[NSURL fileURLWithPath:[@"~/Documents" stringByExpandingTildeInPath]]];

	[panel beginWithCompletionHandler:^(NSInteger result){
	    if (result == NSFileHandlingPanelOKButton) {
			const char * filename;
			[panel orderOut:self];
			filename = [[[panel URL] path] UTF8String];
			NSLog(@"Got URL: %s", filename);
			App_Open(filename);
			[self setNeedsDisplay:YES];
			hasBeenSaved = YES;
	    }

	    [panel release];
	}];
}

- (void)keyDown:(NSEvent *)anEvent
{
	//unsigned char character = [[anEvent characters] UTF8String][0];
	unsigned char character = [[anEvent charactersIgnoringModifiers] characterAtIndex:0];
	//unsigned char character = [anEvent keyCode];
	
	if(character == 27) {
		if(!hasBeenSaved) {
			[self saveAs];
		} else {
			App_Save();
		}
	} else if(character == '`') {
		[self openFile];
	} else {
		//NSLog(@"Char: %d", character);
		App_OnKeyDown(character);
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

NSRect
InitialWindowSize()
{
	NSRect rect;
	
	NSScreen * mainScreen = [NSScreen mainScreen];
	NSRect mainScreenRect = [mainScreen frame];
	int x = (mainScreenRect.size.width - WIN_INIT_WIDTH) / 2;
	int y = (mainScreenRect.size.height - WIN_INIT_HEIGHT) / 2;
	
	//should make a call here to a function that returns the sizes (so cross-platform)
	rect = NSMakeRect(x, y, WIN_INIT_WIDTH, WIN_INIT_HEIGHT);
	
	return rect;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
	CGRect cgr;
	NSRect rect;

	[SysDelegate populateMainMenu];

	cgr = CGDisplayBounds(CGMainDisplayID());
	
	rect = InitialWindowSize();

	window = [[NSWindow alloc] initWithContentRect: rect
		styleMask: NSClosableWindowMask | NSTitledWindowMask |
			NSMiniaturizableWindowMask | NSResizableWindowMask
		backing: NSBackingStoreBuffered
		defer: YES
		screen: [NSScreen mainScreen]];
	[window setContentMinSize:NSMakeSize(WIN_INIT_WIDTH, WIN_INIT_HEIGHT)];
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
	
	// This line sets the new Lion fullscreen (that opens a new Space) and adds the icon in the top right
	//[window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

	[window makeKeyAndOrderFront:nil];
}

- (void)fullscreen:(NSNotification *)notification
{
	if (isfullscreen) {
		NSRect frame;
		
		[view exitFullScreenModeWithOptions:nil];
		isfullscreen = 0;
		
		frame = InitialWindowSize();
	    [window setFrame:frame display:NO animate:NO];
		
		[window makeKeyAndOrderFront:nil];
		[window setInitialFirstResponder:window.contentView];
		[window makeFirstResponder:window.contentView];
	} else {
		[view enterFullScreenMode:[window screen] withOptions:nil];
		isfullscreen = 1;
	}
}

- (void)windowWillClose:(NSNotification *)notification
{
	//puts("closed!");
	[NSApp stop:self];
}

- (void)windowDidResize:(NSNotification *)notification
{
	//puts("resized!");
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
