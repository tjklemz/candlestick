#ifndef APP_H
#define APP_H

/*typedef enum key_codes {
} KeyCode;
*/

//Golden Rectangle
#define WIN_INIT_WIDTH  850 * 1.1
#define WIN_INIT_HEIGHT 525 * 1.1

// should put these defines in app.h OR have them loaded from a singleton resource module
#define FONT_SIZE 18
#define CHARS_PER_LINE 64

void
App_OnInit();

void
App_OnDestroy();

void
App_OnResize(int w, int h);

void
App_OnRender();

void
App_OnKeyDown(unsigned char key);

void
App_Open(const char * filename);

void
App_SaveAs(const char * filename);

void
App_Save();

void
App_Reload();

#endif
