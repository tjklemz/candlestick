/**********************************************************************
 * gapbuf.c
 * 
 * An implementation of the Gap Buffer data struct.
 * 
 * Converted to C code from the C++ original written by:
 * 
 * 		Author: Hsin Tsao (stsao@lazyhacker.com)
 *  	Version: 1.0 (June 12, 2003)
 * 		http://www.lazyhacker.com
 * 
 * Modifications: Copyright 2013 Thomas Klemz
 **********************************************************************/

#include "gapbuf.h"

struct gapbuf {
	char * point;
	char * buf;
	char * bufend;
	char * gapstart;
	char * gapend;
	int gsize;
};


/*
 * Copy the characters from one location to another.  We have
 * to write our own instead of using memcopy because we are
 * working within a single linear buffer and thus can have
 * overlap between the source and destination.
 */
static
int 
GapBuf_CopyBytes(GapBuf * gbuf, char * dest, char * src, unsigned int len)
{
    if ((dest == src) || (len == 0)) {
        return 1;
    }

    // if we're moving the character toward the front of the buffer
    if (src > dest) {

        // check to make sure that we don't go beyond the buffer
        if ((src + len) >= gbuf->bufend) {
            return 0;
        }

        for (; len > 0; len--) {
            *(dest++) = *(src++);
        }
        
    } else {

        // To prevent overwriting characters we still
        // need to move, go to the back and copy forward.
        src += len;
        dest += len;

        for (; len > 0; len--) {
            // decrement first 'cause we start one byte beyond where we want
            *(--dest) = *(--src); 
        }
    }

    return 1;
}


/*
 *  Expand the buffer to new size + GAP_SIZE.
 *  
 */
static
void 
GapBuf_ExpandBuffer(GapBuf * gbuf, unsigned int size)
{   
    // Check to see that we actually need to increase the buffer
    // since BufferSize doesn't include the gap.
    if (((gbuf->bufend - gbuf->buf) + size) > GapBuf_BufSize(gbuf)) {
        char * origbuf = gbuf->buf;

        int newbufsize = (gbuf->bufend - gbuf->buf) + size + gbuf->gsize;
        
        gbuf->buf = realloc(gbuf->buf, newbufsize);

        gbuf->point += gbuf->buf - origbuf;
        gbuf->bufend += gbuf->buf - origbuf;
        gbuf->gapstart += gbuf->buf - origbuf;
        gbuf->gapend += gbuf->buf - origbuf;
    }
}


/*
 *  Expand the size of the gap.  If the required
 *  size is less then the current gap size, do
 *  nothing.  If the size is greater than the 
 *  current size, increase the gap to the default
 *  gap size + size.
 */

static
void 
GapBuf_ExpandGap(GapBuf * gbuf, unsigned int size)
{
    if (size > GapBuf_GapSize(gbuf)) {
        size += gbuf->gsize;
        GapBuf_ExpandBuffer(gbuf, size);
        GapBuf_CopyBytes(gbuf, gbuf->gapend + size, gbuf->gapend, gbuf->bufend - gbuf->gapend);

        gbuf->gapend += size;
        gbuf->bufend += size;
    }
}

static 
int
GapBuf_InitBuffer(GapBuf * gbuf, int size)
{
	free(gbuf->buf);
	
	gbuf->buf = malloc(size);
	
	if(!gbuf->buf) {
		return 0;
	}
	
	gbuf->point = gbuf->buf;
	gbuf->gapstart = gbuf->buf;
	
	//initially gapend is outside of buffer
	gbuf->gapend = gbuf->buf + size;
	gbuf->bufend = gbuf->gapend;
	
	return 1;
}

GapBuf* 
GapBuf_Init(int gsize)
{
	GapBuf * gbuf = malloc(sizeof(GapBuf));
	gbuf->buf = NULL;
	gbuf->gsize = gsize;
	
	if(!GapBuf_InitBuffer(gbuf, gsize)) {
		return NULL;
	}
	
	return gbuf;
}

GapBuf* 
GapBuf_InitFromFile(FILE * file, int gsize)
{	
	fseek(file, 0L, SEEK_END);
	long filelen = ftell(file);
	rewind(file);
	
	GapBuf * gbuf = GapBuf_Init(filelen + gsize);
	
	if(!gbuf) {
		return NULL;
	}
	
	GapBuf_MoveGapToPoint(gbuf);
	GapBuf_ExpandGap(gbuf, (int)filelen);
	unsigned int amount = fread(gbuf->gapstart, 1, filelen, file);
	
	gbuf->gapstart += amount;
	
	//this moves the cursor to the end of the "file"
	//GapBuf_SetPoint(gbuf, amount);
	
	return gbuf;
}

void
GapBuf_Destroy(GapBuf * gbuf)
{
	free(gbuf->buf);
	gbuf = NULL;
}

int
GapBuf_BufSize(GapBuf * gbuf)
{
	return (gbuf->bufend - gbuf->buf) - (gbuf->gapend - gbuf->gapstart);
}

void
GapBuf_MoveGapToPoint(GapBuf * gbuf)
{
	if(gbuf->point == gbuf->gapstart) {
		return;
	}
	if(gbuf->point == gbuf->gapend) {
		gbuf->point = gbuf->gapstart;
		return;
	}
	
	//move gap towards the left
	if(gbuf->point < gbuf->gapstart) {
		GapBuf_CopyBytes(gbuf, gbuf->point + (gbuf->gapend - gbuf->gapstart),
				gbuf->point, gbuf->gapstart - gbuf->point);
		gbuf->gapend -= (gbuf->gapstart - gbuf->point);
		gbuf->gapstart = gbuf->point;
	} else {
		//since point is after the gap, find distance between
		//gapend and point and move that much from gapend to gapstart
		GapBuf_CopyBytes(gbuf, gbuf->gapstart, gbuf->gapend, gbuf->point - gbuf->gapend);
		gbuf->gapstart += gbuf->point - gbuf->gapend;
		gbuf->gapend = gbuf->point;
		gbuf->point = gbuf->gapstart;
	}
}

