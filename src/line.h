// line.h

#ifndef CS_LINE_H
#define CS_LINE_H

typedef enum line_end_t {
	SOFT,
	HARD
} LINE_END;

typedef struct line_t {
	char * text;   //dynamic array
	int len;       //points to the next insert place (bytes)
	int size;      //current size (bytes)
	int num_chars; //number of unicode chars (not bytes)
	LINE_END end;
} Line;


char*
Line_Text(Line * line);

//constructor
Line*
Line_Init(int init_size);

//destructor
void
Line_Destroy(Line * line);

//insert utf8 character
void
Line_InsertCh(Line * line, char * ch);

//insert utf8 string
void
Line_InsertStr(Line * line, char * str);

//deletes one utf8 character from the line
void
Line_DeleteCh(Line * line);

#endif
