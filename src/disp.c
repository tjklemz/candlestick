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

#define LINE_HEIGHT 1.95f


/**************************************************************************
 * Scrolling and Animation
 **************************************************************************/

#define NUM_STEPS 22
#define STEP_AMT (0.0006f)

typedef enum {
	SCROLL_UP,
	SCROLL_DOWN
} scrolling_dir_t;

typedef struct {
	int requested;
	unsigned int step;
	int moving;
	double amt;
	double limit;
	scrolling_dir_t dir;
} scrolling_t;

// Initialize the scroll amount values here, instead of in Init.
// This way, the Disp can be recreated without reseting the scroll.
static scrolling_t scroll = {0, 0, 0, 0.0, 0.0, SCROLL_UP};
static anim_del_t * anim_del = 0;

static
void
Disp_AnimStart()
{
	if(anim_del && !anim_del->called_start) {
		anim_del->on_start();
		anim_del->called_start = 1;
		anim_del->called_end = 0;
	}
}

static
void
Disp_AnimEnd()
{
	if(!scroll.requested && !scroll.moving) {
		if(anim_del && !anim_del->called_end) {
			anim_del->on_end();
			anim_del->called_end = 1;
			anim_del->called_start = 0;
		}
	}
}

static
void
Disp_Scroll(float amt)
{	
	if((scroll.step < NUM_STEPS || scroll.requested)) {
#if defined(_WIN32)
		scroll.amt += amt*pow((double)scroll.step / 100.0, 2.125);
#else
		scroll.amt += amt*pow((double)scroll.step / 100.0, 1.22);
#endif
		
		if(scroll.amt < 0) {
			scroll.amt = 0;
		} else if(scroll.amt > scroll.limit) {
			scroll.amt = scroll.limit;
		}
		scroll.moving = 1;
		++scroll.step;
	} else {
		scroll.moving = 0;
		scroll.step = 0;
		
		Disp_AnimEnd();
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
	scroll.requested = 1;
	scroll.dir = SCROLL_UP;
	
	Disp_AnimStart();
}

void
Disp_ScrollDownRequested()
{
	scroll.requested = 1;
	scroll.dir = SCROLL_DOWN;
	
	Disp_AnimStart();
}

void
Disp_ScrollStopRequested()
{
	scroll.requested = 0;
}

void
Disp_ScrollReset()
{
	scroll.requested = 0;
	scroll.step = 0;
	scroll.moving = 0;
	scroll.amt = 0;
	scroll.dir = SCROLL_UP; // doesn't matter, but good to know the state
	
	Disp_AnimEnd();
}


/**************************************************************************
 * Display
 **************************************************************************/

static int disp_h = 1;
static int disp_w = 1;
static Fnt * fnt_reg = 0;

static const char * const fnt_reg_name = "./font/Lekton-Regular.ttf";

#define TEXT_COLOR     glColor3ub(50, 31, 20);
#define DRAWING_COLOR  glColor3ub(64, 64, 64);

void
Disp_Init(int fnt_size)
{
	scroll.moving = 0;
	scroll.requested = 0;
	
	fnt_reg = Fnt_Init(fnt_reg_name, fnt_size, LINE_HEIGHT);

	glShadeModel(GL_SMOOTH);
	//NOTE: background color matched with TEXT_COLOR
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
	//NOTE: should refactor the Fnt module so can use multiple fonts
}

void
Disp_BeginRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);
	TEXT_COLOR
}

