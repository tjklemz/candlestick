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
#define OPEN_SCREEN_LINE_HEIGHT 40


/**************************************************************************
 * Display
 **************************************************************************/

static int disp_h = 1;
static int disp_w = 1;
static Fnt * fnt_reg = 0;

static int save_anim = 0;
static int save_anim_amt = 0;
static int save_err_anim = 0;
static double save_err_anim_amt = 0.0;
static int open_err_anim = 0;
static double open_err_anim_amt = 0.0;
static anim_del_t * anim_del = 0;

static const char * const fnt_reg_name = "./font/Lekton-Regular.ttf";

#define TEXT_COLOR     glColor3ub(50, 31, 20);
#define DRAWING_COLOR  glColor3ub(64, 64, 64);
#define ERR_COLOR      glColor3ub(199, 31, 0);
#define BG_COLOR       glColor3f(0.8825f, 0.8825f, 0.87f);


static
void
Disp_DrawSaveIcon(int x, int y);

static
void
Disp_DrawOpenIcon(int x, int y);


void
Disp_Init(int fnt_size)
{	
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
Disp_TriggerSaveAnim()
{
	save_anim = 1;
	Anim_Start(anim_del);
}

void
Disp_TriggerSaveErrAnim()
{
	save_err_anim = 1;
	Anim_Start(anim_del);
}

void
Disp_TriggerOpenErrAnim()
{
	open_err_anim = 1;
	Anim_Start(anim_del);
}


#define ANIM_AMT_MAX 80
#define ANIM_W 30.0
#define ANIM_H 50.0

static
void
Disp_UpdateSaveAnim()
{
	static int t = 1;
	static int step = 0;

	++step;

	if(step % 6 == 0) {
		/*
		 * There are two options here: one is to do a fourier series
		 * to mimic a square wave, the other is to do a piece-wise
		 * function that starts as a decreasing sinusoidal wave then
		 * turns into a decreasing exponential after a certain time.
		 *
		 * The first option works fine, but the second option produces
		 * a smoother, more cartoonish animation that I personally prefer.
		 */

		/* The fourier:
		int i, k;
		save_anim_amt = (2*ANIM_H/3);
		for(i = 0; i < 2; ++i) {
			k = 2*i + 1;
			save_anim_amt += (ANIM_H/3) * (1.0/k)*sin((k*M_PI*t)/ANIM_W);
		}*/

		// the piecewise, with sinusoidal (a bessel function actually), and
		// decreasing cubic at the end:

		if(t < (int)ANIM_W) {
			save_anim_amt = (ANIM_H) + (ANIM_H/2) * (-cos(1.2*t-1) / t);
		} else if(t < (int)(ANIM_W + 10)) {
			save_anim_amt = ANIM_H + -(t - ANIM_W)*(t - ANIM_W)*(t - ANIM_W);
		} else {
			save_anim = 0;
			save_anim_amt = 0;
			step = 0;
			t = 1;

			Anim_End(anim_del);
		}
		++t;
	}
}

static
void
Disp_UpdateSaveErrAnim()
{
	static int t = 0;
	static int step = 0;

	++step;

	if(step % 6 == 0) {
		save_err_anim_amt = 16*sin(t)*exp(-0.2*t);
		
		if(t++ > 20) {
			save_err_anim = 0;
			save_err_anim_amt = 0.0;
			step = 0;
			t = 0;

			Anim_End(anim_del);
		}
	}
}

static
void
Disp_UpdateOpenErrAnim()
{
	static int t = 0;
	static int step = 0;

	++step;

	if(step % 8 == 0) {
		if(t < 18) {
			open_err_anim_amt = 40*sin(t)*exp(-0.35*t);
		} else if(t < 20) {
			open_err_anim_amt = 0.0;
		} else {
			open_err_anim = 0;
			open_err_anim_amt = 0.0;
			step = 0;
			t = 0;

			Anim_End(anim_del);
		}
		
		++t;
	}
}

void
Disp_UpdateAnim()
{
	if(save_anim) {
		Disp_UpdateSaveAnim();
	}

	if(save_err_anim) {
		Disp_UpdateSaveErrAnim();
	}

	if(open_err_anim) {
		Disp_UpdateOpenErrAnim();
	}
}

void
Disp_AnimDel(anim_del_t * the_anim_del)
{
	anim_del = the_anim_del;
}


#define DISP_LINE_PADDING 2

//Frame is passed in, since input needs to deal with the Frame
// and Display does not handle input, but only displaying
void
Disp_TypingScreen(Frame * frm, scrolling_t * scroll)
{	
	//window coords for start of frame
	float fnt_width = Fnt_Width(fnt_reg);
	float line_height = Fnt_LineHeight(fnt_reg) * 1.55 * fnt_width;

	float disp_x = (int)((disp_w - (CHARS_PER_LINE*fnt_width)) / 2);
	float disp_y = 5 + disp_h / 2;

	int num_lines;
	int first_line;
	int show_cursor;
	double scroll_amt;
	
	scroll->limit = Frame_NumLines(frm) - 1;

	scroll_amt = scroll->amt;
	
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

	glPushMatrix();
		if(save_anim) {
			int x = disp_w - (disp_x / 2) - 22;
			int y = disp_h - (int)round(save_anim_amt);

			glPushMatrix();
			glLoadIdentity();

			PushScreenCoordMat();

			DRAWING_COLOR
			Disp_DrawSaveIcon(x, y);

			PopScreenCoordMat();
			glPopMatrix();
		}

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
	float r4_w = 22.0f;
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

		// the "slider" square on top
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
Disp_SaveScreen(char * filename, int err)
{
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*Fnt_Width(fnt_reg))) / 2);
	float disp_y = disp_h / 2;
	
	glPushMatrix();
		glLoadIdentity();

		PushScreenCoordMat();

		if(err) {
			ERR_COLOR
		} else {
			DRAWING_COLOR
		}

		Disp_DrawSaveIcon(disp_x, disp_y - 84 - (int)round(save_err_anim_amt));
		Disp_DrawInputBox(disp_x, disp_w - disp_x, disp_y);

		PopScreenCoordMat();
		
		TEXT_COLOR
		Fnt_Print(fnt_reg, filename, disp_x + 10, disp_y, 1);
		
	glPopMatrix();
}

