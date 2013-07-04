#include "scroll.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// #if !defined(_WIN32)
// #define NUM_STEPS 22
// #define STEP_AMT (0.0006f)
// #else
#define NUM_STEPS 18
#define STEP_AMT (0.0006f)
// #endif

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
		
		scroll->amt += 1.2*amt*pow((double)scroll->step, 1.8);

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
		//double max_amt = abs(amt)*pow((double)(NUM_STEPS - 1) >> 3), 2.125);
		double old_amt = scroll->amt;
		scrolling_dir_t old_dir = scroll->dir;
		
		//scroll->amt += amt*pow((double)scroll->step / 100.0, 1.7);
// #if defined(_WIN32)
		scroll->amt += 0.75*pow(1 + num_scrolls, 1.1)*amt*pow((double)scroll->step, 1.7);
// #else
		// scroll->amt += 0.7*(1 + num_scrolls*num_scrolls)*amt*pow((double)scroll->step / 100.0, 1.7);
// #endif
		
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
	scroll->update(scroll);
}

void
Scroll_UpRequested(scrolling_t * scroll)
{
	scroll->requested = 1;
	scroll->dir = SCROLL_UP;
	
	Scroll_AnimStart(scroll);
}

void
Scroll_DownRequested(scrolling_t * scroll)
{
	scroll->requested = 1;
	scroll->dir = SCROLL_DOWN;
	
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
