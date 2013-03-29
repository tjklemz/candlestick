/*************************************************************************
 * fnt.h -- A simple opengl font library that uses GNU freetype2
 *			and OpenGL for rendering. Can read .ttf and .otf font files.
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

#ifndef MY_FONT_H
#define MY_FONT_H

#include "opengl.h"
#include "frame.h"

//FreeType Headers
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/fttrigon.h"

typedef struct fnt_data Fnt;

/**********************************************************************
 * return the font size in points
 **********************************************************************/
float Fnt_Size(Fnt * fnt);

/**********************************************************************
 * returns the line height
 **********************************************************************/
float Fnt_LineHeight(Fnt * fnt);

/**********************************************************************
 * creates a fnt with a given name and height (in points)
 **********************************************************************/
Fnt * Fnt_Init(const char * fname, unsigned int height, float line_height);

/**********************************************************************
 * destroys the fnt (destructor)
 **********************************************************************/
void Fnt_Destroy(Fnt * fnt);

/**********************************************************************
 * prints text at window coords (x,y) using the fnt
 **********************************************************************/

void Fnt_Print(Fnt * fnt, Frame * frm, int x, int y, int max_lines);


#endif
