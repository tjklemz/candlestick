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

#define APP_NAME "candlestick"

//Golden Rectangle
#define WIN_INIT_WIDTH  (850 * 1.2)
#define WIN_INIT_HEIGHT (514 * 1.2)

#define FPS 260

/* 
 * Win32 uses Sleep, which takes milliseconds
 * whereas Unix-esque platforms use usleep,
 * which of course takes microseconds
 */
#if defined(_WIN32)
# define SKIP_TICKS (1000 / FPS)
#else
# define SKIP_TICKS (1000000 / FPS)
#endif

#define FONT_SIZE 24


/**************************************************************************
 * Keys
 **************************************************************************/

#define MODS_SHIFTED(mods)  ((mods & CS_SHIFT_L) || (mods & CS_SHIFT_R))
#define MODS_COMMAND(mods)  ((mods & CS_CONTROL_L) || (mods & CS_CONTROL_R) || \
                             (mods & CS_SUPER_L) || (mods & CS_SUPER_R))

typedef enum {
	CS_UNDEFINED     = -1,
	//mods
	CS_NONE          = 0,
	CS_SHIFT_L       = (1 << 0),
	CS_SHIFT_R       = (1 << 1),
	CS_SHIFT         = (CS_SHIFT_L | CS_SHIFT_R),
	CS_CONTROL_L     = (1 << 2),
	CS_CONTROL_R     = (1 << 3),
	CS_CONTROL       = (CS_CONTROL_L | CS_CONTROL_R),
	CS_ALT_L         = (1 << 4),
	CS_ALT_R         = (1 << 5),
	CS_ALT           = (CS_ALT_L | CS_ALT_R),
	CS_SUPER_L       = (1 << 6),
	CS_SUPER_R       = (1 << 7),
	CS_SUPER         = (CS_SUPER_L | CS_SUPER_R),
	//other
	CS_ESCAPE        = 27,
	CS_ARROW_LEFT    = 37,
	CS_ARROW_UP      = 38,
	CS_ARROW_RIGHT   = 39,
	CS_ARROW_DOWN    = 40
} cs_key_t;

typedef long cs_key_mod_t;


/**************************************************************************
 * Methods
 **************************************************************************/

void
App_OnInit();

void
App_OnDestroy();

void
App_OnResize(int w, int h);

void
App_OnRender();

void
App_OnUpdate();

void
App_OnSpecialKeyUp(cs_key_t key, cs_key_mod_t mods);

void
App_OnSpecialKeyDown(cs_key_t key, cs_key_mod_t mods);

void
App_OnKeyDown(char * key, cs_key_mod_t mods);

void
App_AnimationDel(void (*OnStart)(void), void (*OnEnd)(void));

void
App_FullscreenDel(void (*ToggleFullscreen)(void));

void
App_QuitRequestDel(void (*OnQuitRequest)(void));

#endif

