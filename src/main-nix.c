/*************************************************************************
 * main-nix.c -- Uses X11 to create a window and get the ball rolling.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include "opengl.h"
#include "app.h"
#include "utf.h"
#include "keysym2ucs.h"

#include <X11/Xatom.h>

#define FPS 25
static const int SKIP_TICKS = (10000 / FPS);

static Display * dpy;
static Window root;
static GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
static XVisualInfo * vi;
static Colormap cmap;
static XSetWindowAttributes swa;
static Window win;
static GLXContext glc;
static XWindowAttributes gwa;
static XEvent xev;
static int quit = 0;
static int fullscreen = 0;
static Atom wmDeleteMessage;

static pthread_t loop_thread;
static int runLoop = 0;

int
timeval_subtract (struct timeval * result, struct timeval *x, struct timeval *y)
{
	/* Perform the carry for the later subtraction by updating y. */
       if (x->tv_usec < y->tv_usec) {
         int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
         y->tv_usec -= 1000000 * nsec;
         y->tv_sec += nsec;
       }
       if (x->tv_usec - y->tv_usec > 1000000) {
         int nsec = (x->tv_usec - y->tv_usec) / 1000000;
         y->tv_usec += 1000000 * nsec;
         y->tv_sec -= nsec;
       }
     
       /* Compute the time remaining to wait.
          tv_usec is certainly positive. */
       result->tv_sec = x->tv_sec - y->tv_sec;
       result->tv_usec = x->tv_usec - y->tv_usec;
     
       /* Return 1 if result is negative. */
       return x->tv_sec < y->tv_sec;
}

static
void *
loop(void * q)
{
	//XEvent exp;
	//Display * d;
	
	struct timeval then;
	struct timeval now;
	struct timeval diff;
	useconds_t sleep_time = 0;
	
	gettimeofday(&then, NULL);
	
	//d = XOpenDisplay(NULL);
	
	/*memset(&exp, 0, sizeof(exp));
	exp.type = Expose;
	exp.xexpose.window = win;
	XSendEvent(d, win, False, ExposureMask, &exp);
	XFlush(d);*/
	
	while(runLoop)
	{
        App_OnUpdate();
		
		then.tv_usec += SKIP_TICKS;
		gettimeofday(&now, NULL);
		
		if(!timeval_subtract(&diff, &then, &now)) {
			sleep_time = /*(diff.tv_sec / 10000.0) + */ diff.tv_usec;
			usleep(sleep_time);
			//printf("sleep_time: %d diff.tv_sec: %d\n", sleep_time, diff.tv_sec);
		}
	}
	
	return NULL;
}

static
void
startLoop()
{
	runLoop = 1;
	pthread_create(&loop_thread, NULL, loop, NULL);
}

static
void
stopLoop()
{
	runLoop = 0;
	//puts("loop should stop now");
}

static
void
EnableOpenGL()
{
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

static
void
DestroyWindow()
{
	stopLoop();
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, xev.xclient.window);
	XCloseDisplay(dpy);
}

static
void
Quit()
{
	DestroyWindow();
	exit(0);
}

static
void
CreateWindow()
{
	unsigned long valuemask = 0;
	int width;
	int height;
	XWindowAttributes xwa;
	int desktop_w;
	int desktop_h;
	
	dpy = XOpenDisplay(NULL);

	if(dpy == NULL) {
		puts("\n\tcannot connect to X server\n");
		exit(0);
	}

	root = DefaultRootWindow(dpy);
	vi = glXChooseVisual(dpy, 0, att);

	if(vi == NULL) {
		puts("\n\tno appropriate visual found\n");
		exit(0);
	} 

	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask;
	swa.background_pixel = 1;
	
	//get width and height of desktop
	XGetWindowAttributes(dpy, DefaultRootWindow(dpy), &xwa);
	desktop_w = xwa.width;
	desktop_h = xwa.height;
	
	if(fullscreen) {
		//crazy masking hack to bypass window manager (i.e. pure X11)
		swa.override_redirect = True;
		valuemask |= CWOverrideRedirect;
		width = desktop_w;
		height = desktop_h;
	} else {
		width = WIN_INIT_WIDTH;
		height = WIN_INIT_HEIGHT;
	}
	
	win = XCreateWindow(dpy, root, 
		0, 0, 
		width, height, 
		0, vi->depth, InputOutput, vi->visual, 
		CWColormap | CWEventMask | valuemask, &swa);
	
	XMapWindow(dpy, win);
	//lower first, move it, enable opengl, *then* show it
	XLowerWindow(dpy, win);
	XStoreName(dpy, win, APP_NAME);
	
	XGrabKeyboard(dpy, win, True, GrabModeAsync,
            GrabModeAsync, CurrentTime);
	//XGrabPointer(dpy, win, True, ButtonPressMask,
	//	GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
		
	if(!fullscreen) {
		int x;
		int y;
		
		x = (desktop_w - width) / 2;
		y = (desktop_h - height) / 2;
		
		XMoveWindow(dpy, win, x, y);
	}
	
	// Register interest in the delete window message.
	// This allows us to catch the close button window message directly
	// from the window manager and handle it ourselves.
	wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", 0);
	XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);
	
	// make sure to enable OpenGL before showing window,
	// otherwise weird window flash/artifact for a second.
	EnableOpenGL();
	XRaiseWindow(dpy, win);
}

