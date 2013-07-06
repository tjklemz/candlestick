// anim.c

#include "anim.h"

#include <stdlib.h>

anim_del_t*
Anim_Init(anim_del_func_t on_start, anim_del_func_t on_end)
{
	anim_del_t * anim_del = (anim_del_t *)malloc(sizeof(anim_del_t));
	
	anim_del->on_start = on_start;
	anim_del->called_start = 0;
	anim_del->on_end = on_end;
	anim_del->called_end = 0;

	return anim_del;
}


void
Anim_Destroy(anim_del_t * anim_del)
{
	//TODO: clear memory for anim_del struct

	free(anim_del);
	anim_del = 0;
}


void
Anim_Start(anim_del_t * anim_del)
{		
	if(anim_del && !anim_del->called_start) {
		anim_del->on_start();
		anim_del->called_start = 1;
		anim_del->called_end = 0;
	}
}


void
Anim_End(anim_del_t * anim_del)
{	
	if(anim_del && !anim_del->called_end) {
		anim_del->on_end();
		anim_del->called_end = 1;
		anim_del->called_start = 0;
	}
}
