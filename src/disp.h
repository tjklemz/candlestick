#ifndef DISPLAY_H
#define DISPLAY_H

#include "frame.h"

void
Disp_Init(int fnt_size);

void
Disp_Destroy();

void
Disp_Render(Frame * frm);

void
Disp_Resize(int w, int h);

#endif
