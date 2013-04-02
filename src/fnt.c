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

/*
 * A very simple font cache and rasterizer that uses freetype
 * to draw fonts from a single OpenGL texture. The code uses
 * a linear-probe hashtable, and writes new glyphs into
 * the texture using glTexSubImage2D. When the texture fills
 * up, or the hash table gets too crowded, everything is wiped.
 *
 * This is designed to be used for horizontal text only,
 * and draws unhinted text with subpixel accurate metrics
 * and kerning. As such, you should always call the drawing
 * function with an identity transform that maps units
 * to pixels accurately.
 *
 * If you wish to use it to draw arbitrarily transformed
 * text, change the min and mag filters to GL_LINEAR and
 * add a pixel of padding between glyphs and rows, and
 * make sure to clear the texture when wiping the cache.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fnt.h"
#include "opengl.h"
#include "utf.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
/*#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "freetype/ftoutln.h"
#include "freetype/fttrigon.h"
#include "freetype/ftadvanc.h" */

#define PADDING 1		/* set to 0 to save some space but disallow arbitrary transforms */

#define MAXGLYPHS 4093	/* prime number for hash table goodness */
#define CACHESIZE 256
#define XPRECISION 4
#define YPRECISION 1

static inline void die(char *msg)
{
	fprintf(stderr, "error: %s\n", msg);
	exit(1);
}

struct fnt_t
{
	float size;
	FT_Face face;
	float line_height;
};

struct key
{
	FT_Face face;
	short size;
	short gid;
	short subx;
	short suby;
};

struct glyph
{
	char lsb, top, w, h;
	short s, t;
	float advance;
};

struct table
{
	struct key key;
	struct glyph glyph;
};

static FT_Library g_freetype_lib = NULL;
static struct table g_table[MAXGLYPHS];
static int g_table_load = 0;
static unsigned int g_cache_tex = 0;
static int g_cache_w = CACHESIZE;
static int g_cache_h = CACHESIZE;
static int g_cache_row_y = 0;
static int g_cache_row_x = 0;
static int g_cache_row_h = 0;

static void init_font_cache(void)
{
	int code;

	code = FT_Init_FreeType(&g_freetype_lib);
	if (code)
		die("cannot initialize freetype");

	glGenTextures(1, &g_cache_tex);
	glBindTexture(GL_TEXTURE_2D, g_cache_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g_cache_w, g_cache_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, NULL);
}

static void clear_font_cache(void)
{
#if PADDING > 0
	unsigned char *zero = malloc(g_cache_w * g_cache_h);
	memset(zero, 0, g_cache_w * g_cache_h);
	glBindTexture(GL_TEXTURE_2D, g_cache_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_cache_w, g_cache_h, GL_ALPHA, GL_UNSIGNED_BYTE, zero);
	free(zero);
#endif

	memset(g_table, 0, sizeof(g_table));
	g_table_load = 0;

	g_cache_row_y = PADDING;
	g_cache_row_x = PADDING;
	g_cache_row_h = 0;
}

static FT_Face load_font(const char *fontname)
{
	FT_Face face;
	int code;

	if (g_freetype_lib == NULL)
	{
		init_font_cache();
		clear_font_cache();
	}

	code = FT_New_Face(g_freetype_lib, fontname, 0, &face);
	if (code)
		die("cannot load font file");

	FT_Select_Charmap(face, ft_encoding_unicode);

	return face;
}

static void free_font(FT_Face face)
{
	clear_font_cache();
	FT_Done_Face(face);
}

static unsigned int hashfunc(struct key *key)
{
	unsigned char *buf = (unsigned char *)key;
	unsigned int len = sizeof(struct key);
	unsigned int h = 0;
	while (len--)
		h = *buf++ + (h << 6) + (h << 16) - h;
	return h;
}

