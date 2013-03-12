#include "app.h"
#include "disp.h"
#include "frame.h"

#include <stdio.h>
#include <ctype.h>

//should Window handle the frame? or the input? or is that the same thing?
//in other words, who passes the frame to Display? who calls render?

//Is App turning into Window? or vice-versa?
//or is App simply delegating to Window so that Window can respond to events
//regardless of platform, etc (yes)


static const int FONT_SIZE = 17;
static const int WIDTH_PTS = 828;

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
		frm = Frame_Init(WIDTH_PTS / FONT_SIZE);
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

