#ifndef APP_H
#define APP_H

/*typedef enum key_codes {
} KeyCode;
*/

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
