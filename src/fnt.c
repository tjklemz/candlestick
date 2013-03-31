/*************************************************************************
 * fnt.c -- A simple opengl fnt library that uses GNU freetype2
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

#include "fnt.h"
#include "utils.h"
#include <math.h>

#define GLYPH_SPACING 12
#define NUM_CHARS 128

//defined as Fnt in the interface

struct fnt_data {
	float height;		//in points
	float line_height;
	GLuint * textures;	//list of textures
	GLuint list_base;	//beginning texture id
};

float
Fnt_Size(Fnt * fnt)
{
	return GLYPH_SPACING;// + 1 / GLYPH_SPACING;
}

float
Fnt_LineHeight(Fnt * fnt)
{
	return fnt->line_height;
}


/**********************************************************************
 * Fnt_MakeGlyphTexture
 * 
 * creates an OpenGL texture from a character glyph
 **********************************************************************/
 
 static 
 void 
 Fnt_MakeGlyphTexture(FT_Bitmap * bitmap, GLuint textureID, GLuint listID)
 {
	int w = bitmap->width;
	int h = bitmap->rows;
	//get the appropriate size for the texture (powers of 2)
	int w_2 = NextP2(w);
	int h_2 = NextP2(h);

	int i, j;

	//allocate memory for the texture data.
	GLubyte * expanded_data = (GLubyte *)malloc(sizeof(GLubyte) * 2 * w_2 * h_2);

	/*
	 * Here we fill in the data for the expanded bitmap.
	 * Notice that we are using two channel bitmap (one for luminocity 
	 * and one for alpha), but we assign both luminocity and alpha 
	 * to the value that we find in the FreeType bitmap. 
	 * The value will be 0 if we are in the padding zone, 
	 * and whatever is in the Freetype bitmap otherwise.
	 */
	for(j = 0; j < h_2; ++j) {
		for(i = 0; i < w_2; ++i) {
			expanded_data[2 * (i + j * w_2)] = 255;
			expanded_data[2 * (i + j * w_2) + 1] = 
				(i >= w || j >= h) ? 0 : bitmap->buffer[i + w * j];
		}
	}

	//setup some texture paramaters.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	//create the texture itself
	//notice GL_LUMINANCE_ALPHA to indicate 2 channel data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w_2, h_2, 
		0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

	//have texture, no need for expanded_data
	free(expanded_data);
 }
 
 
 /**********************************************************************
 * Fnt_CreateGlyph
 * 
 * creates the glyph from the given fnt face and character
 **********************************************************************/
 
 static 
 FT_Glyph 
 Fnt_CreateGlyph(FT_Face face, char ch)
 {
	FT_Glyph glyph;
	unsigned long ci = FT_Get_Char_Index(face, ch);
	
	//load the glyph for the character
	if(FT_Load_Glyph(face, ci, FT_LOAD_RENDER)) {
		printf("FT_Load_Glyph failed");
		exit(1);
	}

	//move the face's glyph into a Glyph object.
	if(FT_Get_Glyph(face->glyph, &glyph)) {
		printf("FT_Get_Glyph failed");
		exit(1);
	}
	
	return glyph;
 }

/**********************************************************************
 * Fnt_MakeDisplayList
 * 
 * creates an OpenGL display list correspending to the passed in char
 **********************************************************************/

