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

#include "opengl.h"
#include "app.h"

static const char * const APP_NAME = "Candlestick";

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
	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, glc);
	XDestroyWindow(dpy, win);
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
		printf("\n\tcannot connect to X server\n\n");
		exit(0);
	}

	root = DefaultRootWindow(dpy);
	vi = glXChooseVisual(dpy, 0, att);

	if(vi == NULL) {
		printf("\n\tno appropriate visual found\n\n");
		exit(0);
	} 

	cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);

	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask;
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
	XGrabPointer(dpy, win, True, ButtonPressMask,
		GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
		
	if(!fullscreen) {
		int x;
		int y;
		
		x = (desktop_w - width) / 2;
		y = (desktop_h - height) / 2;
		
		XMoveWindow(dpy, win, x, y);
	}
	
	
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

int main(int argc, char *argv[])
{
	char text[255];
	KeySym key;
	
	CreateWindow();
	
	App_OnInit();

	while(!quit) {
		while(XPending(dpy)) {
			XNextEvent(dpy, &xev);
			
			if(xev.type == Expose) {
				XGetWindowAttributes(dpy, win, &gwa);
				App_OnResize(gwa.width, gwa.height);
			} else if(xev.type == KeyPress) {
				XLookupString(&xev.xkey, text, sizeof(text), &key, 0);
				
				switch(key) {
					case XK_Escape:
						quit = 1;
						break;
					case XK_F1:
					{
						ToggleFullscreen();
						break;
					}
					default:
						App_OnKeyDown(text[0]);
						break;
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