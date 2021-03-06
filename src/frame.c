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
#include "line.h"
#include "list.h"
#include "utf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct frame_t {
	int num_lines;
	Node * lines;
	Node * cur_line;
	int iter_end;
};

static
void
Frame_AddLine(Frame * frm)
{
	Node * new_line = Node_Init();
	new_line->data = Line_Init(CHARS_PER_LINE);
	Node_Append(frm->cur_line, new_line);
	
	frm->cur_line = frm->cur_line->next;
	frm->num_lines += 1;
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
		
		frm->num_lines -= 1;
	}
}

int
Frame_NumLines(Frame * frm)
{
	return frm->num_lines;
}

Frame *
Frame_Init()
{
	Frame * frm = (Frame *)malloc(sizeof(Frame));
	frm->lines = Node_Init();
	frm->lines->data = Line_Init(CHARS_PER_LINE);
	frm->cur_line = frm->lines;
	frm->num_lines = 1;
	frm->iter_end = 1;
	
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
	Line * full_line;
	Line * new_line;
	
	full_line = (Line *)frm->cur_line->data;
	
	Frame_AddLine(frm);
	
	new_line = (Line *)frm->cur_line->data;
	
	i = full_line->len;
	while(i > 0 && full_line->text[i] != ' ') {
		--i;
	}
	
	//make sure it is worth wrapping (greater than zero)
	if(i > 0) {
		// i now points to the space character, 
		// so move up one to start of the word
		++i;
		
		//soft wrap to the next line
		Line_InsertStr(new_line, &full_line->text[i]);
		
		//update the full line with the appropriate data (new length, etc)
		full_line->size -= strlen(&full_line->text[i]);
		full_line->num_chars -= utflen(&full_line->text[i]);
		full_line->len = i;
		full_line->text[i] = '\0';
		//printf("full_line->size: %d, chars: %d, len: %d\n", full_line->size, full_line->num_chars, full_line->len);
	}
}

// If there's room on the previous line, tries to put back the word.
// Assumes that the current line has at least one character.
// Will call Frame_DeleteLine if undoes the soft wrap.
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
			
			// check if the current line only has one word
			while(i > 0 && cur_line->text[i] != ' ') {
				--i;
			}
			
			// if i is *not* zero, then no need to soft wrap
			// (i.e. not on the first word of the line)
			// if i is zero, then current line has only one word (with no space) 
			if(i == 0) {
				//see if the previous line has room for the word (actual characters)
				int prev_char_room = (CHARS_PER_LINE - prev_line->num_chars);
				
				if(prev_char_room >= cur_line->num_chars) {
					Line_InsertStr(prev_line, cur_line->text);
					// no need for the current line now
					Frame_DeleteLine(frm);
				}
			}
		}
	}
}

void
Frame_InsertCh(Frame * frm, char * ch)
{	
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->num_chars < CHARS_PER_LINE) {
		Line_InsertCh(cur_line, ch);
	} else {
		cur_line->end = SOFT;
		
		if(*ch == ' ') {
			Line_InsertCh(cur_line, ch);
			Frame_AddLine(frm);
		} else {
			Frame_SoftWrap(frm);
			cur_line = (Line *)frm->cur_line->data;
			Line_InsertCh(cur_line, ch);
		}
	}
}

void
Frame_DeleteCh(Frame * frm)
{
	Line * cur_line = (Line *)frm->cur_line->data;
	
	if(cur_line->len > 0) {
		Line_DeleteCh(cur_line);
		Frame_UndoSoftWrap(frm);
	} else {
		Frame_DeleteLine(frm);
		
		cur_line = (Line *)frm->cur_line->data;
		if(cur_line->end == SOFT && 
			cur_line->num_chars > CHARS_PER_LINE && 
			cur_line->text[cur_line->len-1] == ' ') {
			Line_DeleteCh(cur_line);
		}
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
	Line * cur_line = (Line *)frm->cur_line->data;
	char * space = " ";
	
	for(i = 0; i < 4 && cur_line->num_chars < CHARS_PER_LINE; ++i) {
		Frame_InsertCh(frm, space);
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

/*static
int
Frame_IterHasNext(Frame * frm)
{
	return (iter != NULL);
}*/

void
Frame_IterBegin(Frame * frm)
{
	iter = frm->lines;
}

void
Frame_SetEnd(Frame * frm, int line)
{
	if(line < 1) {
		line = 1;
	}
	frm->iter_end = line;
}

void
Frame_IterEnd(Frame * frm)
{
	int i = 1;
	//start at the end
	iter = frm->cur_line;
	//move forward to the client's end
	while(i < frm->iter_end && iter->prev) {
		iter = iter->prev;
		++i;
	}
}

Line*
Frame_IterPrev(Frame * frm)
{
	//reverse of next is previous
	Line * prev = NULL;
	
	if(iter) {
		prev = (Line*)iter->data;
		iter = iter->prev;
	}
	
	return prev;
}


/**************************************************************************
 * Frame write
 **************************************************************************/

#define BUF_SIZE 4096
#define EOL_SIZE 1
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
		len = cur_line->len;
		
		//flush the buffer if it is full
		if(ptr + len + EOL_SIZE >= BUF_SIZE) {
			fwrite(buf, sizeof(char), ptr, file);
			ptr = 0;
		}
		
		memcpy(&buf[ptr], cur_line->text, len * sizeof(char));
		
		ptr += len;
		
		//put the right character for EOL
		if(cur_line->end == HARD) {
			buf[ptr] = HARD_CHAR;
			ptr += EOL_SIZE;
		}
	}
	
	//flush the buffer
	fwrite(buf, sizeof(char), ptr, file);
}
