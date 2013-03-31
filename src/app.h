/*************************************************************************
 * app.h
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

#ifndef APP_H
#define APP_H

typedef enum {
	CS_UNDEFINED   = -1,
	CS_ARROW_LEFT  = 37,
	CS_ARROW_UP    = 38,
	CS_ARROW_RIGHT = 39,
	CS_ARROW_DOWN  = 40
} cs_key_t;

//Golden Rectangle
#define WIN_INIT_WIDTH  850 * 1.1
#define WIN_INIT_HEIGHT 525 * 1.1

#define FONT_SIZE 18


void
App_OnInit();

void
App_OnDestroy();

void
App_OnResize(int w, int h);

void
App_OnRender();

void
App_OnSpecialKeyUp(cs_key_t key);

void
App_OnSpecialKeyDown(cs_key_t key);

void
App_OnKeyDown(char key);

void
App_AnimationDel(void (*OnStart)(void), void (*OnEnd)(void));

void
App_Open(const char * filename);

void
App_SaveAs(const char * filename);

void
App_Save();

void
App_Reload();

#endif
