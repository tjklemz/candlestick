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
 
// This class is really an "App Delegate" that simply tells the other
// classes what to do.

#include "app.h"
#include "disp.h"
#include "line.h"
#include "frame.h"
#include "utf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

typedef void (*fullscreen_del_func_t)(void);
typedef void (*quit_del_func_t)(void);

typedef enum {
	CS_TYPING,
	CS_SAVING
} cs_app_state_t;

static Frame * frm = 0;
static Line * filename = 0;
static anim_del_t * anim_del = 0;
static fullscreen_del_func_t fullscreen_del = 0;
static quit_del_func_t quit_del = 0;
static cs_app_state_t app_state = CS_TYPING;

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
	
	app_state = CS_TYPING;
}

void
App_OnDestroy()
{
	free(anim_del);
	anim_del = 0;
	
	Line_Destroy(filename);
	
	Disp_Destroy();
	
	Frame_Destroy(frm);
	frm = 0;
}

void
App_OnResize(int w, int h)
{
	Disp_Resize(w, h);
}

void
App_OnRender()
{
	Disp_BeginRender();
	
	switch(app_state) {
	case CS_TYPING:
		Disp_TypingScreen(frm);
		break;
	case CS_SAVING:
		Disp_SaveScreen(Line_Text(filename));
		break;
	}
}

void
App_OnSpecialKeyUp(cs_key_t key, cs_key_mod_t mods)
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
App_OnSpecialKeyDown(cs_key_t key, cs_key_mod_t mods)
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
	switch(*ch) {
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
	//ignore the carriage return
	case '\r':
		break;
	/* Handle just newlines.
	 * This is the default behaviour of Mac/Unix (that is, '\n'),
	 * but Windows still uses '\r\n'. So, just ignore the '\r'.
	 * Let Frame handle any logic regarding newlines.
	 * For instance, could possibly do Unicode line endings (LS and PS).
	 * For now, though, just do Unix style line endings (line feeds, '\n').
	 */
	case '\n':
		Frame_InsertNewLine(frm);
		break;
	default:
		Frame_InsertCh(frm, ch);
		break;
	}
}

/**************************************************************************
 * File management
 **************************************************************************/

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
	
	//get the file size
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
}

/*static
void
App_Open(const char * the_filename)
{
	FILE * file;
	
	printf("Opening file: %s\n", the_filename);
	file = fopen(the_filename, "r");
	
	App_Read(file);
	
	fclose(file);
	printf("...Done.\n");
	
	App_SaveFilename(the_filename);
	
	Disp_ScrollReset();
}*/

static
void
App_Save()
{
	FILE * file;
	
	printf("Saving to file: %s\n", Line_Text(filename));
	file = fopen(Line_Text(filename), "w");
	
	Frame_Write(frm, file);
	
	fclose(file);
	printf("...Done.\n");
}

static
int
App_SaveAs()
{
	if(!Line_Text(filename)) {
		fprintf(stderr, "Can't save an empty filename!\n");
		return 0;
	}
	
	//tack on the .txt extension
	Line_InsertStr(filename, ".txt");
	
	App_Save();
	
	return 1; //everything went ok
}

static
void
App_OnCharSave(char * ch)
{
	switch(*ch) {
	case '\n':
	case '\r':
		if(App_SaveAs()) {
			app_state = CS_TYPING;
		}
		break;
	default:
		Line_InsertCh(filename, ch);
		break;
	}
}

void
App_OnKeyDown(char * key, cs_key_mod_t mods)
{
	Disp_ScrollReset();
	
	if(MODS_COMMAND(mods)) {
		switch(*key) {
		case 'f':
			printf("fullscreen command combo...\n");
			if(fullscreen_del) {
				fullscreen_del();
			}
			break;
		case 'o':
			printf("open command combo...\n");
			break;
		case 'q':
			printf("quit command combo...\n");
			if(quit_del) {
				quit_del();
			}
			break;
		case 's':
			printf("save command combo...\n");
			if(!filename) {
				filename = Line_Init(CHARS_PER_LINE);
				app_state = CS_SAVING;
			}
			break;
		default:
			printf("invalid command combo...\n");
			break;
		}
	} else {
		if(app_state == CS_TYPING) {
			App_OnChar(key);
		} else if(app_state == CS_SAVING) {
			App_OnCharSave(key);
		}
	}
}

/**************************************************************************
 * Delegates
 **************************************************************************/

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
	
	printf("Animation delegates set.\n");
}

void
App_FullscreenDel(void (*ToggleFullscreen)(void))
{
	fullscreen_del = ToggleFullscreen;
	printf("Fullscreen delegates set.\n");
}

void
App_QuitRequestDel(void (*Quit)(void))
{
	quit_del = Quit;
	printf("Quit delegates set.\n");
}
