/*************************************************************************
 * scroll.h
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

#ifndef SCROLL_H
#define SCROLL_H

typedef void (*anim_del_func_t)(void);

struct scrolling_tag;
typedef void (*scroll_func_t)(struct scrolling_tag *);

typedef struct {
	anim_del_func_t on_start;
	int called_start;
	anim_del_func_t on_end;
	int called_end;
} anim_del_t;

typedef enum {
	SCROLL_UP,
	SCROLL_DOWN
} scrolling_dir_t;

typedef struct scrolling_tag {
	int requested;
	unsigned int step;
	int moving;
	double amt;
	double limit;
	scrolling_dir_t dir;
	anim_del_t * anim_del;
	scroll_func_t on_update; 
} scrolling_t;

void
Scroll_TextScroll(scrolling_t * scroll);

void
Scroll_OpenScroll(scrolling_t * scroll);

void
Scroll_Update(scrolling_t * scroll);

void
Scroll_Requested(scrolling_t * scroll, scrolling_dir_t dir);

void
Scroll_StopRequested(scrolling_t * scroll);

void
Scroll_Reset(scrolling_t * scroll);

void
Scroll_AnimationDel(scrolling_t * scroll, anim_del_t * the_anim_del);


#endif
