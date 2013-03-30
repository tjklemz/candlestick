/*************************************************************************
 * frame.h -- Defines a Frame abstraction that holds lines of pure text.
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
Frame_SetRevIterBegin(Frame * frm, int line);

void
Frame_IterBegin(Frame * frm);

void
Frame_RevIterBegin(Frame * frm);

Line*
Frame_IterNext(Frame * frm);

Line*
Frame_RevIterNext(Frame * frm);

/************************************
 * Frame I/O
 ************************************/

void
Frame_Write(Frame * frm, FILE * file);

#endif
