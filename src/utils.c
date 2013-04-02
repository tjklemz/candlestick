/*************************************************************************
 * utils.c -- Defines some basic utilities used project-wide.
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

#include "utils.h"

#include "opengl.h"

#include <math.h>

/**********************************************************************
 * nextP2
 * 
 * gets the first power of 2 that is greater than or equal to
 * the passed in integer
 **********************************************************************/

int
NextP2(int a)
{
	int rval = 1;
	while (rval < a) rval <<= 1;
	return rval;
}


/**********************************************************************
 * PushScreenCoordMat
 * 
 * makes object world coords identical to window coords
 * the new projection matrix is left on the stack
 **********************************************************************/
 
void 
PushScreenCoordMat() 
{
	GLint viewport[4];

	glPushAttrib(GL_TRANSFORM_BIT);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		//gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
		gluOrtho2D(viewport[1], viewport[2], viewport[3], viewport[0]);
	//the matrix is now on the stack and setup; leave it
	glPopAttrib();
}


/**********************************************************************
 * PopScreenCoordMat
 * 
 * pops the projection matrix without changing the current matrix mode
 **********************************************************************/
 
void 
PopScreenCoordMat() 
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}
