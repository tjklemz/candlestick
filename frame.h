// frame.h

#ifndef FRAME_H
#define FRAME_H

#include <stdio.h>

typedef struct frame Frame;
typedef struct line_type Line;

/************************************
 * Line (should be another file?)
 ************************************/

int
Line_Length(Line * line);

unsigned char*
Line_Text(Line * line);

/************************************
 * Frame Operations
 ************************************/

//returns number of characters per line
int
Frame_Length(Frame * frm);

Frame*
Frame_Init(int line_len);

void
Frame_Destroy(Frame * frm);

void
Frame_InsertCh(Frame * frm, unsigned char ch);

void
Frame_DeleteCh(Frame * frm);

void
Frame_InsertNewLine(Frame * frm);

void
Frame_InsertTab(Frame * frm);

/************************************
 * Frame Iterator
 ************************************/

void
Frame_IterBegin(Frame * frm);

void
Frame_IterEnd(Frame * frm);

Line*
Frame_IterPrev(Frame * frm);

Line*
Frame_IterNext(Frame * frm);

/************************************
 * Frame I/O
 ************************************/

void
Frame_Write(Frame * frm, FILE * file);

#endif
