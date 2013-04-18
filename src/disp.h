/*************************************************************************
 * disp.h
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

#ifndef DISPLAY_H
#define DISPLAY_H

#include "frame.h"
#include "list.h"

typedef void (*anim_del_func_t)(void);

typedef struct {
	anim_del_func_t on_start;
	int called_start;
	anim_del_func_t on_end;
	int called_end;
} anim_del_t;

void
Disp_Init(int fnt_size);

void
Disp_Destroy();

void
Disp_BeginRender();

void
Disp_Update();

void
Disp_TypingScreen(Frame * frm);

void
Disp_SaveScreen(char * filename);

void
Disp_OpenScreen(Node * files);

void
Disp_Resize(int w, int h);

void
Disp_AnimationDel(anim_del_t * anim_del);

void
Disp_ScrollUpRequested();

void
Disp_ScrollDownRequested();

void
Disp_ScrollStopRequested();

void
Disp_ScrollReset();

#endif
