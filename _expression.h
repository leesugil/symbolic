#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>

struct expression {
	/* expr3: op1(expr1, expr2) */
	/* all binary? */
	/* a mathematical expression can have as much sub-expressions as possible. */
	/* what should define the simplest form of a mathematical expression? */
	/* should i declare variables for all n-ary operations and the corresponding n argument storage spaces? */
	/* let's start with binary */
	/* 0-nary: not needed
	 * unary: x, -x, etc.; print variable as 0 + x, -1 * x (binary)
	 * binary: f(x, y) form
	 * ternary: can all ternary expressions be decomposed into nested binary operations? */
	struct expression *x;
	struct expression *y;
	struct expression *(*f)(struct expression *, struct *expression *);
};

typedef struct expression EXPR;

#endif	/* _EXPRESSION_H */