void
Disp_Update()
{
	// if scrolling, update the animation
	if(scroll.moving || scroll.requested) {
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
}

#define DISP_LINE_PADDING 2

//Frame is passed in, since input needs to deal with the Frame
// and Display does not handle input, but only displaying
void
Disp_TypingScreen(Frame * frm)
{	
	//window coords for start of frame
	float fnt_width = Fnt_Width(fnt_reg);
	float line_height = Fnt_LineHeight(fnt_reg) * 1.5 * fnt_width;
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*fnt_width)) / 2);
	//printf("disp_x: %f, disp_w: %d, fnt_width: %f\n", disp_x, disp_w, fnt_width);
	float disp_y = disp_h / 2;
	int num_lines;
	int first_line;
	int show_cursor;
	double scroll_amt;
	
	scroll.limit = Frame_NumLines(frm) - 1;

	scroll_amt = scroll.amt;
	
	// figure out num lines
	num_lines = (int)ceil(disp_h / line_height);
	first_line = (int)(scroll_amt - num_lines / 2);
	
	if(first_line <= 1) {
		first_line = 1;
		show_cursor = 1;
	} else {
		show_cursor = 0;
	}
	
	// crop the frame to what is viewable
	Frame_SetEnd(frm, first_line);
	
	// scroll the display appropriately, minus the part that isn't viewable
	// NOTE: (0, 0) in screen coords is now the top left of the window
	disp_y = disp_y - line_height * (-scroll_amt + first_line - 1);
	
	//printf("num_lines: %d\tfirst_line: %d\tdisp_y: %f\n", num_lines, first_line, disp_y);

	glPushMatrix();
		TEXT_COLOR
		glLoadIdentity();
		Fnt_PrintFrame(fnt_reg, frm, disp_x, disp_y, num_lines + DISP_LINE_PADDING, show_cursor);
	glPopMatrix();
}

