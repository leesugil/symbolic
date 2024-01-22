#ifndef OPERATION_H
#define OPERATION_H

#include <string.h>

#include "_operation.h"
#include "expression.h"
#include "constants.h"

/* fundamental design question: how should i define EXPR properly? */

/* abinary: abstract binary */
char *abinary(EXPR *a, EXPR *b)
{
	int n;
	n = 2 + strlen(a->content) + 1;
	if (b != NULL)
		n += 2 + strlen(b->content);
	char w[n+1];
	w[0] = '\0';
	strcat(w, "f(");
	strcat(w, a->content);
	if (b != NULL) {
		strcat(w, ", ");
		strcat(w, b->content);
	}
	strcat(w, ")");
	return strdup(w);
}

/* aunary: abstract unary */
char *aunary(EXPR *a)
{
	return abinary(a, NULL);
}


#endif	/* OPERATION_H */
