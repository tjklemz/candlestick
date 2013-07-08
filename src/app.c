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
#include "list.h"
#include "scroll.h"
#include "files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>
 

typedef void (*fullscreen_del_func_t)(void);
typedef void (*quit_del_func_t)(void);
typedef void (*update_title_del_func_t)(char*);

typedef enum {
	CS_TYPING,
	CS_SAVING,
	CS_OPENING
} cs_app_state_t;

static Frame * frm = 0;
static Line * filename = 0;
static Line * filename_buf = 0;

static files_t * files = 0; 

static update_title_del_func_t update_title_del = 0;

static anim_del_t * scroll_anim_del = 0;
static anim_del_t * disp_anim_del = 0;
static fullscreen_del_func_t fullscreen_del = 0;
static int is_fullscreen = 0;
static quit_del_func_t quit_del = 0;

static int save_err = 0;

static cs_app_state_t app_state = CS_TYPING;
static scrolling_t open_scroll = {0};
static scrolling_t text_scroll = {0};
static scrolling_t * cur_scroll = 0;


static
int
App_Save();


static
void
App_UpdateTitle(int dirty)
{
	static int old_state = 0;

	if(dirty && old_state == dirty) {
		//nothing to do here
	} else if(update_title_del) {
		char buf[512]; //8 * 64 = 512
		char * title = filename ? Line_Text(filename) : "untitled";

		if(dirty) {
			sprintf(buf, "*%s", title);
		} else {
			strcpy(buf, title);
		}

		update_title_del(buf);
	}

	old_state = dirty;
}

void
App_OnInit()
{	
	/* 
	 * Linux requires that a new window be created when going
	 * to fullscreen. This code makes sure that the frame doesn't
	 * get destroyed/recreated on a re-entry.
	 */
	if(!frm) {
		frm = Frame_Init();
		text_scroll.on_update = Scroll_TextScroll;
		open_scroll.on_update = Scroll_OpenScroll;

		App_UpdateTitle(1);

		app_state = CS_TYPING;
		cur_scroll = &text_scroll;
	} else {
		Disp_Destroy();
		text_scroll.requested = 0;
		open_scroll.requested = 0;
	}
	
	Disp_Init(FONT_SIZE);
}


void
App_OnDestroy()
{
	// attempt to autosave before quiting
	if(filename && Line_Text(filename)) {
		App_Save();
	}

	Anim_Destroy(scroll_anim_del);
	scroll_anim_del = 0;
	Anim_Destroy(disp_anim_del);
	disp_anim_del = 0;
	
	Line_Destroy(filename);
	filename = 0;
	
	//filename_buf and filename should never point to the same thing,
	//so this is a valid delete.
	Line_Destroy(filename_buf);
	filename_buf = 0;
	
	Files_Destroy(files);
	
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
		Disp_TypingScreen(frm, &text_scroll);
		break;
	case CS_SAVING:
		Disp_SaveScreen(Line_Text(filename_buf), save_err);
		break;
	case CS_OPENING:
		Disp_OpenScreen(files, &open_scroll);
		break;
	default:
		break;
	}
}


/**************************************************************************
 * OnUpdate
 *
 * This is called by another thread.
 *
 * Be careful what happens inside this function as it is not thread safe.
 * Updating the scroll var (cur_scroll) should be fine as nothing else
 * currently updates it (only reads from it).
 **************************************************************************/

void
App_OnUpdate()
{
	if(cur_scroll) {
		Scroll_Update(cur_scroll);
	}

	// doesn't display, just updates (a clue that this should be extracted)
	Disp_UpdateAnim();
}


void
App_OnSpecialKeyUp(cs_key_t key, cs_key_mod_t mods)
{
	switch(key) {
	case CS_ARROW_UP:
	case CS_ARROW_DOWN:
		if(cur_scroll) {
			Scroll_StopRequested(cur_scroll);
		}
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
	case CS_ARROW_DOWN:
		if(cur_scroll) {
			scrolling_dir_t dir = (key == CS_ARROW_UP) ? SCROLL_UP : SCROLL_DOWN;
			Scroll_Requested(cur_scroll, dir);
		}
		break;
	case CS_ESCAPE:
		if(app_state == CS_SAVING) {
			app_state = CS_TYPING;
			cur_scroll = &text_scroll;
			Line_Destroy(filename_buf);
			filename_buf = 0;
		} else if(app_state == CS_OPENING) {
			app_state = CS_TYPING;
			cur_scroll = &text_scroll;
		} else if(app_state == CS_TYPING) {
			if(is_fullscreen && fullscreen_del) {
				fullscreen_del();
				is_fullscreen = 0;
			}
		}
		break;
	default:
		break;
	}
}


