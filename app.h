#ifndef APP_H
#define APP_H

/*typedef enum key_codes {
} KeyCode;
*/

#define WIN_INIT_WIDTH  850
#define WIN_INIT_HEIGHT 525

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

#endif
