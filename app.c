#include "app.h"
#include "disp.h"
#include "frame.h"

#include <stdio.h>
#include <ctype.h>

//This class is really an "App Delegate" that simply tells the other
// classes what to do.

// should put these defines in app.h OR have them loaded from a singleton resource module
#define FONT_SIZE 18
#define CHARS_PER_LINE 64
//static const int FONT_SIZE = 18;
//static const int WIDTH_PTS = 1152;

static Frame * frm = 0;

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
	//fn-delete on mac (actual delete)
	case 239:
	//delete on mac (actual backspace)
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

void App_SaveAs(const char * filename)
{
	Line * cur_line;
	int len = 0;
	FILE * file;
	printf("Saving to file: %s\n", filename);
	
	file = fopen(filename, "w");
	
	Frame_IterBegin(frm);
	while((cur_line = Frame_IterNext(frm))) {
		len = Line_Length(cur_line);
		if(len == 0) {
			fputc('\n', file);
		} else if(len == Frame_Length(frm)) {
			fputc(' ', file);
		} else {
			fwrite(Line_Text(cur_line), 1, Line_Length(cur_line), file);
		}
	}
	
	fclose(file);
}

