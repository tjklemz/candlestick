#include "app.h"
#include "display.h"
#include "frame.h"

//should Appdow handle the frame? or the input? or is that the same thing?
//in other words, who passes the frame to Display? who calls render?

//Is App turning into Appdow? or vice-versa?
//or is App simply delegating to Appdow so that Appdow can respond to events
//regardless of platform, etc (yes)


static const int FONT_SIZE = 18;
static const int WIDTH_PTS = 828;

static Frame * frm = 0;

void
App_OnInit()
{
	Disp_Init(FONT_SIZE);
	frm = Frame_Init(WIDTH_PTS / FONT_SIZE);
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
		Frame_InsertCh(frm, key);
		break;
	}
}