void
GapBuf_SetPoint(GapBuf * gbuf, unsigned int offset)
{
	gbuf->point = gbuf->buf + offset;
	
	if(gbuf->point > gbuf->gapstart) {
		gbuf->point += gbuf->gapend - gbuf->gapstart;
	}
}

int
GapBuf_GapSize(GapBuf * gbuf)
{
	return gbuf->gapend - gbuf->gapstart;
}

unsigned int
GapBuf_PointOffset(GapBuf * gbuf)
{
	if(gbuf->point > gbuf->gapend) {
		return ((gbuf->point - gbuf->buf) - (gbuf->gapend - gbuf->gapstart));
    } else {
		return (gbuf->point - gbuf->buf);
	}
}

char
GapBuf_GetChar(GapBuf * gbuf)
{
	// If the point is anywhere in the gap, then
    // it should always be at the start of the gap.
	if(gbuf->point == gbuf->gapstart) {
		gbuf->point = gbuf->gapend;
	}

	return *(gbuf->point);
}

char
GapBuf_PrevChar(GapBuf * gbuf)
{
	if (gbuf->point == gbuf->gapend) {
		gbuf->point = gbuf->gapstart;
	}

	--(gbuf->point);
	return *(gbuf->point);
}

void
GapBuf_ReplaceChar(GapBuf * gbuf, char ch)
{
	// Since we're just replacing the current character,
	// we don't need to move or modify the gap.
	if (gbuf->point == gbuf->gapstart) {
		gbuf->point = gbuf->gapend;
	}

	if (gbuf->point == gbuf->bufend) {
		GapBuf_ExpandBuffer(gbuf, 1);
		++(gbuf->bufend);
	}

	*gbuf->point = ch;
}

char
GapBuf_NextChar(GapBuf * gbuf)
{
	// point should not be in the gap.
	if (gbuf->point == gbuf->gapstart) {
		gbuf->point = gbuf->gapend;
		return *(gbuf->point);
	} 

	++gbuf->point;
	return *(gbuf->point);
}

void
GapBuf_PutChar(GapBuf * gbuf, char ch)
{
	GapBuf_InsertChar(gbuf, ch);
    gbuf->point++;
}

void
GapBuf_InsertChar(GapBuf * gbuf, char ch)
{
	// Here we do need to move the gap if the point
	// is not already at the start of the gap.

	if (gbuf->point != gbuf->gapstart) {
		GapBuf_MoveGapToPoint(gbuf);
	}

	// check to make sure that the gap has room
	if (gbuf->gapstart == gbuf->gapend) {
		GapBuf_ExpandGap(gbuf, 1);
	}

	*(gbuf->gapstart++) = ch;
}

void
GapBuf_DeleteChars(GapBuf * gbuf, unsigned int size)
{
	if (gbuf->point != gbuf->gapstart) {
        GapBuf_MoveGapToPoint(gbuf);
    }

    // We shifted the gap so that gapend points to the location
    // where we want to start deleting so extend it 
    // to cover all the characters.
    gbuf->gapend += size;
}

void
GapBuf_InsertStr(GapBuf * gbuf, char * str, unsigned int len)
{
	GapBuf_MoveGapToPoint(gbuf);

    if (len > GapBuf_GapSize(gbuf)) {
        GapBuf_ExpandGap(gbuf, len);
    }

    do {
        GapBuf_PutChar(gbuf, *(str++));
    } while (len--);
}

void
GapBuf_Iterate(GapBuf * gbuf, void (*visit)(char * str, int len))
{
	//this assumes that the gap comes before or after?
	//also, isn't this visiting the gap?
	(*visit)(gbuf->buf, gbuf->gapstart - gbuf->buf);
	(*visit)(gbuf->gapend, gbuf->bufend - gbuf->gapend);
	
	/*char *temp = gbuf->buf;

    while (temp < gbuf->bufend) {

        if ( (temp >= gbuf->gapstart) && (temp < gbuf->gapend) ) {
            putchar('_');
            temp++;
        } else {
            putchar(*(temp++));
        }

    }
    printf("\n");*/
}

//TODO: This function should be reworked/refactored. Too hard to read.
int
GapBuf_SaveToFile(GapBuf * gbuf, FILE * file, unsigned int bytes)
{
	if (!bytes) {
        return 1;
    }

    if (gbuf->point == gbuf->gapstart) {
        gbuf->point = gbuf->gapend;
    }

    if ((gbuf->gapstart > gbuf->point) && 
		(gbuf->gapstart < (gbuf->point + bytes)) && 
		(gbuf->gapstart != gbuf->gapend)) {
        if (gbuf->gapstart - gbuf->point != fwrite(gbuf->point, 1, gbuf->gapstart-gbuf->point, file)) {
            return 0;
        }

        if ((bytes - (gbuf->gapstart - gbuf->point)) != fwrite(gbuf->gapend, 1, bytes-(gbuf->gapstart - gbuf->point), file) ) {
            return 1;
        }

        return 1;
    } else {
        return bytes == fwrite(gbuf->point, 1, bytes, file);
    }
}
