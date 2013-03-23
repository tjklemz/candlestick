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

static int disp_h = 1;
static int disp_w = 1;
static Fnt * fnt_reg = 0;
static const char * const fnt_reg_name = "./font/Lekton-Regular.ttf";

void
Disp_Init(int fnt_size)
{
	fnt_reg = Fnt_Init(fnt_reg_name, fnt_size);

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
	float disp_y = disp_h / 2 + fnt_size;
	
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);

	glColor3ub(50, 31, 20);

	glPushMatrix();
	//PushScreenCoordMat();
		glLoadIdentity();
	//	glTranslatef(0, -200, 0);
		//glMultMatrixf(modelview_matrix);
		Fnt_Print(fnt_reg, frm, disp_x, disp_y);
	//PopScreenCoordMat();
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
