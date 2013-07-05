/*************************************************************************
 * scroll.c
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

#include "scroll.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_STEPS 22
#define STEP_AMT (0.0004f)


/**************************************************************************
 * Scrolling and Animation
 **************************************************************************/

static
void
Scroll_AnimStart(scrolling_t * scroll)
{
	anim_del_t * anim_del = scroll->anim_del;
		
	if(anim_del && !anim_del->called_start) {
		anim_del->on_start();
		anim_del->called_start = 1;
		anim_del->called_end = 0;
	}
}

static
void
Scroll_AnimEnd(scrolling_t * scroll)
{
	if(!scroll->requested && !scroll->moving) {
		anim_del_t * anim_del = scroll->anim_del;
		
		if(anim_del && !anim_del->called_end) {
			anim_del->on_end();
			anim_del->called_end = 1;
			anim_del->called_start = 0;
		}
	}
}

void Scroll_TextScroll(scrolling_t * scroll)
{
	if((scroll->step < NUM_STEPS || scroll->requested)) {
		float amt = (scroll->dir == SCROLL_UP) ? STEP_AMT : -STEP_AMT;
		
		scroll->amt += amt*pow((double)scroll->step, 1.22);

		if(scroll->amt < 0) {
			scroll->amt = 0;
		} else if(scroll->amt > scroll->limit) {
			scroll->amt = scroll->limit;
		}
		scroll->moving = 1;
		++scroll->step;
	} else {
		scroll->moving = 0;
		scroll->step = 0;
		
		Scroll_AnimEnd(scroll);
	}
}

void Scroll_OpenScroll(scrolling_t * scroll)
{
	static int num_scrolls = 0;
	
	if((scroll->moving || scroll->requested)) {
		float amt = (scroll->dir == SCROLL_UP) ? -STEP_AMT : STEP_AMT;
		double old_amt = scroll->amt;
		scrolling_dir_t old_dir = scroll->dir;
		
		scroll->amt += 0.15*pow(1 + num_scrolls, 1.7)*amt*pow((double)scroll->step, 1.75);
		// scroll->amt += 0.7*(1 + num_scrolls*num_scrolls)*amt*pow((double)scroll->step / 100.0, 1.7);
		
		//default to false
		scroll->moving = 0;
		
		if(scroll->amt < 0) {
			scroll->amt = 0;
			scroll->step = 0;
		} else if(scroll->amt > scroll->limit) {
			scroll->amt = scroll->limit;
			scroll->step = 0;
		} else {
			// see if we have passed an integer amount (i.e. a line)
			if((scroll->dir == SCROLL_UP && old_dir == SCROLL_UP && 
				(int)ceil(old_amt) > (int)ceil(scroll->amt)) ||
				(scroll->dir == SCROLL_DOWN && (int)old_amt < (int)scroll->amt)) {

				scroll->amt = round(scroll->amt);
				scroll->step = 0;
				++num_scrolls;
			} else {
				// not done, so keep moving
				scroll->moving = 1;
			}
			++scroll->step;
		}
	} else {
		scroll->moving = 0;
		scroll->step = 0;
		num_scrolls = 0;
		
		Scroll_AnimEnd(scroll);
	}
}

void
Scroll_Update(scrolling_t * scroll)
{
	scroll->on_update(scroll);
}

void
Scroll_Requested(scrolling_t * scroll, scrolling_dir_t dir)
{
	scroll->requested = 1;

	/*
	 * Step keeps track of how long the key's been down.
	 * If the user presses the other key (changes direction),
	 * then reset it so the momentum/inertia resets.
	 */

	if(dir != scroll->dir) {
		scroll->step = 0;
	}

	scroll->dir = dir;
	
	Scroll_AnimStart(scroll);
}

void
Scroll_StopRequested(scrolling_t * scroll)
{
	scroll->requested = 0;
}

void
Scroll_Reset(scrolling_t * scroll)
{
	scroll->requested = 0;
	scroll->step = 0;
	scroll->moving = 0;
	scroll->amt = 0;
	//scroll->dir = SCROLL_UP; // doesn't matter, but good to know the state
	
	Scroll_AnimEnd(scroll);
}

void
Scroll_AnimationDel(scrolling_t * scroll, anim_del_t * the_anim_del)
{
	scroll->anim_del = the_anim_del;
}
