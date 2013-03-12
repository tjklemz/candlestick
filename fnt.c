/**********************************************************************
 * fnt.c
 * 
 * A simple opengl fnt library that uses GNU freetype2
 * 
 * Copyright 2013 Thomas Klemz
 **********************************************************************/

#include "fnt.h"
#include <math.h>

#define GLYPH_SPACING 12
#define NUM_CHARS 255

//defined as Fnt in the interface

struct fnt_data {
	float height;		//in points
	GLuint * textures;	//list of textures
	GLuint list_base;	//beginning texture id
};


/**********************************************************************
 * nextP2
 * 
 * gets the first power of 2 that is greater than or equal to
 * the passed in integer
 * 
 * Should probably be in a utilities file
 **********************************************************************/

static int NextP2(int a)
{
	int rval = 1;
	while (rval < a) rval <<= 1;
	return rval;
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
 Fnt_CreateGlyph(FT_Face face, unsigned char ch)
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
Fnt_MakeDisplayList(FT_Face face, unsigned char ch, GLuint list_base, GLuint * tex_base)
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
}


/**********************************************************************
 * PushScreenCoordMat
 * 
 * makes object world coords identical to window coords
 * the new projection matrix is left on the stack
 **********************************************************************/

static 
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

static 
void 
PopScreenCoordMat() 
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}


/**********************************************************************
 * Fnt_Init
 * 
 * creates a fnt from the specified fnt name and height
 **********************************************************************/

Fnt* 
Fnt_Init(const char * fname, unsigned int height) 
{
	FT_Library library;
	FT_Face face;
	unsigned char ch;

	Fnt * fnt = (Fnt *)malloc(sizeof(Fnt));
	
	fnt->textures = (GLuint *)malloc(sizeof(GLuint) * NUM_CHARS);
	fnt->height = (float)height;

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
		Fnt_MakeDisplayList(face, ch, fnt->list_base, fnt->textures);
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
Fnt_Print(Fnt * fnt, Frame * frm, int y)
{
	static int x = 320;
	GLuint flist = fnt->list_base;
	//make the line height bigger than the fnt so space between lines
	float h = fnt->height / .63f;
	float modelview_matrix[16];
	Line * cur_line;
	int line = 1;
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

	Frame_IterBegin(frm);
	
	while((cur_line = Frame_IterNext(frm))) {
		len = Line_Length(cur_line);
		glPushMatrix();
			glLoadIdentity();
			glTranslatef((float)x, (float)y-h*line, 0);
			glMultMatrixf(modelview_matrix);
			glCallLists(len, GL_UNSIGNED_BYTE, Line_Text(cur_line));
		glPopMatrix();
		++line;
	}

	glPopAttrib();
	
	//display cursor
	glPushMatrix();
		glLoadIdentity();
		glTranslatef((float)x + len*GLYPH_SPACING, (float)y-h*(line-1), 0);
		glMultMatrixf(modelview_matrix);
		glBegin(GL_LINES);
			glVertex2i(0, 0);
			glVertex2i(GLYPH_SPACING, 0);
		glEnd();
	glPopMatrix();

	//go back to world coords
	PopScreenCoordMat();
} 

