#ifndef CS_UTILS_H
#define CS_UTILS_H

/**********************************************************************
 * nextP2
 * 
 * gets the first power of 2 that is greater than or equal to
 * the passed in integer
 **********************************************************************/

int
NextP2(int a);


/**********************************************************************
 * PushScreenCoordMat
 * 
 * makes object world coords identical to window coords
 * the new projection matrix is left on the stack
 **********************************************************************/
 
void 
PushScreenCoordMat();


/**********************************************************************
 * PopScreenCoordMat
 * 
 * pops the projection matrix without changing the current matrix mode
 **********************************************************************/
 
void 
PopScreenCoordMat();

#endif