static
void
Disp_DrawOpenCursor(int x, int y)
{
	int x1 = x;
	int x2 = x1 + 15;
	int y1 = y;
	int y2 = y1 + 24;
	
	glBegin(GL_POLYGON);
		glVertex2f(x1, y1);
		glVertex2f(x1, y2);
		glVertex2f(x2, y2);
		glVertex2f(x2 + 11, y1 + 24.0/2.0);
		glVertex2f(x2, y1);
		glVertex2f(x1, y1);
	glEnd();
}

static
void
Disp_DrawOpenIcon(int x, int y)
{
	int x1 = x;
	int x2 = x1 + 40;
	
	//the top of the box
	int y1 = y;
	int y2 = y1 + 6;
	
	//actual box part
	int y3 = y2 + 2;
	int y4 = y1 + 38;
	
	//the handle part
	int x3 = x1 + 12;
	int x4 = x2 - 12;
	int y5 = y1 + 6 * 3;
	int y6 = y5 + 4;
	
	glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x1, y2);
		glVertex2f(x2, y2);
		glVertex2f(x2, y1);
		
		glVertex2f(x1, y3);
		glVertex2f(x1, y4);
		glVertex2f(x2, y4);
		glVertex2f(x2, y3);
		
		BG_COLOR
		
		glVertex2f(x3, y5);
		glVertex2f(x3, y6);
		glVertex2f(x4, y6);
		glVertex2f(x4, y5);
	glEnd();
}

void
Disp_OpenScreen(files_t * files, scrolling_t * scroll)
{
	float disp_x = (int)((disp_w - (CHARS_PER_LINE*Fnt_Width(fnt_reg))) / 2);
	int line_height = 40;
	int num_lines = (int)ceil(disp_h / line_height) - 6;
	int heading_h = 112;
	int start_h = heading_h + 46;
	float scroll_amt = scroll->amt * line_height;
	int open_cursor_x = (int)(disp_x - 36 - open_err_anim_amt);

	
	glPushMatrix();
		glLoadIdentity();

		PushScreenCoordMat();
		
		glPushMatrix();

			if(files->len > 0) {
				if(open_err_anim) {
					ERR_COLOR
				} else {
					DRAWING_COLOR
				}
		
				if((int)ceil(scroll->amt) > num_lines) {
					scroll_amt = (int)ceil((scroll->amt - num_lines)*line_height);
					Disp_DrawOpenCursor(open_cursor_x, 138 + num_lines*line_height);
					glTranslatef(0.0f, -scroll_amt /* ceil((scroll->amt - num_lines)*line_height) */, 0.0f);
				} else {
					Disp_DrawOpenCursor(open_cursor_x, 138 + scroll_amt);
				}
			}

			TEXT_COLOR

			//print files
			if(files->len == 0) {
				Fnt_Print(fnt_reg, "No files.", disp_x, start_h, 0);
			} else {
				int i;
				for(i = 0; i < files->len; ++i) {
					Fnt_Print(fnt_reg, files->names[i], disp_x, start_h + line_height*i, 0);
				}
			}
		
		glPopMatrix();
		
		BG_COLOR
			
		//draw a box so that any scrolling lines go under it
		glBegin(GL_QUADS);
			glVertex2f(disp_x, 0);
			glVertex2f(disp_x, heading_h + 12);
			glVertex2f(disp_w - disp_x, heading_h + 12);
			glVertex2f(disp_w - disp_x, 0);
		glEnd();
		
		DRAWING_COLOR
		
		//draw a line for the heading
		glBegin(GL_LINES);
			glVertex2f(disp_x, heading_h);
			glVertex2f(disp_w - disp_x, heading_h);
		glEnd();
		
		Disp_DrawOpenIcon(disp_x, heading_h - 50);
		
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
