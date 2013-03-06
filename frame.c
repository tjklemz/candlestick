// frame.c

#include "frame.h"
#include "dlist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct frame {
	int line_len;
	Node * lines;
	Node * cur_line;
};

struct line_type {
	unsigned char * text;
	int len;
	int size;
};

int
Line_Length(Line * line)
{
	return line->len;
}

unsigned char*
Line_Text(Line * line)
{
	return line->text;
}

static
Line*
Line_CreateLine(int size)
{
	Line * line = (Line *)malloc(sizeof(Line));
	line->text = (unsigned char *)malloc(sizeof(unsigned char) * size);
	line->size = size;
	line->len = 0;
	
	return line;
}

static
void
Line_Destroy(Line * line)
{
	free(line->text);
	free(line);
	
	line = NULL;
}

static
void
Line_InsertCh(Line * line, unsigned char ch)
{
	if(line->len < line->size) {
		line->text[line->len] = ch;
		line->len += 1;
	}
}

static
void
Line_DeleteCh(Line * line)
{
	if(line->len > 0) {
		line->len -= 1;
	}
}

static
void
Frame_AddLine(Frame * frm)
{
	Node * new_line = Node_Init();
	new_line->data = Line_CreateLine(frm->line_len);
	Node_Append(frm->lines, new_line);
	
	frm->cur_line = frm->cur_line->next;
}

static
void
Frame_DeleteLine(Frame * frm)
{
	//always have at least one line
	if(frm->cur_line != frm->lines) {
		Node * del = frm->cur_line;
		Line * del_line = (Line *)del->data;
		frm->cur_line = frm->cur_line->prev;
		Line_Destroy(del_line);
		Node_Delete(del);
	}
}

Frame *
Frame_Init(int line_len)
{
	Frame * frm = (Frame *)malloc(sizeof(Frame));
	frm->line_len = line_len;
	frm->lines = Node_Init();
	frm->lines->data = Line_CreateLine(line_len);
	frm->cur_line = frm->lines;
	
	return frm;
}

void
Frame_Destroy(Frame * frm)
{
	Node_Destroy(frm->lines);
	frm->cur_line = NULL;
	free(frm);
}

void
Frame_InsertCh(Frame * frm, unsigned char ch)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->len >= cur_line->size && ch != ' ') {
		Frame_AddLine(frm);
		cur_line = (Line *)frm->cur_line->data;
	}
	
	Line_InsertCh(cur_line, ch);
}

void
Frame_DeleteCh(Frame * frm)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->len > 0) {
		Line_DeleteCh(cur_line);
	} else {
		Frame_DeleteLine(frm);
	}
}

void
Frame_InsertNewLine(Frame * frm)
{
	Frame_AddLine(frm);
}

void
Frame_InsertTab(Frame * frm)
{
	int i;
	for(i = 0; i < 4; ++i) {
		Frame_InsertCh(frm, ' ');
	}
}

static Node * iter = NULL;

void
Frame_IterBegin(Frame * frm)
{
	iter = frm->lines;
}

Line*
Frame_IterNext(Frame * frm)
{
	Line * next = NULL;
	
	if(iter) {
		next = (Line *)iter->data;
		iter = iter->next;
	}
	return next;
}

