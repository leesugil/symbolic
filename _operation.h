#ifndef _OPERATION_H
#define _OPERATION_H

#include "expression.h"

struct operation {
	/* assume binary */
	char *name;
	EXPR *(*df)(...);				/* display formula. pointer to a function to display the n-ary expression for human eyes */
	unsigned int isdist : 1;		/* is distributive over +? */
	unsigned int isassoc : 1;		/* f(f(a, b), c) = f(a, f(b, c)) */
	unsigned int iscomm : 1;		/* f(a, b) = f(b, a) */
	char *id;						/* unit (identity element) captured in ASCII string */
	EXPR *(*dif)(...);				/* display inverse formula. */
};

typedef struct operation OP;

#endif	/* _OPERATION_H */
