/*************************************************************************
 * frame.c -- Defines a Frame abstraction that holds lines of pure text.
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
	int rev_iter_start;
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
	frm->rev_iter_start = 1;
	
	return frm;
}

void
Frame_Destroy(Frame * frm)
{
	Line * line = NULL;
	Node * travel = frm->lines;
	
	while(travel) {
		line = (Line *)travel->data;
		Line_Destroy(line);
		travel = travel->next;
	}
	
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

/**************************************************************************
 * Iterator
 **************************************************************************/

static Node * iter = NULL;

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

static
int
Frame_IterHasNext(Frame * frm)
{
	return (iter != NULL);
}

void
Frame_IterBegin(Frame * frm)
{
	iter = frm->lines;
}


/**************************************************************************
 * Reverse Iterator
 **************************************************************************/

static Node * rev_iter = NULL;

void
Frame_SetRevIterBegin(Frame * frm, int line)
{
	if(line < 1) {
		line = 1;
	}
	frm->rev_iter_start = line;
}

void
Frame_RevIterBegin(Frame * frm)
{
	int i = 1;
	rev_iter = frm->cur_line;
	while(i < frm->rev_iter_start && rev_iter) {
		Frame_RevIterNext(frm);
		++i;
	}
}

Line*
Frame_RevIterNext(Frame * frm)
{
	//reverse of next is previous
	Line * prev = NULL;
	
	if(rev_iter) {
		prev = (Line*)rev_iter->data;
		rev_iter = rev_iter->prev;
	}
	
	return prev;
}


/**************************************************************************
 * Frame write
 **************************************************************************/

#define BUF_SIZE 4096
#define EOL_SIZE 1
#define SOFT_CHAR ' '
#define HARD_CHAR '\n'

//The implemenation of this needs to change so that
// Frame doesn't have to worry about Files. (?) <-- Necessary? Or over-engineering it?
//It should work just like how Fnt displays the Frame
// (as far as interfacing with Frame).
//OR have a method that takes a function pointer OnLine()?
//The problem really is that Frame does not store the EOL character.
//Should this change? Then this breaks Fnt.c (display code)

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
