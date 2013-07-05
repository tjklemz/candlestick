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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <math.h>

#if defined(__unix__) || defined(__APPLE__)
#  include <dirent.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#elif defined(_WIN32)
#  include <windows.h>
#  include <io.h>
#endif

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
static Node * files = 0;

static update_title_del_func_t update_title_del = 0;

static anim_del_t * anim_del = 0;
static fullscreen_del_func_t fullscreen_del = 0;
static int is_fullscreen = 0;
static quit_del_func_t quit_del = 0;

static cs_app_state_t app_state = CS_TYPING;
static scrolling_t open_scroll = {0};
static scrolling_t text_scroll = {0};
static scrolling_t * cur_scroll;

#if defined(__APPLE__)
// assumes exe lives in Appname.app/Contents/MacOS
#    define DOCS_FOLDER "../../../documents/"
#else
#    define DOCS_FOLDER "./documents/"
#endif

#define FILE_EXT ".txt"
#define FILE_EXT_LEN 4
#define MAX_FILE_CHARS (CHARS_PER_LINE - FILE_EXT_LEN)


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
	} else {
		Disp_Destroy();
		text_scroll.requested = 0;
		open_scroll.requested = 0;
	}
	
	Disp_Init(FONT_SIZE);
	
	app_state = CS_TYPING;
	cur_scroll = &text_scroll;
}


static
void
App_DestroyFilesList()
{
	if(files) {
		Node * travel = files;
		
		while(travel) {
			free(travel->data);
			travel->data = 0;
			travel = travel->next;
		}
		
		Node_Destroy(files);
		files = 0;
	}
}


void
App_OnDestroy()
{
	free(anim_del);
	anim_del = 0;
	
	Line_Destroy(filename);
	filename = 0;
	
	//filename_buf and filename should never point to the same thing,
	//so this is a valid delete.
	Line_Destroy(filename_buf);
	filename_buf = 0;
	
	App_DestroyFilesList();
	
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
		Disp_SaveScreen(Line_Text(filename_buf));
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
App_OnChar(char * ch)
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
	
	//printf("Got mem, now reading...\n");
	
	result = fread(buffer, 1, len, file);

	//printf("len: %ld result: %ld", len, result);

	if(result != len) {
		fputs("Reading error for App_Read", stderr);
		exit(3);
	}
	
	//printf("Read into mem, now filling the frame with len: %ld\n", len);
	
	for(i = 0; i < len; i += size) {
		chartorune(&rune, &buffer[i]);
		
		if(rune == Runeerror) {
			fputs("Bad things happened! Error parsing file as UTF-8.", stderr);
			break;
		}
		
		size = runetochar(ch, &rune);
		ch[size] = '\0';
		
		//these print statements really slow down the read...
		//printf("Got char of size: %d\n", size);
		
		App_OnChar(ch);
	}
	
	free(buffer);
}


/**************************************************************************
 * CreateAbsFile
 *
 * Will return a newly allocated full_filename that has everything
 * needed to save or open (i.e. actual file access).
 *
 * Expects the current "filename" to be valid (and have an extension
 * if there is one).
 **************************************************************************/

