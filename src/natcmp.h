#ifndef CS_NATCMP_H
#define CS_NATCMP_H


// natural string compare (accounts for numbers)


/*
 * From an answer by Norman Ramsey on Stack Overflow.
 * http://stackoverflow.com/questions/1343840/natural-sort-in-c-array-of-strings-containing-numbers-and-letters
 * 
 * like strcmp but compare sequences of digits numerically
 */

int
natstrcmp(const char *s1, const char *s2);

int
natcmp(const void *p1, const void *p2);


#endif