static
void
Disp_DrawSaveIcon(int x, int y)
{
	//N.B. Everything assumes (0,0) is top left corner
	float h = 100.0f;
	float w = 100.0f;
	float l_w = 15.0f;
	float b_h = 10.0f;
	// the right side is a polygon, so multiple parts
	// r1: the right vertical strip
	float r1_w = 12.0f;
	// r2: the middle horizontal strip
	float r2_h = 12.0f;
	float r2_h2 = b_h + 26.0f;
	// r3: the strip of the tip
	float r3_w = 10.0f;
	// r4: the triangle of the tip
	float r4_w = 25.0f;
	float r4_h = 20.0f;

	/*glPushAttrib(GL_POLYGON_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	glPushMatrix();
	glTranslatef(x, y, 0.0f);
	glScalef(0.4f, 0.4f, 1.0f);

	glBegin(GL_QUADS);
		// left side
		glVertex2f(0.0f, 0.0f);
		glVertex2f(0.0f, h);
		glVertex2f(l_w, h);
		glVertex2f(l_w, 0.0f);

		// bottom strip
		glVertex2f(l_w, h - b_h);
		glVertex2f(l_w, h);
		glVertex2f(w - r1_w, h);
		glVertex2f(w - r1_w, h - b_h);

		// right side
		glVertex2f(w - r1_w, h);
		glVertex2f(w - r1_w, r2_h2);
		glVertex2f(w, r2_h2 + r2_h);
		glVertex2f(w, h);

		// horizontal strip
		glVertex2f(l_w, r2_h2);
		glVertex2f(l_w, r2_h2 + r2_h);
		glVertex2f(w - r4_w - r3_w, r2_h2 + r2_h);
		glVertex2f(w - r4_w - r3_w, r2_h2);

		// the two lines representing text
		// bottom:
		glVertex2f(l_w * 1.75, h - b_h * 2.0);
		glVertex2f(l_w * 1.75, h - b_h * 2.5);
		glVertex2f(w - r1_w * 1.9, h - b_h * 2.5);
		glVertex2f(w - r1_w * 1.9, h - b_h * 2.0);

		// top:
		glVertex2f(l_w * 1.75, h - b_h * 3.75);
		glVertex2f(l_w * 1.75, h - b_h * 4.25);
		glVertex2f(w - r1_w * 1.9, h - b_h * 4.25);
		glVertex2f(w - r1_w * 1.9, h - b_h * 3.75);

		// the "slider" on top
		glVertex2f(w - r4_w - 1.84 * r3_w, -0.45 * r2_h + r2_h2);
		glVertex2f(w - r4_w - 1.84 * r3_w, r3_w * 0.374);
		glVertex2f(w - r4_w - 3.7 * r3_w, r3_w * 0.374);
		glVertex2f(w - r4_w - 3.7 * r3_w, -0.45 * r2_h + r2_h2);
	glEnd();
	glBegin(GL_POLYGON);
		// triangle-like shape (tip)
		glVertex2f(w - r4_w - r3_w, r2_h2 + r2_h);
		glVertex2f(w - r4_w - r3_w, 0.0f);
		glVertex2f(w - r4_w, 0.0f);
		glVertex2f(w, r4_h);
		glVertex2f(w, r2_h2 + r2_h);
	glEnd();

	glPopMatrix();

	//glPopAttrib();
}

static
void
Disp_DrawInputBox(int left, int right, int middle)
{
	static const int BOX_TOP = 32;
	static const int BOX_BOT = 14;

	glPushAttrib(GL_POLYGON_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBegin(GL_QUADS);

	glVertex2f(left, middle - BOX_TOP);
	glVertex2f(left, middle + BOX_BOT);
	glVertex2f(right, middle + BOX_BOT);
	glVertex2f(right, middle - BOX_TOP);

	glEnd();
	
	glPopAttrib();
}

void
Disp_SaveScreen(char * filename)
{
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*Fnt_Width(fnt_reg))) / 2);
	float disp_y = disp_h / 2;
	
	glPushMatrix();
		glLoadIdentity();

		PushScreenCoordMat();

		DRAWING_COLOR
		Disp_DrawSaveIcon(disp_x, disp_y - 84);
		Disp_DrawInputBox(disp_x, disp_w - disp_x, disp_y);

		PopScreenCoordMat();
		
		TEXT_COLOR
		Fnt_Print(fnt_reg, filename, disp_x + 10, disp_y, 1);
		
	glPopMatrix();
}

void
Disp_OpenScreen(Node * files)
{
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*Fnt_Width(fnt_reg))) / 2);
	float disp_y = disp_h / 2;
	Node * cur;
	int line;
	
	glPushMatrix();
		glLoadIdentity();

		PushScreenCoordMat();

		DRAWING_COLOR
		
		//draw a line for the heading
		glBegin(GL_LINES);
			glVertex2f(disp_x, 114);
			glVertex2f(disp_w - disp_x, 114);
		glEnd();
		
		//print header
		Fnt_Print(fnt_reg, "Open", disp_x, 110, 0);
		
		TEXT_COLOR

		//print files
		if(!files) {
			Fnt_Print(fnt_reg, "No files.", disp_x, 165, 0);
		} else {
			//int sel_box_x1 = disp_x;
			//int sel_box_x2 = sel_box_x1 + 200;
			//int sel_box_y1 = -scroll.amt*10;
			//int sel_box_y2 = sel_box_y1 + 100;

			for(cur = files, line = 0; cur; cur = cur->next, ++line) {
				/*glColor3ub(30, 30, 30);

				glBegin(GL_QUADS);
					glVertex2f(sel_box_x1, sel_box_y1);
					glVertex2f(sel_box_x1, sel_box_y2);
					glVertex2f(sel_box_x2, sel_box_y2);
					glVertex2f(sel_box_x2, sel_box_y1);
					//glVertex2f(disp_x - 20, 140 - 40*line);
					//glVertex2f(disp_x - 20, 140 + 40*line);
					//glVertex2f(disp_w - disp_x + 20, 140 + 40*line);
					//glVertex2f(disp_w - disp_x + 20, 140 - 40*line);
				glEnd();*/

				//if(sel_line == line)
				
				Fnt_Print(fnt_reg, (char*)cur->data, disp_x, 158 + 40*line, 0);
			}
		}
		
		PopScreenCoordMat();
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

void
Disp_AnimationDel(anim_del_t * the_anim_del)
{
	anim_del = the_anim_del;
}
