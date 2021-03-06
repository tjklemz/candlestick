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
#include <sys/time.h>
#include "opengl.h"
#include "app.h"
#include "timesub.h"


@interface SysView : NSOpenGLView
{
}
@end

@interface SysDelegate : NSObject <NSApplicationDelegate>
{
}
+(void)populateMainMenu;
+(void)populateApplicationMenu:(NSMenu *)aMenu;
-(void)bringTextToFocus;
@end


static NSWindow *window;
static SysView *view;
static BOOL runLoop = FALSE;
static int isfullscreen = 0;

void fullscreen();
void updatetitle(char*);


@implementation SysView

- (void)loop
{
	struct timeval then;
	struct timeval now;
	struct timeval diff;
	double sleep_time = 0.0;
	
	gettimeofday(&then, NULL);

	while(runLoop) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

		App_OnUpdate();
		
		[view setNeedsDisplay:YES];
		
		then.tv_usec += SKIP_TICKS;
		gettimeofday(&now, NULL);
		
		if(!timeval_subtract(&diff, &then, &now)) {
			sleep_time = (diff.tv_sec / 10000.0) + diff.tv_usec;
			usleep(sleep_time);
		}

		[pool release];
	}
	
	[view setNeedsDisplay:YES];
}

static int num_anims = 0;

static void startLoop()
{
	if(num_anims++ == 0) {
		runLoop = TRUE;
		[NSThread detachNewThreadSelector:@selector(loop) toTarget:view withObject:nil];
	}
}


static void stopLoop()
{
	if(--num_anims <= 0) {
		runLoop = FALSE;
		num_anims = 0;
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
	
	App_AnimationDel(startLoop, stopLoop);
	App_FullscreenDel(fullscreen);
	App_UpdateTitleDel(updatetitle);
	App_OnInit();
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


#define UP_ARROW    126
#define DOWN_ARROW  125
#define RIGHT_ARROW 124
#define LEFT_ARROW  123
#define ESCAPE      53

- (void)keyDown:(NSEvent *)anEvent
{	
	cs_key_mod_t mods = CS_NONE;
	
	NSUInteger flags = [anEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
	
	if(flags & NSCommandKeyMask)    mods |= CS_SUPER;
	if(flags & NSControlKeyMask)    mods |= CS_CONTROL;
	if(flags & NSAlternateKeyMask)  mods |= CS_ALT;
	if(flags & NSShiftKeyMask)      mods |= CS_SHIFT;
	
	switch([anEvent keyCode]) {
	case LEFT_ARROW:     App_OnSpecialKeyDown(CS_ARROW_LEFT, mods);     break;
	case RIGHT_ARROW:    App_OnSpecialKeyDown(CS_ARROW_RIGHT, mods);    break; 
	case UP_ARROW:       App_OnSpecialKeyDown(CS_ARROW_UP, mods);       break;
	case DOWN_ARROW:     App_OnSpecialKeyDown(CS_ARROW_DOWN, mods);     break;
	case ESCAPE:         App_OnSpecialKeyDown(CS_ESCAPE, mods);         break;
	default:
		{
			char ch[255];
			const char * key = [[anEvent charactersIgnoringModifiers] UTF8String];
			strcpy(ch, key);
			if(ch[0] == '\r') {
				//NSLog(@"Converted CR to LF\n");
				ch[0] = '\n';
			} else if((int)ch[0] == -17) {
				//NSLog(@"Converted Mac delete to actual delete\n");
				ch[0] = 127;
			}
			App_OnKeyDown(ch, mods);
		}
	    break;
	}
	
	[self setNeedsDisplay:YES];
}


- (void)keyUp:(NSEvent *)anEvent
{
	cs_key_mod_t mods = CS_NONE;
	
	NSUInteger flags = [anEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask;
	
	if(flags & NSCommandKeyMask)    mods |= CS_SUPER;
	if(flags & NSControlKeyMask)    mods |= CS_CONTROL;
	if(flags & NSAlternateKeyMask)  mods |= CS_ALT;
	if(flags & NSShiftKeyMask)      mods |= CS_SHIFT;
	
	switch([anEvent keyCode]) {
	case LEFT_ARROW:     App_OnSpecialKeyUp(CS_ARROW_LEFT, mods);       break;
	case RIGHT_ARROW:    App_OnSpecialKeyUp(CS_ARROW_RIGHT, mods);      break;
	case UP_ARROW:       App_OnSpecialKeyUp(CS_ARROW_UP, mods);         break;
	case DOWN_ARROW:     App_OnSpecialKeyUp(CS_ARROW_DOWN, mods);       break;
	case CS_ESCAPE:      App_OnSpecialKeyUp(CS_ESCAPE, mods);           break;
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

	[NSApp setMainMenu:mainMenu];
}


+(void)populateApplicationMenu:(NSMenu *)aMenu
{
	NSString *applicationName = [NSString stringWithUTF8String:APP_NAME];
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
	[window setContentMinSize:NSMakeSize(WIN_INIT_WIDTH, WIN_INIT_HEIGHT / 2)];
	//[window setTitle: [NSString stringWithUTF8String:APP_NAME]]; //[[NSProcessInfo processInfo] processName]];
	[window setAcceptsMouseMovedEvents: YES];
	[window setDelegate: [NSApp delegate]];

	view = [[SysView alloc] initWithFrame: rect];
	[window setContentView: view];
	[view release];

	// [[NSApp dockTile] setShowsApplicationBadge: YES];
	// [[NSApp dockTile] display];
	
	// This line sets the new Lion fullscreen (that opens a new Space) and adds the icon in the top right
	//[window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

	[window makeKeyAndOrderFront:nil];
	// redraw the title bar
	[window display];
}


-(void)applicationWillTerminate:(NSNotification *)notification
{
	stopLoop();
	App_OnDestroy();
	[window release];
}


-(void)bringTextToFocus
{
	[window makeKeyAndOrderFront:nil];
	[window setInitialFirstResponder:window.contentView];
	[window makeFirstResponder:window.contentView];
}


- (void)windowWillClose:(NSNotification *)notification
{
	[NSApp stop:self];
}

@end


// - (void)fullscreen:(NSNotification *)notification
void fullscreen()
{
	if (isfullscreen) {
		NSRect frame;
		
		[view exitFullScreenModeWithOptions:nil];
		isfullscreen = 0;
		
		frame = InitialWindowSize();
	    //[window setFrame:frame display:NO animate:NO];
		[window setFrameOrigin:frame.origin];
		[window setContentSize:frame.size];
		
		[window makeKeyAndOrderFront:nil];
		[window setInitialFirstResponder:window.contentView];
		[window makeFirstResponder:window.contentView];
	} else {
		[view enterFullScreenMode:[window screen] withOptions:nil];
		isfullscreen = 1;
	}
}

void updatetitle(char * title)
{
	[window setTitle:[NSString stringWithUTF8String:title]];
}


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
	
	//[[NSApplication sharedApplication] setApplicationIconImage:[NSImage imageNamed:@"candlestick_logo.icns"]];
	
	[app run];
	[app setDelegate:NULL];
	[del release];
	[pool release];
	return 0;
}