static unsigned int lookup_table(struct key *key)
{
	unsigned int pos = hashfunc(key) % MAXGLYPHS;
	while (1)
	{
		if (!g_table[pos].key.face) /* empty slot */
			return pos;
		if (!memcmp(key, &g_table[pos].key, sizeof(struct key))) /* matching slot */
			return pos;
		pos = (pos + 1) % MAXGLYPHS;
	}
}

static struct glyph * lookup_glyph(FT_Face face, int size, int gid, int subx, int suby)
{
	FT_Vector subv;
	struct key key;
	unsigned int pos;
	int code;
	int w, h;

	/*
	 * Look it up in the table
	 */

	key.face = face;
	key.size = size;
	key.gid = gid;
	key.subx = subx;
	key.suby = suby;

	pos = lookup_table(&key);
	if (g_table[pos].key.face)
		return &g_table[pos].glyph;

	/*
	 * Render the bitmap
	 */

	glEnd();

	subv.x = subx;
	subv.y = suby;

	FT_Set_Transform(face, NULL, &subv);

	code = FT_Load_Glyph(face, gid, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING);
	if (code < 0)
		return NULL;

	code = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_LIGHT);
	if (code < 0)
		return NULL;

	w = face->glyph->bitmap.width;
	h = face->glyph->bitmap.rows;

	/*
	 * Find an empty slot in the texture
	 */

	if (g_table_load == (MAXGLYPHS * 3) / 4)
	{
		puts("font cache table full, clearing cache");
		clear_font_cache();
		pos = lookup_table(&key);
	}

	if (h + PADDING > g_cache_h || w + PADDING > g_cache_w)
		die("rendered glyph exceeds cache dimensions");

	if (g_cache_row_x + w + PADDING > g_cache_w)
	{
		g_cache_row_y += g_cache_row_h + PADDING;
		g_cache_row_x = PADDING;
	}
	if (g_cache_row_y + h + PADDING > g_cache_h)
	{
		puts("font cache texture full, clearing cache");
		clear_font_cache();
		pos = lookup_table(&key);
	}

	/*
	 * Copy bitmap into texture
	 */

	memcpy(&g_table[pos].key, &key, sizeof(struct key));
	g_table[pos].glyph.w = face->glyph->bitmap.width;
	g_table[pos].glyph.h = face->glyph->bitmap.rows;
	g_table[pos].glyph.lsb = face->glyph->bitmap_left;
	g_table[pos].glyph.top = face->glyph->bitmap_top;
	g_table[pos].glyph.s = g_cache_row_x;
	g_table[pos].glyph.t = g_cache_row_y;
	g_table[pos].glyph.advance = face->glyph->advance.x / 64.0;
	g_table_load ++;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, face->glyph->bitmap.pitch);
	glTexSubImage2D(GL_TEXTURE_2D, 0, g_cache_row_x, g_cache_row_y, w, h,
			GL_ALPHA, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glBegin(GL_QUADS);

	g_cache_row_x += w + PADDING;
	if (g_cache_row_h < h + PADDING)
		g_cache_row_h = h + PADDING;

	return &g_table[pos].glyph;
}

static float draw_glyph(FT_Face face, int size, int gid, float x, float y)
{
	struct glyph *glyph;
	int subx = (x - floor(x)) * XPRECISION;
	int suby = (y - floor(y)) * YPRECISION;
	subx = (subx * 64) / XPRECISION;
	suby = (suby * 64) / YPRECISION;

	glyph = lookup_glyph(face, size, gid, subx, suby);
	if (!glyph)
		return 0.0;

	float s0 = (float) glyph->s / g_cache_w;
	float t0 = (float) glyph->t / g_cache_h;
	float s1 = (float) (glyph->s + glyph->w) / g_cache_w;
	float t1 = (float) (glyph->t + glyph->h) / g_cache_h;
	float xc = floor(x) + glyph->lsb;
	float yc = floor(y) - glyph->top + glyph->h;

	glTexCoord2f(s0, t0); glVertex2f(xc, yc - glyph->h);
	glTexCoord2f(s1, t0); glVertex2f(xc + glyph->w, yc - glyph->h);
	glTexCoord2f(s1, t1); glVertex2f(xc + glyph->w, yc);
	glTexCoord2f(s0, t1); glVertex2f(xc, yc);

	return glyph->advance;
}

