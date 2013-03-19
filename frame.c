// frame.c

#include "frame.h"
#include "dlist.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct frame {
	int line_len;
	Node * lines;
	Node * cur_line;
};

typedef enum line_end_type {
	SOFT,
	HARD
} LINE_END;

struct line_type {
	unsigned char * text;
	int len;
	int size;
	LINE_END end;
};

int
Line_Length(Line * line)
{
	return line->len;
}

static
void
Line_SetLength(Line * line, int len)
{
	assert(len <= line->size);
	line->len = len;
}

//should rename to Line_TextRaw or something
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
	line->end = SOFT;
	
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

int
Frame_Length(Frame * frm)
{
	return frm->line_len;
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

//SoftWrap assumes current line is full
// Calls Frame_AddLine

static
void
Frame_SoftWrap(Frame * frm)
{
	int i;
	int wrap_amount;
	Line * full_line;
	Line * new_line;
	
	full_line = (Line *)frm->cur_line->data;
	
	assert(full_line->len == full_line->size);
	
	Frame_AddLine(frm);
	
	new_line = (Line *)frm->cur_line->data;
	
	i = full_line->len - 1;
	while(full_line->text[i] != ' ' && i > 0) {
		--i;
	}
	
	//make sure it is worth wrapping (greater than zero)
	if(i > 0) {
		//printf("New full_line len: %d\n", i);
		Line_SetLength(full_line, i);
		
		wrap_amount = full_line->size - i - 1;
		
		memcpy(new_line->text, &full_line->text[i+1], wrap_amount*sizeof(unsigned char));
		
		Line_SetLength(new_line, wrap_amount);
		//printf("New new_line len: %d\n", wrap_amount);
	}
}

//if there's room on the previous line, tries to do soft wrap
//assumes that the current line has at least one character
//Will call Frame_DeleteLine if undoes the soft wrap
static
void
Frame_UndoSoftWrap(Frame * frm)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->len > 0 && frm->cur_line->prev) {
		Line * prev_line = (Line *)frm->cur_line->prev->data;
		
		//only undo soft wrap if the line was soft wrapped
		if(prev_line->end == SOFT) {
			int i = cur_line->len;
			
			while(cur_line->text[i] != ' ' && i > 0) {
				--i;
			}
			
			//if i is not zero, then no need to soft wrap
			// (i.e. not on the first word of the line)
			if(i == 0) {
				//see if the previous line has room for the word
				int prev_room = prev_line->size - prev_line->len;
				int cur_room = cur_line->len;
				
				if(prev_room > cur_room) {
					memcpy(&prev_line->text[prev_line->len+1], cur_line->text, cur_room*sizeof(unsigned char));
					Line_SetLength(prev_line, prev_line->len + cur_room + 1);
					Frame_DeleteLine(frm);
				}
			}
		}
	}
}

void
Frame_InsertCh(Frame * frm, unsigned char ch)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->len < cur_line->size) {
		Line_InsertCh(cur_line, ch);
	} else {
		//reached the end of the line, make sure it is soft
		cur_line->end = SOFT;
		
		if(ch != ' ') {
			Frame_SoftWrap(frm);
			cur_line = (Line *)frm->cur_line->data;
			Line_InsertCh(cur_line, ch);
		} else {
			Frame_AddLine(frm);
		}
	}
}

void
Frame_DeleteCh(Frame * frm)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->len > 0) {
		Line_DeleteCh(cur_line);
		if(cur_line->len > 0) {
			Frame_UndoSoftWrap(frm);
		}
	} else {
		Frame_DeleteLine(frm);
	}
}

void
Frame_InsertNewLine(Frame * frm)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	cur_line->end = HARD;
	
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

void
Frame_IterEnd(Frame * frm)
{
	iter = frm->cur_line;
}

Line*
Frame_IterPrev(Frame * frm)
{
	Line * prev = NULL;
	
	if(iter) {
		prev = (Line*)iter->data;
		iter = iter->prev;
	}
	
	return prev;
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

int
Frame_IterHasNext(Frame * frm)
{
	return (iter && iter->next);
}

#define BUF_SIZE 4096
#define EOL_SIZE 1
#define SOFT_CHAR ' '
#define HARD_CHAR '\n'

void
Frame_Write(Frame * frm, FILE * file)
{
	char buf[BUF_SIZE];
	Line * cur_line;
	int len = 0;
	int ptr = 0;
	
	Frame_IterBegin(frm);
	
	while((cur_line = Frame_IterNext(frm))) {
		len = Line_Length(cur_line);
		
		//flush the buffer if it is full (+1 because of EOL character)
		if(ptr + len + EOL_SIZE >= BUF_SIZE) {
			fwrite(buf, sizeof(char), ptr, file);
			ptr = 0;
		}
		
		memcpy(&buf[ptr], Line_Text(cur_line), len * sizeof(char));
		
		ptr += len;
		
		//put the right character for EOL
		if(cur_line->end == SOFT && Frame_IterHasNext(frm)) {
			buf[ptr] = SOFT_CHAR;
			ptr += EOL_SIZE;
		} else if(cur_line->end == HARD) {
			buf[ptr] = HARD_CHAR;
			ptr += EOL_SIZE;
		}
	}
	
	//flush the buffer
	fwrite(buf, sizeof(char), ptr, file);
}
