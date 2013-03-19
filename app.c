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

// should put these defines in app.h OR have them loaded from a singleton resource module
#define FONT_SIZE 18
#define CHARS_PER_LINE 64

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
	//char ch[] = {key, '\0'};
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
		//puts(ch);
		break;
	}
}

//The App module should keep track of the File state
// but delegate to other modules to handle the File

void
App_SaveAs(const char * filename)
{
	FILE * file;
	
	printf("Saving to file: %s\n", filename);
	
	file = fopen(filename, "w");
	
	Frame_Write(frm, file);
	
	fclose(file);
	
	printf("...Done.\n");
	
	//save the filename
	free(_filename);
	_filename = (char *)malloc(strlen(filename) + 1);
	strcpy(_filename, filename);
}

void
App_Save()
{
	assert(_filename != 0);
	
	App_SaveAs(_filename);
}