static
void
App_OnChar(char * ch, Frame * frm)
{
	switch(*ch) {
	case '\t':
		Frame_InsertTab(frm);
		break;
	//delete
	case 127:
	//backspace
	case '\b':
		Frame_DeleteCh(frm);
		break;
	//ignore the carriage return
	case '\r':
		fprintf(stderr, "whoa! carriage returns are old school.\ntry the linefeed from now on. strlen: %d\n", (int)strlen(ch));
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
int
App_Read(FILE * file)
{
	char * buffer;
	char ch[7];
	long len;
	size_t result;
	long i;
	Rune rune;
	int size;
	int valid = 1;
	
	Frame * new_frm = Frame_Init(CHARS_PER_LINE);
	
	//get the file size
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	rewind(file);
	
	buffer = (char *)malloc(sizeof(char) * len);
	if (!buffer) {
		fputs("Memory error for App_Read", stderr); 
		return 0;
	}
	
	//printf("Got mem, now reading...\n");
	
	result = fread(buffer, 1, len, file);

	//printf("len: %ld result: %ld", len, result);

	if(result != len) {
		fputs("Reading error for App_Read", stderr);
		return 0;
	}
	
	//printf("Read into mem, now filling the frame with len: %ld\n", len);
	
	for(i = 0; i < len; i += size) {
		chartorune(&rune, &buffer[i]);
		
		if(rune == Runeerror) {
			fputs("Bad things happened! Error parsing file as UTF-8.", stderr);
			valid = 0;
			break;
		}
		
		size = runetochar(ch, &rune);
		ch[size] = '\0';
		
		//these print statements really slow down the read...
		//printf("Got char of size: %d\n", size);
		
		App_OnChar(ch, new_frm);
	}
	
	free(buffer);

	if(valid) {
		Frame_Destroy(frm);
		frm = new_frm;
	} else {
		Frame_Destroy(new_frm);
	}

	return valid;
}


/**************************************************************************
 * SaveFilename
 *
 * Expects the filename as "name.txt" (or whatever),
 * but nothing else (no DOCS_FOLDER or file path or anything like that)
 **************************************************************************/

static
void
App_SaveFilename(char * the_filename)
{
	Line_Destroy(filename);
	filename = Line_Init(strlen(the_filename) + 1);
	Line_InsertStr(filename, the_filename);

	App_UpdateTitle(0);
}


static
int
App_Open(char * the_filename)
{
	int opened = 0;
	FILE * file;
	char * full_filename;
	
	full_filename = Files_GetAbsPath(the_filename);
	
	printf("Opening file: %s\n", full_filename);
	file = fopen(full_filename, "rb");
	
	if(!file) {
		fprintf(stderr, "Could not open requested file!\n");
	} else {
		opened = App_Read(file);

		fclose(file);
		printf("...Done.\n");
	}
	
	free(full_filename);

	return opened;
}


static
int
App_Save()
{
	FILE * file;
	
	char * the_filename = Line_Text(filename);
	char * full_filename = Files_GetAbsPath(the_filename);
	
	Files_CheckDocDir();
	
	printf("Saving to file: %s\n", full_filename);
	file = fopen(full_filename, "wb");

	if(file) {
		Frame_Write(frm, file);
		fclose(file);
		free(full_filename);
		
		puts("...Done.");
		App_UpdateTitle(0);
		return 1;
	}

	// bad
	return 0;
}

static
Line*
App_CreateFileName()
{
	Line * file = 0;
	int len;

	len = strlen(Line_Text(filename_buf));

	if(len > 0) {
		file = Line_Init(len);
		//copy the buffer
		Line_InsertStr(file, Line_Text(filename_buf));
		//tack on the .txt extension
		Line_InsertStr(file, FILE_EXT);
	}

	return file;
}


static
int
App_TestSave()
{
	int valid = 0;
	Line * test = 0;

	test = App_CreateFileName();
	
	if(test) {
		char * test_filename = 0;
		char * full_test_filename = 0;

		//create a full filename from the copy
		test_filename = Line_Text(test);
		full_test_filename = Files_GetAbsPath(test_filename);

		valid = !Files_Exists(full_test_filename);
		
		free(full_test_filename);
		Line_Destroy(test);
	}
	
	return valid;
}


static
int
App_SaveAs()
{
	int saved = 0;

	Line * file = App_CreateFileName();
	
	if(file) {
		Line_Destroy(filename);
		filename = file;

		saved = App_Save();

		// don't destroy the buffer if something went wrong,
		// although we're kinda screwed...
		if(saved) {
			Line_Destroy(filename_buf);
			filename_buf = 0;
		}
	}

	return saved;
}


static
void
App_OnCharSave(char * ch)
{
	switch(*ch) {
	case '\t':
		break;
	case '\n':
	case '\r':
		save_err = !(App_TestSave() && App_SaveAs());
		if(!save_err) {
			app_state = CS_TYPING;
			cur_scroll = &text_scroll;
		} else {
			Disp_TriggerSaveErrAnim();
		}
		break;
	case 127:
	case '\b':
		Line_DeleteCh(filename_buf);
		break;
	default:
		if(utflen(Line_Text(filename_buf)) < MAX_FILE_CHARS) {
			Line_InsertCh(filename_buf, ch);
		}
		break;
	}

	if(app_state == CS_SAVING && filename_buf && Line_Text(filename_buf)) {
		save_err = !App_TestSave();
	}
}


static
void
App_OnCharOpen(char * ch)
{
	switch(*ch) {
	case '\n':
	case '\r':
	{
		double amt = open_scroll.amt;
		int file_num = (int)(open_scroll.dir == SCROLL_UP ? ceil(amt) : floor(amt));
		
		if(file_num < files->len) {
			char * filename = files->names[file_num];

			if(App_Open(filename)) {
				Scroll_Reset(&text_scroll);
				cur_scroll = &text_scroll;
				app_state = CS_TYPING;

				App_SaveFilename(filename);

				Files_Destroy(files);
			} else {
				fputs("Error on open...\n", stderr);
			}
		} else {
			// this should never happen so long as the scrolling logic stays withing
			// the scrolling bounds (i.e. scroll->limit remains valid)
			fprintf(stderr, "Requested open index is bad!");
			// just do nothing...
		}
		break;
	}
	default:
		break;
	}
}


void
App_OnKeyDown(char * ch, cs_key_mod_t mods)
{
	if(MODS_COMMAND(mods)) {
		switch(*ch) {
		case 'f':
			if(fullscreen_del) {
				fullscreen_del();
				is_fullscreen = !is_fullscreen;
			}
			break;
		case 'o':
			if(app_state == CS_TYPING) {
				app_state = CS_OPENING;
				Scroll_Reset(&open_scroll);

				Files_Destroy(files);
				files = Files_Populate();
				open_scroll.limit = files->len - 1;
				cur_scroll = &open_scroll;
			}
			break;
		case 'q':
			if(quit_del) {
				quit_del();
			}
			break;
		case 's':
			if(app_state == CS_TYPING) {
				if(!filename) {
					save_err = 0;
					app_state = CS_SAVING;
					cur_scroll = NULL;
					filename_buf = Line_Init(CHARS_PER_LINE);
				} else {
					if(App_Save()) {
						Disp_TriggerSaveAnim();
					}
				}
			}
			break;
		default:
			break;
		}
	} else {
		if(app_state == CS_TYPING) {
			Scroll_Reset(&text_scroll);
			App_OnChar(ch, frm);

			App_UpdateTitle(1);
		} else if(app_state == CS_SAVING) {
			App_OnCharSave(ch);
		} else if(app_state == CS_OPENING) {
			App_OnCharOpen(ch);
		}
	}
}


/**************************************************************************
 * Delegates
 **************************************************************************/

void
App_AnimationDel(void (*OnStart)(void), void (*OnEnd)(void))
{	
	scroll_anim_del = Anim_Init(OnStart, OnEnd);
	disp_anim_del = Anim_Init(OnStart, OnEnd);
	
	Scroll_AnimationDel(&open_scroll, scroll_anim_del);
	Scroll_AnimationDel(&text_scroll, scroll_anim_del);

	Disp_AnimDel(disp_anim_del);
}


void
App_FullscreenDel(void (*ToggleFullscreen)(void))
{
	fullscreen_del = ToggleFullscreen;
}


void
App_QuitRequestDel(void (*Quit)(void))
{
	quit_del = Quit;
}

void
App_UpdateTitleDel(void (*UpdateTitle)(char*))
{
	update_title_del = UpdateTitle;
}
