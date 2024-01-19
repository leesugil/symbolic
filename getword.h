/* this is an improved version of getword shown in the book that only utilizes the standard C libraries */

#ifndef GETWORD_H
#define GETWORD_H

#include <ctype.h>
#include <stdio.h>

/* getword: get next word or character from input */
int getword(char *word, int lim)
{
	int c;
	char *w = word;

	while (isspace(c = getchar()))
		;
	if (c != EOF)
		*w++ = c;
	if (!isalpha(c)) {
		*w = '\0';
		return c;
	}
	for ( ; --lim > 0; w++)
		if (!isalnum(*w = getchar())) {
			ungetc(*w, stdin);
			break;
		}
	*w = '\0';
	return word[0];
}

#endif	/* GETWORD_H */
