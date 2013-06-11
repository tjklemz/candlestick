#include "scroll.h"

#include <math.h>

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

void
Scroll_Update(scrolling_t * scroll)
{	
	if((scroll->step < NUM_STEPS || scroll->requested)) {
		float amt = (scroll->dir == SCROLL_UP) ? STEP_AMT : -STEP_AMT;
		
#if defined(_WIN32)
		scroll->amt += amt*pow((double)scroll->step / 100.0, 2.125);
#else
		scroll->amt += amt*pow((double)scroll->step / 100.0, 1.22);
#endif
		
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
	scroll->dir = SCROLL_UP; // doesn't matter, but good to know the state
	
	Scroll_AnimEnd(scroll);
}

void
Scroll_AnimationDel(scrolling_t * scroll, anim_del_t * the_anim_del)
{
	scroll->anim_del = the_anim_del;
}
