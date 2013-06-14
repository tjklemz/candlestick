// line.c

#include "line.h"

#include "utf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//should rename to Line_TextRaw or something
char*
Line_Text(Line * line)
{
	return line->text;
}

Line*
Line_Init(int init_size)
{
	Line * line = (Line *)malloc(sizeof(Line));
	line->text = (char *)calloc(init_size + 1, sizeof(char));
	line->text[0] = '\0';
	line->size = init_size + 1;
	line->len = 0;
	line->num_chars = 0;
	line->end = SOFT;
	
	return line;
}

void
Line_Destroy(Line * line)
{
	if(line) {
		free(line->text);
		free(line);
	
		line = NULL;
	}
}

void
Line_InsertCh(Line * line, char * ch)
{
	if(*ch) {
		//printf("strlen: %d\n", (int)strlen(ch));
		for(; *ch; ++ch) {
			if(line->len == line->size - 1) {
				//printf("oh ya, realloc...\n");
				line->text = (char *)realloc(line->text, (line->size + 1)*sizeof(char));
				line->size += 1;
			}
			line->text[line->len] = *ch;
			line->len += 1;
			line->text[line->len] = '\0';
		}
		
		line->num_chars += 1; //inserted one unicode character
	}
}

void
Line_InsertStr(Line * line, char * str)
{
	int c;
	size_t n;
	char ch[7];
	Rune rune;

	n = 0;
	for(;;) {
		c = *(unsigned char*)str;
		
		if(c < Runeself) {
			if(c == 0)
				break;
			n = 1;
			ch[0] = (char)c;
			ch[1] = '\0';
		} else {
			n = chartorune(&rune, str);
			memcpy(ch, str, n);
			ch[n] = '\0';
		}
		
		Line_InsertCh(line, ch);
		
		str += n;
	}
}

void
Line_DeleteCh(Line * line)
{
	int cur = line->len;
	
	if(cur > 0) {
		char cur_byte;
		char prev_byte;
		//if the current byte is negative, then it is unicode
		// so move to the appropriate position
		do {
			--cur;
			
			prev_byte = line->text[cur-1];
			cur_byte = line->text[cur];
			
			if(cur_byte < 0 && !(cur_byte & (1 << 6))) {
				cur_byte = -1; //(1 << 7);
			}
			if(prev_byte < 0 && !(prev_byte & (1 << 6))) {
				prev_byte = -1; //(1 << 7);
			}
		} while(cur > 0 && cur_byte < 0 && prev_byte <= cur_byte);
		
		line->len = cur;
		line->text[cur] = '\0';
		line->num_chars -= 1; //deleted one unicode character
	}
}

