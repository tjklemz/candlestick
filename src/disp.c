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
	int moving;
	double amt;
	double limit;
	scrolling_dir_t dir;
} scrolling_t;

// Initialize the scroll amount values here, instead of in Init.
// This way, the Disp can be recreated without reseting the scroll.
static scrolling_t scroll = {0, 0.0, 0.0, SCROLL_UP};

static int scroll_requested = 0;
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
	if(!scroll_requested && !scroll.moving) {
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
	static unsigned int step = 0;
	
	if((step < NUM_STEPS || scroll_requested)) {
#if defined(_WIN32)
		scroll.amt += amt*pow((double)(step + 1) / 100.0, 2.125);
#else
		scroll.amt += amt*pow(step, 0.7f);
#endif
		
		if(scroll.amt < 0) {
			scroll.amt = 0;
		} else if(scroll.amt > scroll.limit) {
			scroll.amt = scroll.limit;
		}
		scroll.moving = 1;
		++step;
	} else {
		scroll.moving = 0;
		step = 0;
		
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
	scroll_requested = 1;
	scroll.dir = SCROLL_UP;
	
	Disp_AnimStart();
}

void
Disp_ScrollDownRequested()
{
	scroll_requested = 1;
	scroll.dir = SCROLL_DOWN;
	
	Disp_AnimStart();
}

void
Disp_ScrollStopRequested()
{
	scroll_requested = 0;
}

void
Disp_ScrollReset()
{
	scroll_requested = 0;
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
static Fnt * fnt_heading = 0;

static const char * const fnt_reg_name = "./font/Lekton-Regular.ttf";
static const char * const fnt_heading_name = "./font/LiberationSans-Bold.ttf";

void
Disp_Init(int fnt_size)
{
	scroll.moving = 0;
	scroll_requested = 0;
	
	fnt_reg = Fnt_Init(fnt_reg_name, fnt_size, LINE_HEIGHT);
	fnt_heading = Fnt_Init(fnt_heading_name, 60, 2);

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
	//TODO: refactor the Fnt module so can use multiple fonts
	//Fnt_Destroy(fnt_heading);
}

void
Disp_BeginRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);
	glColor3ub(50, 31, 20);
}

void
Disp_Update()
{
	// if scrolling, update the animation
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
		glColor3ub(50, 31, 20);
		glLoadIdentity();
		Fnt_PrintFrame(fnt_reg, frm, disp_x, disp_y, num_lines + DISP_LINE_PADDING, show_cursor);
	glPopMatrix();
}

#define HEADING_SEP_COL glColor3ub(0, 161, 154);
#define HEADING_COL     glColor3ub(0, 161, 154);
#define LINES_COL       glColor3ub(0, 161, 154);
#define TEXT_COL        glColor3ub(64, 64, 64);

void
Disp_SaveScreen(char * filename)
{
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*Fnt_Width(fnt_reg))) / 2);
	float disp_y = disp_h / 2;
	//float line_height = Fnt_LineHeight(fnt_reg) * 1.25 * Fnt_Width(fnt_reg);
	
	float orig_size = Fnt_Size(fnt_heading);
	
	glPushMatrix();
		glLoadIdentity();

		HEADING_SEP_COL
		PushScreenCoordMat();
		glBegin(GL_QUADS);
			glVertex2f(disp_x, disp_y - 134);
			glVertex2f(disp_x, disp_y - 135);
			glVertex2f(disp_w - disp_x, disp_y - 135);
			glVertex2f(disp_w - disp_x, disp_y - 134);
		glEnd();

#define BOX_TOP 32
#define BOX_BOT 14

		LINES_COL
		glBegin(GL_LINES);
			//the box around the filename
			glVertex2f(disp_x, disp_y - BOX_TOP);
			glVertex2f(disp_x, disp_y + BOX_BOT);

			glVertex2f(disp_x, disp_y + BOX_BOT);
			glVertex2f(disp_w - disp_x, disp_y + BOX_BOT);

			glVertex2f(disp_w - disp_x, disp_y + BOX_BOT);
			glVertex2f(disp_w - disp_x, disp_y - BOX_TOP);

			glVertex2f(disp_w - disp_x, disp_y - BOX_TOP);
			glVertex2f(disp_x, disp_y - BOX_TOP);
			//the frame line at the bottom
			glVertex2f(disp_x, disp_y + 120);
			glVertex2f(disp_w - disp_x, disp_y + 120);
		glEnd();
		PopScreenCoordMat();

		HEADING_COL
		Fnt_Print(fnt_heading, "save", disp_x, disp_y - 135, 0);
		
		TEXT_COL

		Fnt_SetSize(fnt_heading, orig_size * 0.35);
		Fnt_Print(fnt_heading, "Enter a filename:", disp_x, disp_y - 40, 0);

		Fnt_SetSize(fnt_heading, orig_size * 0.25);
		Fnt_Print(fnt_heading, "Press enter when done. The .txt extension is added automatically.", disp_x, disp_y + 114, 0);
		Fnt_SetSize(fnt_heading, orig_size);

		glColor3ub(16, 16, 16);
		Fnt_Print(fnt_reg, filename, disp_x + 10, disp_y, 1);
		//Fnt_Print(fnt_reg, ".txt", x, disp_y, 0);
		
	glPopMatrix();
}

void
Disp_OpenScreen(Node * files)
{
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*Fnt_Width(fnt_reg))) / 2);
	float disp_y = disp_h / 2;
	float orig_size = Fnt_Size(fnt_heading);
	Node * cur;
	int line;
	
	glPushMatrix();
		glLoadIdentity();

		PushScreenCoordMat();
		
		//draw a line for the heading
		HEADING_SEP_COL
		glBegin(GL_QUADS);
			glVertex2f(disp_x, 110);
			glVertex2f(disp_x, 111);
			glVertex2f(disp_w - disp_x, 111);
			glVertex2f(disp_w - disp_x, 110);
		glEnd();

		LINES_COL
		glBegin(GL_LINES);
			//the frame line at the bottom
			glVertex2f(disp_x, disp_y + 110);
			glVertex2f(disp_w - disp_x, disp_y + 110);
		glEnd();
		
		HEADING_COL
		//print header
		Fnt_Print(fnt_heading, "open", disp_x, 110, 0);
		
		Fnt_SetSize(fnt_heading, orig_size * 0.4);
		
		TEXT_COL

		//print files
		if(!files) {
			Fnt_Print(fnt_heading, "No files.", disp_x, 165, 0);
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
				
				Fnt_Print(fnt_heading, (char*)cur->data, disp_x, 165 + 40*line, 0);
			}
		}
		
		Fnt_SetSize(fnt_heading, orig_size);
		
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
