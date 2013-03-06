// frame.h

#ifndef FRAME_H
#define FRAME_H

typedef struct frame Frame;
typedef struct line_type Line;

int
Line_Length(Line * line);

unsigned char*
Line_Text(Line * line);

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

void
Frame_IterBegin(Frame * frm);

Line*
Frame_IterNext(Frame * frm);

#endif
