#include "natcmp.h"

#include <stdlib.h>
#include <ctype.h>

/*
 * From an answer by Norman Ramsey on Stack Overflow.
 * http://stackoverflow.com/questions/1343840/natural-sort-in-c-array-of-strings-containing-numbers-and-letters
 * 
 * like strcmp but compare sequences of digits numerically
 */

int
natstrcmp(const char *s1, const char *s2)
{
	for (;;) {
		if (*s2 == '\0')
			return *s1 != '\0';
		else if (*s1 == '\0')
			return 1;
		else if (!(isdigit(*s1) && isdigit(*s2))) {
			if (*s1 != *s2)
				return (int)*s1 - (int)*s2;
			else
				(++s1, ++s2);
		} else {
			char *lim1, *lim2;
			unsigned long n1 = strtoul(s1, &lim1, 10);
			unsigned long n2 = strtoul(s2, &lim2, 10);
			if (n1 > n2)
				return 1;
			else if (n1 < n2)
				return -1;
			s1 = lim1;
			s2 = lim2;
		}
	}
}

/* wrapper for the c library qsort function */

int
natcmp(const void *p1, const void *p2)
{
	const char * const *ps1 = p1;
	const char * const *ps2 = p2;
	return natstrcmp(*ps1, *ps2);
}