/*static float measure_string(FT_Face face, float fsize, char *str)
{
	int size = fsize * 64;
	FT_Fixed advance;
	FT_Vector kern;
	Rune ucs, gid;
	float w = 0.0;
	int left = 0;

	FT_Set_Char_Size(face, size, size, 72, 72);

	while (*str)
	{
		str += chartorune(&ucs, str);
		gid = FT_Get_Char_Index(face, ucs);
		FT_Get_Advance(face, gid, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING, &advance);
		w += advance / 65536.0;
		FT_Get_Kerning(face, left, gid, FT_KERNING_UNFITTED, &kern);
		w += kern.x / 64.0;
		left = gid;
	}

	return w;
}*/

static float draw_string(FT_Face face, float fsize, float x, float y, char *str, int len)
{
	int size = fsize * 64;
	FT_Vector kern;
	Rune ucs, gid;
	int left = 0;
	//temporary; see NOTE below
	int i;

	FT_Set_Char_Size(face, size, size, 72, 72);

	glBindTexture(GL_TEXTURE_2D, g_cache_tex);
	glBegin(GL_QUADS);

	//NOTE: temporary; will change back to while loop; need to refactor Frame
	//while(*str)
	for(i = 0; i < len; ++i)
	{
		str += chartorune(&ucs, str);
		gid = FT_Get_Char_Index(face, ucs);
		x += draw_glyph(face, size, gid, x, y);
		FT_Get_Kerning(face, left, gid, FT_KERNING_UNFITTED, &kern);
		x += kern.x / 64.0;
		left = gid;
		//printf("(x, y) is... (%f, %f)\n", x, y);
	}

	glEnd();

	return x;
}

/**********************************************************************
 * return the font size in points
 **********************************************************************/
float
Fnt_Size(Fnt * fnt)
{
	return fnt->size;
}

/**********************************************************************
 * returns the line height
 **********************************************************************/
float
Fnt_LineHeight(Fnt * fnt)
{
	return fnt->line_height;
}

/**********************************************************************
 * creates a fnt with a given name and height (in points)
 **********************************************************************/
Fnt*
Fnt_Init(const char * fname, unsigned int size, float line_height)
{
	Fnt * fnt = (Fnt *)malloc(sizeof(Fnt));
	
	fnt->size = size;
	fnt->line_height = line_height;
	fnt->face = load_font(fname);
	
	return fnt;
}

/**********************************************************************
 * destroys the fnt (destructor)
 **********************************************************************/
void
Fnt_Destroy(Fnt * fnt)
{
	free_font(fnt->face);
	free(fnt);
	fnt = 0;
}

/**********************************************************************
 * prints text at window coords (x,y) using the fnt
 **********************************************************************/

void
Fnt_Print(Fnt * fnt, Frame * frm, int x, int y, int max_lines, int show_cursor)
{
	float modelview_matrix[16];
	
	Line * cur_line;
	int line = 0;
	int len = 0;
	float h = fnt->line_height;
	FT_Face f = fnt->face;
	float s = fnt->size;
	
	char * bob = "Hi, bob";
	
	//print using screen coords
	PushScreenCoordMat();

	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	//glListBase(flist);

	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
	
	glPushMatrix();
	
	/*Frame_IterEnd(frm);
	while(line < max_lines && (cur_line = Frame_IterPrev(frm))) {
		len = Line_Length(cur_line);
		
		draw_string(f, s, (float)x, (float)y + h*line, Line_Text(cur_line), len);
		
		++line;
	}*/
	//printf("(x, y) is... (%d, %d)\n", x, y);
	draw_string(f, s, (float)x, (float)y, bob, strlen(bob));
	
	glPopMatrix();
	
	glPopAttrib();
	
	//go back to world coords
	PopScreenCoordMat();
}