static 
void 
Fnt_MakeDisplayList(Fnt * fnt, FT_Face face, char ch, GLuint list_base, GLuint * tex_base)
{
	GLuint textureID = tex_base[(int)ch];
	GLuint listID = list_base + (GLuint)ch;
	float x, y;

	FT_Glyph glyph = Fnt_CreateGlyph(face, ch);
	FT_BitmapGlyph bitmap_glyph;
	FT_Bitmap * bitmap;

	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	bitmap_glyph = (FT_BitmapGlyph)glyph;
	bitmap = &bitmap_glyph->bitmap;
	
	Fnt_MakeGlyphTexture(bitmap, textureID, listID);
	
	glNewList(listID, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glPushMatrix();
			/*
			 * Places the character horizontally (i.e. centers the character image
			 * in the bounding box. The bounding box is placed correctly, but the
			 * actual character, for it to look right, must be moved into the center,
			 * since some characters take up different amounts of space or are
			 * placed/positioned differently in that bounding box.)
			 *
			 * Also places the character vertically, for similar reasons,
			 * namely that of the ascenders and descender.
			 */
			glTranslatef((float)bitmap_glyph->left, (float)(bitmap_glyph->top - bitmap->rows), 0);

			//take into account any empty padding space for the texture
			x = (float)bitmap->width / (float)NextP2(bitmap->width);
			y = (float)bitmap->rows / (float)NextP2(bitmap->rows);

			//draw the texture mapped quads; orient the FreeType bitmap properly
			glBegin(GL_QUADS);
				glTexCoord2d(0, 0); glVertex2i(0, bitmap->rows);
				glTexCoord2d(0, y); glVertex2i(0, 0);
				glTexCoord2d(x, y); glVertex2i(bitmap->width, 0);
				glTexCoord2d(x, 0); glVertex2i(bitmap->width, bitmap->rows);
			glEnd();
		glPopMatrix();
		
		//glTranslatef((float)(face->glyph->advance.x >> 6) , 0, 0);
		//NOTE: must move an integer amount, otherwise weird artifacts
		//should also be constant for monospace fnt
		glTranslatef(GLYPH_SPACING, 0, 0);

	glEndList();
	
	FT_Done_Glyph(glyph);
}


/**********************************************************************
 * Fnt_Init
 * 
 * creates a fnt from the specified fnt name and height
 **********************************************************************/

Fnt* 
Fnt_Init(const char * fname, unsigned int height, float line_height) 
{
	FT_Library library;
	FT_Face face;
	unsigned char ch;

	Fnt * fnt = (Fnt *)malloc(sizeof(Fnt));
	
	fnt->textures = (GLuint *)malloc(sizeof(GLuint) * NUM_CHARS);
	fnt->height = (float)height;
	fnt->line_height = (float)fnt->height * line_height;

	if (FT_Init_FreeType(&library)) { 
		printf("FT_Init_FreeType failed. Something's wrong with FreeType");
		exit(1);
	}
	if (FT_New_Face(library, fname, 0, &face )) {
		printf("FT_New_Face failed. Couldn't load %s", fname);
		exit(1);
	}

	//FreeType measures fnt size in terms of 1/64ths of pixels.
	//Thus, to make a fnt h pixels high, we need to request a size of h*64.
	// (h << 6 is just a prettier way of writting h*64)
	FT_Set_Char_Size(face, height << 6, height << 6, 96, 96);

	fnt->list_base = glGenLists(NUM_CHARS);
	glGenTextures(NUM_CHARS, fnt->textures );

	//This is where we actually create each of the fnts display lists.
	for(ch = 0; ch < NUM_CHARS; ++ch) {
		Fnt_MakeDisplayList(fnt, face, ch, fnt->list_base, fnt->textures);
	}

	//display lists are created; don't need the fnt face or library
	FT_Done_Face(face);
	FT_Done_FreeType(library);
	
	return fnt;
}


/**********************************************************************
 * Fnt_Destroy
 * 
 * destroys and cleans up the specified fnt
 **********************************************************************/
 
void 
Fnt_Destroy(Fnt * fnt) 
{
	//list_base
	glDeleteLists(fnt->list_base, NUM_CHARS);
	//textures
	glDeleteTextures(NUM_CHARS, fnt->textures);
	free(fnt->textures);
	//height
	fnt->height = 0;
	//the thing itself
	free(fnt);
	fnt = NULL;
}


/**********************************************************************
 * Fnt_Print
 * 
 * the print function in all its glory; prints using the fnt
 **********************************************************************/

void
Fnt_Print(Fnt * fnt, Frame * frm, int x, int y, int max_lines, int show_cursor)
{
	GLuint flist = fnt->list_base;
	float h = fnt->line_height;
	float modelview_matrix[16];
	Line * cur_line;
	int line = 0;
	int len = 0;

	//print using screen coords
	PushScreenCoordMat();

	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	glListBase(flist);

	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
	
	glPushMatrix();
	
	Frame_IterEnd(frm);
	while(line < max_lines && (cur_line = Frame_IterPrev(frm))) {
		len = Line_Length(cur_line);
		glLoadIdentity();
		glTranslatef((float)x, (float)y + h*line, 0);
		glMultMatrixf(modelview_matrix);
		glCallLists(len, GL_UNSIGNED_BYTE, Line_Text(cur_line));
		++line;
	}
	
	glPopMatrix();
	
	glPopAttrib();
	
	/*********************************
	 * display cursor
	 *********************************/
	
	if(show_cursor) {
		Frame_IterEnd(frm);
		cur_line = Frame_IterPrev(frm);
		len = Line_Length(cur_line);
		glPushMatrix();
			//glColor3f(0.2f, 0.2f, 0.2f);
			glLoadIdentity();
			glTranslatef((float)x + len*GLYPH_SPACING, (float)y, 0);
			glMultMatrixf(modelview_matrix);
			glBegin(GL_LINES);
				glVertex2i(0, 0);
				glVertex2i(GLYPH_SPACING, 0);
			glEnd();
		glPopMatrix();
	}

	//go back to world coords
	PopScreenCoordMat();
} 

