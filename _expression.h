#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include "operation.h"
#include "variable.h"

struct expression {
	char *content;				/* in human-readible expression */
	OP *op;
	VAR *var;					/* only for atom */
	struct expression *x1;		/* NULL if atom */
	struct expression *x2;		/* NULL if atom */
};

typedef struct expression EXPR;

#endif	/* _EXPRESSION_H */
