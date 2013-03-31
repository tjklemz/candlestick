/*************************************************************************
 * main-mac.m -- Based on the file "nibless.m"; creates Cocoa window
 *               without a nib file.
 *
 * Candlestick App: Just Write. A minimalist, cross-platform writing app.
 * Copyright (C) 2013 Thomas Klemz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

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
+(void)populateFileMenu:(NSMenu *)aMenu;
-(void)bringTextToFocus;
@end

static int isfullscreen = 0;
static NSWindow *window;
static SysView *view;
static BOOL hasBeenSaved = NO;
static NSTimer * renderTimer;

@implementation SysView
 
// Timer callback method
- (void)timerFired:(id)sender
{
    // It is good practice in a Cocoa application to allow the system to send the -drawRect:
    // message when it needs to draw, and not to invoke it directly from the timer.
    // All we do here is tell the display it needs a refresh
    [self setNeedsDisplay:YES];
}

static void startTimer()
{
	if(renderTimer == nil) {
		renderTimer = [[NSTimer scheduledTimerWithTimeInterval:0.001   //in seconds
	                                                    target:view
	                                                  selector:@selector(timerFired:)
	                                                  userInfo:nil
	                                                   repeats:YES] retain];
	}
}

static void stopTimer()
{
	if(renderTimer != nil) {
		[renderTimer invalidate];
		renderTimer = nil;
	}
}

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
	App_AnimationDel(&startTimer, &stopTimer);
}

- (void)reshape
{
	int w = [[[self window] contentView] bounds].size.width;
	int h = [[[self window] contentView] bounds].size.height;
	
	App_OnResize(w, h);
}

- (void)drawRect:(NSRect)rect
{
	App_OnRender();
	
	//swaps the buffers (double buffering) and calls glFlush()
	[[self openGLContext] flushBuffer];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

-(void)saveAs:(NSNotification *)notification
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
	
	[self setNeedsDisplay:YES];
}

-(void)save:(NSNotification *)notification
{
	if(!hasBeenSaved) {
		[self saveAs:notification];
	} else {
		App_Save();
	}
	
	[self setNeedsDisplay:YES];
}

-(void)open:(NSNotification *)notification
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
		
		[[NSApp delegate] bringTextToFocus];

	    [panel release];
	}];
}

-(void)reload:(NSNotification *)notification
{
	if(!hasBeenSaved) {
		[self open:notification];
	} else {
		App_Reload();
	}
	
	[self setNeedsDisplay:YES];
}

#define UP_ARROW    126
#define DOWN_ARROW  125
#define RIGHT_ARROW 124
#define LEFT_ARROW  123

- (void)keyDown:(NSEvent *)anEvent
{	
	switch([anEvent keyCode]) {
	case UP_ARROW:
		App_OnSpecialKeyDown(CS_ARROW_UP);
	    break;
	case DOWN_ARROW:
		App_OnSpecialKeyDown(CS_ARROW_DOWN);
		break;
	default:
		{
			char character = [[anEvent charactersIgnoringModifiers] characterAtIndex:0];
			App_OnKeyDown(character);
		}
	    break;
	}
	
	[self setNeedsDisplay:YES];
}

- (void)keyUp:(NSEvent *)anEvent
{
	switch([anEvent keyCode]) {
	case UP_ARROW:
		App_OnSpecialKeyUp(CS_ARROW_UP);
	    break;
	case DOWN_ARROW:
		App_OnSpecialKeyUp(CS_ARROW_DOWN);
		break;
	default:
	    break;
	}
	
	[self setNeedsDisplay:YES];
}

@end

@implementation SysDelegate

+(void)populateMainMenu
{
	NSMenu *mainMenu = [[[NSMenu alloc] initWithTitle:@"MainMenu"] autorelease];
	NSMenuItem *menuItem;
	NSMenu *submenu;

	menuItem = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:@"Apple"] autorelease];
	[NSApp performSelector:@selector(setAppleMenu:) withObject:submenu];
	[self populateApplicationMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	menuItem = [mainMenu addItemWithTitle:@"File" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"File", @"The File menu")] autorelease];
	[self populateFileMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];

	menuItem = [mainMenu addItemWithTitle:@"Window" action:NULL keyEquivalent:@""];
	submenu = [[[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", @"The Window menu")] autorelease];
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

+(void)populateFileMenu:(NSMenu *)aMenu
{
	NSMenuItem *menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Reload", nil)
		action:@selector(reload:)
		keyEquivalent:@"r"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
	[menuItem setTarget:view];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Open...", nil)
		action:@selector(open:)
		keyEquivalent:@"o"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
	[menuItem setTarget:view];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Save", nil)
		action:@selector(save:)
		keyEquivalent:@"s"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
	[menuItem setTarget:view];

	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Save As...", nil)
		action:@selector(saveAs:)
		keyEquivalent:@"S"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
	[menuItem setTarget:view];
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
	[window setContentMinSize:NSMakeSize(WIN_INIT_WIDTH, 100)];
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

-(void)applicationWillTerminate:(NSNotification *)notification
{
	App_OnDestroy();
	[window release];
	stopTimer();
	
	NSLog(@"Terminating...");
}

-(void)bringTextToFocus
{
	[window makeKeyAndOrderFront:nil];
	[window setInitialFirstResponder:window.contentView];
	[window makeFirstResponder:window.contentView];
}

- (void)fullscreen:(NSNotification *)notification
{
	if (isfullscreen) {
		NSRect frame;
		
		[view exitFullScreenModeWithOptions:nil];
		isfullscreen = 0;
		
		frame = InitialWindowSize();
	    [window setFrame:frame display:NO animate:NO];
		
		[self bringTextToFocus];
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

	[app setDelegate:del];
	[app run];
	[app setDelegate:NULL];
	[del release];
	[pool release];
	return 0;
}
