/**********************************************************************
 * gapbuf.h
 * 
 * An implementation of the Gap Buffer data struct.
 * 
 * Copyright 2013 Thomas Klemz
 **********************************************************************/
 
#ifndef GAPBUF_H
#define GAPBUF_H

#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_GAP_SIZE 20

typedef struct gapbuf GapBuf;

GapBuf * GapBuf_Init(int gsize);

GapBuf * GapBuf_InitFromFile(FILE * file, int gsize);

void GapBuf_Destroy(GapBuf * gbuf);

int GapBuf_BufSize(GapBuf * gbuf);

void GapBuf_MoveGapToPoint(GapBuf * gbuf);

void GapBuf_SetPoint(GapBuf * gbuf, unsigned int offset);

int GapBuf_GapSize(GapBuf * gbuf);

unsigned int GapBuf_PointOffset(GapBuf * gbuf);

char GapBuf_GetChar(GapBuf * gbuf);

char GapBuf_PrevChar(GapBuf * gbuf);

void GapBuf_ReplaceChar(GapBuf * gbuf, char ch);

char GapBuf_NextChar(GapBuf * gbuf);

void GapBuf_PutChar(GapBuf * gbuf, char ch);

void GapBuf_InsertChar(GapBuf * gbuf, char ch);

void GapBuf_DeleteChars(GapBuf * gbuf, unsigned int size);

void GapBuf_InsertStr(GapBuf * gbuf, char * str, unsigned int len);

void GapBuf_Iterate(GapBuf * gbuf, void (*visit)(char * str, int len));

int GapBuf_SaveToFile(GapBuf * gbuf, FILE * file, unsigned int bytes);

#endif
