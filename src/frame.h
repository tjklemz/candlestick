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

/* A study at Wichita State University found that CPL had only small effects 
 * on readability, including factors of speed and comprehension. When asked 
 * for preferences, however, 60% of respondents indicated a preference for 
 * either the shortest (35 CPL) or longest (95 CPL) lines used in the study.
 * At the same time, 100% of respondents selected either one of these 
 * quantities as being the least desirable.
 * <http://psychology.wichita.edu/surl/usabilitynews/72/LineLength.asp>
 *
 * The following is simply the average of 35 and 95, but with a nicer,
 * computer number (power of 2, 2^6).
 */

#define CHARS_PER_LINE 64

typedef struct frame Frame;
typedef struct line_type Line;

/************************************
 * Line (should be another file?)
 ************************************/

int
Line_Length(Line * line);

char*
Line_Text(Line * line);

/************************************
 * Frame Operations
 ************************************/

int
Frame_NumLines(Frame * frm);

Frame*
Frame_Init();

void
Frame_Destroy(Frame * frm);

void
Frame_InsertCh(Frame * frm, char ch);

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
Frame_SetEnd(Frame * frm, int line);

void
Frame_IterBegin(Frame * frm);

void
Frame_IterEnd(Frame * frm);

Line*
Frame_IterNext(Frame * frm);

Line*
Frame_IterPrev(Frame * frm);

/************************************
 * Frame I/O
 ************************************/

void
Frame_Write(Frame * frm, FILE * file);

#endif
