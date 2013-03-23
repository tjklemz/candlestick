/*************************************************************************
 * app.c
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

#include "app.h"
#include "disp.h"
#include "frame.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//This class is really an "App Delegate" that simply tells the other
// classes what to do.

static Frame * frm = 0;
static char * _filename = 0;

void
App_OnInit()
{	
	//checks if already initialized; if so, don't recreate the frame
	//using this for the linux app; should probably refactor so that
	// these funcs can be re-called (kinda like malloc vs realloc)
	//OR, could refactor so that the Display module has a pointer struct
	// that way can create and destroy the Display easier...
	//This simple check is fine for now, but if it gets any more
	// complicated, definitely need to refactor.
	//c.f. main.c on the linux fullscreen issue that caused this mess.
	if(!frm) {
		frm = Frame_Init(CHARS_PER_LINE);
	} else {
		Disp_Destroy();
	}
	
	Disp_Init(FONT_SIZE);
}

void
App_OnDestroy()
{
	free(_filename);
	Disp_Destroy();
	Frame_Destroy(frm);
}

void
App_OnResize(int w, int h)
{
	Disp_Resize(w, h);
}

void
App_OnRender()
{
	Disp_Render(frm);
}

void
App_OnKeyDown(unsigned char key)
{
	switch(key)
	{
	case '\t':
		Frame_InsertTab(frm);
		break;
	//actual delete
	case 239:
	//backspace
	case 127:
	case '\b':
		Frame_DeleteCh(frm);
		break;
	case '\r':
	case '\n':
		Frame_InsertNewLine(frm);
		break;
	default:
		//supports Latin-1 set
		//only non-printing character >= space (32) is DEL (127)
		//	which is taken care of already
		if(key >= 32) {
			Frame_InsertCh(frm, key);
		}
		break;
	}
}

//The App module should keep track of the File state
// but delegate to other modules to handle the File?

static
void
App_SaveFilename(const char * filename)
{
	free(_filename);
	_filename = (char *)malloc(strlen(filename) + 1);
	strcpy(_filename, filename);
}

static
void
App_Read(FILE * file)
{
	char c;
	
	//empty the current frame so can fill it
	Frame_Destroy(frm);
	frm = Frame_Init(CHARS_PER_LINE);
	
	while((c = fgetc(file)) != EOF) {
		App_OnKeyDown(c);
	}
}

void
App_Open(const char * filename)
{
	FILE * file;
	
	printf("Opening file: %s\n", filename);
	
	file = fopen(filename, "r");
	
	App_Read(file);
	
	fclose(file);
	
	printf("...Done.\n");
	
	App_SaveFilename(filename);
}

void
App_SaveAs(const char * filename)
{
	FILE * file;
	
	printf("Saving to file: %s\n", filename);
	
	file = fopen(filename, "w");
	
	Frame_Write(frm, file);
	
	fclose(file);
	
	printf("...Done.\n");
	
	App_SaveFilename(filename);
}

void
App_Save()
{
	assert(_filename != 0);
	
	App_SaveAs(_filename);
}

void
App_Reload()
{
	assert(_filename != 0);
	
	App_Open(_filename);
}