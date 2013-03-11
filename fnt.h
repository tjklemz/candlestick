/**********************************************************************
 * fnt.h
 * 
 * Defines an interface for using and displaying fnts (with OpenGL)
 * 
 * Written by:		Thomas Klemz
 * 					February 26, 2013
 * 
 * Requires freetype 2 and OpenGL (gl and glu)
 **********************************************************************/

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
 * creates a fnt with a given name and height (in points)
 **********************************************************************/
Fnt * Fnt_Init(const char * fname, unsigned int height);

/**********************************************************************
 * destroys the fnt (destructor)
 **********************************************************************/
void Fnt_Destroy(Fnt * fnt);

/**********************************************************************
 * prints text at window coords (x,y) using the fnt
 **********************************************************************/

//passing in the y is temporary hack; once scrolling is implemented, all
//that will be in Frame or another module (probably Frame, though)
//OR, will always pass in an x and a y, since that is a bit window specific...
//just not sure yet...
void Fnt_Print(Fnt * fnt, Frame * frm, int y);


#endif
