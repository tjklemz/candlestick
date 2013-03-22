// utils.c

#include "utils.h"

#include "opengl.h"

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
		gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
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
