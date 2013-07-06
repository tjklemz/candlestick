// anim.h

#ifndef CS_ANIM_H
#define CS_ANIM_H

typedef void (*anim_del_func_t)(void);

typedef struct {
	anim_del_func_t on_start;
	int called_start;
	anim_del_func_t on_end;
	int called_end;
} anim_del_t;

anim_del_t*
Anim_Init(anim_del_func_t on_start, anim_del_func_t on_end);

void
Anim_Destroy(anim_del_t * anim_del);

void
Anim_Start(anim_del_t * anim_del);

void
Anim_End(anim_del_t * anim_del);

#endif
