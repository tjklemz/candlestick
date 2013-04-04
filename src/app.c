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
#include "utf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

//This class is really an "App Delegate" that simply tells the other
// classes what to do.

static Frame * frm = 0;
static char * _filename = 0;
static anim_del_t * anim_del = 0;

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
		frm = Frame_Init();
	} else {
		Disp_Destroy();
	}
	
	Disp_Init(FONT_SIZE);
}

void
App_OnDestroy()
{
	free(anim_del);
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
App_OnSpecialKeyUp(cs_key_t key)
{
	switch(key) {
	case CS_ARROW_UP:
	case CS_ARROW_DOWN:
		Disp_ScrollStopRequested();
		break;
	default:
		break;
	}
}

void
App_OnSpecialKeyDown(cs_key_t key)
{
	switch(key) {
	case CS_ARROW_UP:
		Disp_ScrollUpRequested();
		break;
	case CS_ARROW_DOWN:
		Disp_ScrollDownRequested();
		break;
	default:
		break;
	}
}

static
void
App_OnChar(char * ch)
{
	switch(ch[0]) {
	case '\t':
		Frame_InsertTab(frm);
		break;
	//actual delete
	//case 239:
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
		Frame_InsertCh(frm, ch);
		break;
	}
}

void
App_OnKeyDown(char * key)
{
	Disp_ScrollReset();
	App_OnChar(key);
}

void
App_AnimationDel(void (*OnStart)(void), void (*OnEnd)(void))
{
	assert(anim_del == 0);
	
	anim_del = (anim_del_t *)malloc(sizeof(anim_del_t));
	
	anim_del->on_start = OnStart;
	anim_del->called_start = 0;
	anim_del->on_end = OnEnd;
	anim_del->called_end = 0;
	
	Disp_AnimationDel(anim_del);
	
	printf("Delegates set.\n");
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
	char * buffer;
	char ch[7];
	long len;
	size_t result;
	long i;
	Rune rune;
	int size;
	
	//empty the current frame so we can fill it
	Frame_Destroy(frm);
	frm = Frame_Init(CHARS_PER_LINE);
	
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	rewind(file);
	
	buffer = (char *)malloc(sizeof(char) * len);
	if (!buffer) {
		fputs("Memory error for App_Read", stderr); 
		exit(2);
	}
	
	printf("Got mem, now reading...\n");
	
	result = fread(buffer, 1, len, file);
	if(result != len) {
		fputs("Reading error for App_Read", stderr);
		exit(3);
	}
	
	printf("Read into mem, now filling the frame with len: %ld\n", len);
	
	for(i = 0; i < len; i += size) {
		chartorune(&rune, &buffer[i]);
		
		if(rune == Runeerror) {
			fputs("Bad things happened! Error parsing file as UTF-8.", stderr);
			break;
		}
		
		size = runetochar(ch, &rune);
		ch[size] = '\0';
		
		printf("Got char of size: %d\n", size);
		
		App_OnChar(ch);
		if(i % 4096 == 0) {
			printf("Filled %ld bytes...\n", i);
		}
	}
	
	free(buffer);
	
	/*while((c = fgetc(file)) != EOF) {
		App_OnChar(c);
	}*/
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
	
	Disp_ScrollReset();
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
