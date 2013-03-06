/**********************************************************************
 * Candlestick
 * 
 * A minimalist, cross-platform writing app that focuses on text.
 * 
 * Written by:		Thomas Klemz
 * 					February 25, 2013
 * 
 * 
 * Requires SDL (tested w/ 1.2.15), freetype 2, and OpenGL (gl and glu)
 * Tested on GNU/Linux, CrunchBang
 **********************************************************************/

#define TRUE  1
#define FALSE 0

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <math.h>

const char * const APP_NAME = "Candlestick";

/**********************************************************************
 * handleKeyPress
 * 
 * handles the key press events for the app
 **********************************************************************/
 
void handleKeyPress(SDL_keysym * keysym)
{
	unsigned char ch = keysym->sym;
	
    switch (keysym->sym)
	{
	case SDLK_ESCAPE:
	    // ESC key was pressed
	    quit(0);
	    break;
	case SDLK_F1:
	    toggleFullscreen();
	    return;
	case SDLK_BACKSPACE:
		Frame_DeleteCh(frm);
		return;
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:	
		return;
	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		Frame_InsertNewLine(frm);
		return;
	default:
	    break;
	}
	
	if(isprint(ch)) {
		SDLMod mod = SDL_GetModState();
		if(mod & KMOD_SHIFT) {
			ch = toupper(ch);
		}
		Frame_InsertCh(frm, ch);
	}
}




/**********************************************************************
 * main
 **********************************************************************/
 
int main(int argc, char **argv)
{
	int done = FALSE;
	SDL_Event event;

    while(!done)
	{
		// event loop
	    while(SDL_PollEvent(&event))
		{
		    switch(event.type)
			{
			
			case SDL_KEYDOWN:
			    handleKeyPress(&event.key.keysym);
			    break;
			case SDL_QUIT:
			    done = TRUE;
			    break;
			default:
			    break;
			}
		}

	    // drawing
		Disp_Render();
	}

    quit(0);

    // should never get here
    return 0;
}
