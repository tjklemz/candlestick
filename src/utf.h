#ifndef _UTF_H_
#define _UTF_H_ 1

#include <stdlib.h>
#include <sys/types.h>
#ifdef _MSC_VER
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
#ifdef _WIN64
#define ssize_t __int64
#else
#define ssize_t int
#endif
typedef unsigned char bool;
enum {false, true};
#else
#include <stdbool.h>
#include <inttypes.h>
#endif

typedef unsigned int Rune;	/* 32 bits */

enum
{
	UTFmax		= 4,		/* maximum bytes per rune */
	Runesync	= 0x80,		/* cannot represent part of a UTF sequence (<) */
	Runeself	= 0x80,		/* rune and UTF sequences are the same (<) */
	Runeerror	= 0xFFFD,	/* decoding error in UTF */
	Runemax = 0x10FFFF	/* maximum rune value */
};

/* Edit .+1,/^$/ | cfn $PLAN9/src/lib9/utf/?*.c | grep -v static |grep -v __ */
int		chartorune(Rune *rune, char *str);
int		fullrune(char *str, int n);
int		isalpharune(Rune c);
int		islowerrune(Rune c);
int		isspacerune(Rune c);
int		istitlerune(Rune c);
int		isupperrune(Rune c);
int		runelen(long c);
int		runenlen(Rune *r, int nrune);
Rune*		runestrcat(Rune *s1, Rune *s2);
Rune*		runestrchr(Rune *s, Rune c);
int		runestrcmp(Rune *s1, Rune *s2);
Rune*		runestrcpy(Rune *s1, Rune *s2);
Rune*		runestrdup(Rune *s) ;
Rune*		runestrecpy(Rune *s1, Rune *es1, Rune *s2);
long		runestrlen(Rune *s);
Rune*		runestrncat(Rune *s1, Rune *s2, long n);
int		runestrncmp(Rune *s1, Rune *s2, long n);
Rune*		runestrncpy(Rune *s1, Rune *s2, long n);
Rune*		runestrrchr(Rune *s, Rune c);
Rune*		runestrstr(Rune *s1, Rune *s2);
int		runetochar(char *str, Rune *rune);
Rune		tolowerrune(Rune c);
Rune		totitlerune(Rune c);
Rune		toupperrune(Rune c);
char*		utfecpy(char *to, char *e, char *from);
int		utflen(char *s);
int		utfnlen(const char *s, long m);
char*		utfrrune(char *s, long c);
char*		utfrune(char *s, long c);
char*		utfutf(char *s1, char *s2);

char * utf8_from_ucs(char *buffer, int *str, int n);
int utf8proc_encode_char(int32_t uc, char * dst);

#endif
