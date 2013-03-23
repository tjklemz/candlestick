/*************************************************************************
 * utils.h -- Defines some utilities used project-wide.
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
