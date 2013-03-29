/*************************************************************************
 * disp.c
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

#include "disp.h"
#include "opengl.h"
#include "fnt.h"
#include "utils.h"
#include <math.h>

#define LINE_HEIGHT 1.8f


/**************************************************************************
 * Scrolling
 **************************************************************************/

#define NUM_STEPS 10
#define STEP_AMT (1.0f / NUM_STEPS)

typedef enum {
	SCROLL_UP,
	SCROLL_DOWN
} scrolling_dir_t;

typedef struct {
	int moving;
	float amt;
	scrolling_dir_t dir;
} scrolling_t;

static scrolling_t scroll;
static int scroll_requested = 0;

static void Disp_UpdateScroll(float amt)
{
	scroll.amt += amt;
	
	if(scroll.amt < 0) {
		scroll.amt = 0;
	}
}

static void Disp_Scroll(float amt)
{
	static int step = 0;
	
	if(step < NUM_STEPS) {
		Disp_UpdateScroll(amt);
		scroll.moving = 1;
		++step;
	} else {
		scroll.moving = 0;
		step = 0;
	}
}

// How about pass in the Frame and have Display set the begin and end?
// (since must know about the screen size, etc)
// Like a scroll module, almost.

// Have Disp calc how many lines to display? Then pass in to Frame?
// How to know where to begin, though? Also, the top (when to stop scrolling)

void
Disp_ScrollUpRequested()
{
	scroll_requested = 1;
	scroll.dir = SCROLL_UP;
	
	if(!scroll.moving) {
		Disp_Scroll(STEP_AMT);
	}
}

void
Disp_ScrollDownRequested()
{
	scroll_requested = 1;
	scroll.dir = SCROLL_DOWN;
	
	if(!scroll.moving) {
		Disp_Scroll(-STEP_AMT);
	}
}

void
Disp_ScrollStopRequested()
{
	scroll_requested = 0;
}

void
Disp_ScrollReset()
{
	scroll.moving = 0;
	scroll.amt = 0;
	scroll.dir = SCROLL_UP; // doesn't matter, but good to know the state
}


/**************************************************************************
 * Display
 **************************************************************************/

static int disp_h = 1;
static int disp_w = 1;
static Fnt * fnt_reg = 0;
static const char * const fnt_reg_name = "./font/Lekton-Regular.ttf";

void
Disp_Init(int fnt_size)
{
	scroll.moving = 0;
	scroll.amt = 0.0f;
	scroll.dir = SCROLL_UP;
	
	fnt_reg = Fnt_Init(fnt_reg_name, fnt_size, 1.8f);

	glShadeModel(GL_SMOOTH);
    glClearColor(0.8825f, 0.8825f, 0.87f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void
Disp_Destroy()
{
	Fnt_Destroy(fnt_reg);
}

//Frame is passed in, since input needs to deal with the Frame
// and Display does not handle input, but only displaying
void
Disp_Render(Frame * frm)
{	
	//window coords for start of frame
	float fnt_size = Fnt_Size(fnt_reg);
	float disp_x = (disp_w - (Frame_Length(frm)*fnt_size)) / 2;
	float disp_y = disp_h / 2;
	
	if(scroll.moving || scroll_requested) {
		switch(scroll.dir) {
		case SCROLL_UP:
			Disp_Scroll(STEP_AMT);
			break;
		case SCROLL_DOWN:
			Disp_Scroll(-STEP_AMT);
			break;
		default:
			break;
		}
	}
	
	disp_y = disp_y - scroll.amt * Fnt_LineHeight(fnt_reg);
	
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);

	glColor3ub(50, 31, 20);

	glPushMatrix();
		glLoadIdentity();
		Fnt_Print(fnt_reg, frm, disp_x, disp_y, 50 + (int)ceil(scroll.amt));
	glPopMatrix();
}

void
Disp_Resize(int w, int h)
{
	// height and width ratio
    GLfloat ratio;
 
    // protect against a divide by zero
	if(h == 0) {
		h = 1;
	}

	disp_h = h;
	disp_w = w;

    ratio = (GLfloat)w / (GLfloat)h;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