void
ToggleFullscreen()
{
	fullscreen = !fullscreen;
	DestroyWindow();
	CreateWindow();
	App_OnInit();
}

void
OnQuitRequest()
{
	puts("Quit requested...");
	quit = 1;
}

int main(int argc, char *argv[])
{
	char text[255] = { '\0' };
	KeySym sym;
	int ucs;
	cs_key_mod_t mods = CS_NONE;
	
	//XInitThreads();
	
	CreateWindow();
	
	App_OnInit();
	App_AnimationDel(startLoop, stopLoop);
	App_FullscreenDel(ToggleFullscreen);
	App_QuitRequestDel(OnQuitRequest);

	while(!quit) {
		if(XPending(dpy)) {
			XNextEvent(dpy, &xev);
		
			//have to manually handle the window close message
			if (xev.type == ClientMessage &&
				xev.xclient.data.l[0] == wmDeleteMessage) {
				puts("Quitting...");
				quit = 1;
			} else if(xev.type == Expose) {
				XWindowAttributes old_gwa = gwa;
				XGetWindowAttributes(dpy, win, &gwa);
				
				//check for resize
				if(old_gwa.width != gwa.width || old_gwa.height != gwa.height) {
					App_OnResize(gwa.width, gwa.height);
					puts("resizing");
				}
			} else if(xev.type == KeyPress) {
				//pass in shifted so that it returns uppercase/lowercase
				sym = XLookupKeysym(&xev.xkey, MODS_SHIFTED(mods));

				switch(sym) {
				case XK_Super_L:     mods |= CS_SUPER_L;                          break;
				case XK_Super_R:     mods |= CS_SUPER_R;                          break;
				case XK_Alt_L:       mods |= CS_ALT_L;                            break;
				case XK_Alt_R:       mods |= CS_ALT_R;                            break;
				case XK_Control_L:   mods |= CS_CONTROL_L;                        break;
				case XK_Control_R:   mods |= CS_CONTROL_R;                        break;
				case XK_Shift_L:     mods |= CS_SHIFT_L;                          break;
				case XK_Shift_R:     mods |= CS_SHIFT_R;                          break;
				case XK_Escape:      App_OnSpecialKeyDown(CS_ESCAPE,mods);        break;
				case XK_Left:        App_OnSpecialKeyDown(CS_ARROW_LEFT,mods);    break;
				case XK_Right:       App_OnSpecialKeyDown(CS_ARROW_RIGHT,mods);   break;
				case XK_Up:          App_OnSpecialKeyDown(CS_ARROW_UP,mods);      break;
				case XK_Down:        App_OnSpecialKeyDown(CS_ARROW_DOWN,mods);    break;
				default:
					ucs = keysym2ucs(sym);
					
					if(ucs < 0) {
						XLookupString(&xev.xkey, text, sizeof(text), &sym, NULL);
						text[1] = '\0';
					} else {
						int len = utf8proc_encode_char(ucs, text);
						text[len] = '\0';
					}
					
					if(*text) {
						if(*text == '\r') {
							//puts("Converted CR to LF");
							*text = '\n';
						}
						App_OnKeyDown(text, mods);
					}
					break;
				}
				puts("key press");
			} else if(xev.type == KeyRelease) {
				unsigned short is_retriggered = 0;

				if(XEventsQueued(dpy, QueuedAfterReading)) {
					XEvent nev;
					XPeekEvent(dpy, &nev);
					
					if (nev.type == KeyPress && 
						nev.xkey.time == xev.xkey.time &&
						nev.xkey.keycode == xev.xkey.keycode) {
						// delete retriggered KeyPress event
						// XNextEvent(dpy, &xev);
						is_retriggered = 1;
					}
				}

				if(!is_retriggered) {
					puts("real key release");
					//pass in shifted so that it returns uppercase/lowercase
					sym = XLookupKeysym(&xev.xkey, MODS_SHIFTED(mods));
					
					switch(sym) {
					case XK_Super_L:     mods ^= CS_SUPER_L;                          break;
					case XK_Super_R:     mods ^= CS_SUPER_R;                          break;
					case XK_Alt_L:       mods ^= CS_ALT_L;                            break;
					case XK_Alt_R:       mods ^= CS_ALT_R;                            break;
					case XK_Control_L:   mods ^= CS_CONTROL_L;                        break;
					case XK_Control_R:   mods ^= CS_CONTROL_R;                        break;
					case XK_Shift_L:     mods ^= CS_SHIFT_L;                          break;
					case XK_Shift_R:     mods ^= CS_SHIFT_R;                          break;
					case XK_Escape:      App_OnSpecialKeyUp(CS_ESCAPE,mods);          break;
					case XK_Left:        App_OnSpecialKeyUp(CS_ARROW_LEFT,mods);      break;
					case XK_Right:       App_OnSpecialKeyUp(CS_ARROW_RIGHT,mods);     break;
					case XK_Up:          App_OnSpecialKeyUp(CS_ARROW_UP,mods);        break;
					case XK_Down:        App_OnSpecialKeyUp(CS_ARROW_DOWN,mods);      break;
					default:
						break;
					}
				}
			}
		}
		App_OnRender();
		glXSwapBuffers(dpy, win);
	}
	
	App_OnDestroy();
	
	Quit();
	
	return 0;
}