static
char *
App_CreateAbsFile(char * filename)
{
	int filename_size;
	char * full_filename;

	filename_size = strlen(DOCS_FOLDER) + strlen(filename) + 1;
	full_filename = (char*)malloc(filename_size * sizeof(char));
	
	strcpy(full_filename, DOCS_FOLDER);
	strcat(full_filename, filename);
	
	return full_filename;
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
void
App_Open(char * the_filename)
{
	FILE * file;
	char * full_filename;
	
	full_filename = App_CreateAbsFile(the_filename);
	
	printf("Opening file: %s\n", full_filename);
	file = fopen(full_filename, "rb");
	
	if(!file) {
		fprintf(stderr, "Could not open requested file!\n");
		// do nothing... (we'll take the Mac approach for now)
	} else {
		App_Read(file);

		fclose(file);
		printf("...Done.\n");

		App_SaveFilename(the_filename);

		Scroll_Reset(&text_scroll);
		cur_scroll = &text_scroll;
		app_state = CS_TYPING;
	}
	
	free(full_filename);
}


static
void
App_CheckDocsFolder()
{
#if defined(__unix__) || defined(__APPLE__)
	{
		struct stat st = {0};

		if(stat(DOCS_FOLDER, &st) == -1) {
			mkdir(DOCS_FOLDER, (S_IRWXU | S_IRWXG | S_IRWXO));
		}
	}
#elif defined(_WIN32)
	{
		CreateDirectory(DOCS_FOLDER, NULL);
	}
#endif
}


static
int
App_Save()
{
	FILE * file;
	
	char * the_filename = Line_Text(filename);
	char * full_filename = App_CreateAbsFile(the_filename);
	
	App_CheckDocsFolder();
	
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
int
App_FileExists(char * filename)
{
	FILE * file;
	
    if((file = fopen(filename, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}


static
int
App_SaveAs()
{
	int saved = 0;
	char * test_filename;
	char * full_test_filename;
	Line * test;
	int len;

	len = strlen(Line_Text(filename_buf));

	if(len > 0) {

		test = Line_Init(len);
		//copy the buffer
		Line_InsertStr(test, Line_Text(filename_buf));
		//tack on the .txt extension
		Line_InsertStr(test, FILE_EXT);

		//create a full filename from the copy
		test_filename = Line_Text(test);
		full_test_filename = App_CreateAbsFile(test_filename);

		/*
		 * If the file already exists, don't do anything.
		 * This is why the copy was made (test), so that
		 * upon returning, if nothing happened, the buffer
		 * is still there, untouched.
		 */

		if(!App_FileExists(full_test_filename)) {
			Line_Destroy(filename);
			filename = test;

			saved = App_Save();

			// don't destroy the buffer if something went wrong,
			// although we're kinda screwed...
			if(saved) {
				Line_Destroy(filename_buf);
				filename_buf = 0;
			}
		} else {
			Line_Destroy(test);
		}

		free(full_test_filename);
	}
	
	return saved;
}


/**************************************************************************
 * PopulateFiles
 *
 * This only allows ".txt" extensions at the moment.
 * Creates the list of filenames (just the names, with extensions).
 *
 * Returns the number of files that are now in the list.
 **************************************************************************/

static
int
App_PopulateFiles()
{
	Node * cur = 0;
	int num_files = 0;
	
	App_DestroyFilesList();
	
	App_CheckDocsFolder();
	
#if defined(__unix__) || defined(__APPLE__)
	{
		struct dirent * dp;
		DIR * dfd;
		
		dfd = opendir(DOCS_FOLDER);
		
		while((dp = readdir(dfd))) {
			struct stat st = {0};
			int file_len = strlen(dp->d_name);
			char * filename_full = malloc(strlen(dp->d_name) + strlen(DOCS_FOLDER) + 1);
			
			sprintf(filename_full, "%s%s", DOCS_FOLDER, dp->d_name);
			
			//only do something if the thing is an actual file and is .txt
			if(stat(filename_full, &st) != -1) {
				if(S_ISREG(st.st_mode & S_IFMT)) {
					int len = strlen(dp->d_name);
					if(len > 4 && !strcmp(&dp->d_name[len - 4], FILE_EXT)) {
						//alloc storage for the filename
						Node * file = Node_Init();
						char * filename = malloc(file_len + 1);
						//copy the filename
						strcpy(filename, dp->d_name);
						//store it
						file->data = (void*)filename;
						//add the file to the list
						if(!files) {
							files = file;
							cur = files;
						} else {
							Node_Append(cur, file);
							//update the tail pointer
							cur = cur->next;
						}
						++num_files;
					}
				}
			}
		}
		
		closedir(dfd);
	}
#elif defined(_WIN32)
	{
		struct _finddata_t file_d;
		long hFile;

		char search[60];
		sprintf(search, "%s*%s", DOCS_FOLDER, FILE_EXT);

		hFile = _findfirst(search, &file_d);

		if(hFile != -1L) {
			do {
				Node * file = Node_Init();
				char * filename = (char*)malloc(strlen(file_d.name)+1);
				strcpy(filename, file_d.name);
				file->data = (void*)filename;
				// could possibly change list structure to contain a head and tail empty node
				// then don't have to have this if statement
				if(!files) {
					files = file;
					cur = files;
				} else {
					Node_Append(cur, file);
					//update the tail pointer
					cur = cur->next;
				}
				++num_files;
			} while(_findnext(hFile, &file_d) == 0);
		}

		_findclose(hFile);
	}
#endif
	
	return num_files;
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
		if(App_SaveAs()) {
			app_state = CS_TYPING;
			cur_scroll = &text_scroll;
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
}


static
void
App_OnCharOpen(char * ch)
{
	switch(*ch) {
	case '\n':
	case '\r':
	{
		Node * cur;
		int i;
		double amt = open_scroll.amt;
		int file_num = (int)(open_scroll.dir == SCROLL_UP ? ceil(amt) : floor(amt));
		
		for(cur = files, i = 0; cur && i != file_num; cur = cur->next, ++i) {
			//just chill
		}
		
		if(cur) {
			App_Open((char*)cur->data);
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
				open_scroll.limit = App_PopulateFiles() - 1;
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
			App_OnChar(ch);

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
	assert(anim_del == 0);
	
	anim_del = (anim_del_t *)malloc(sizeof(anim_del_t));
	
	anim_del->on_start = OnStart;
	anim_del->called_start = 0;
	anim_del->on_end = OnEnd;
	anim_del->called_end = 0;
	
	Scroll_AnimationDel(&open_scroll, anim_del);
	Scroll_AnimationDel(&text_scroll, anim_del);
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
