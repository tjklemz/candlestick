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
	scroll_func_t update; 
} scrolling_t;

void
Scroll_TextScroll(scrolling_t * scroll);

void
Scroll_OpenScroll(scrolling_t * scroll);

void
Scroll_Update(scrolling_t * scroll);

void
Scroll_UpRequested(scrolling_t * scroll);

void
Scroll_DownRequested(scrolling_t * scroll);

void
Scroll_StopRequested(scrolling_t * scroll);

void
Scroll_Reset(scrolling_t * scroll);

void
Scroll_AnimationDel(scrolling_t * scroll, anim_del_t * the_anim_del);


#endif
