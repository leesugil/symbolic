#ifndef _SYMBOLIC_H
#define _SYMBOLIC_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "symbol.h"
#include "expression.h"

/* updateSymb: a function to
 * read an Expr-tree,
 * recognize p->op == " = ",
 * look up the Symb-tree,
 * add/update a Symb node */
Symb *updateSymb(Symb *p, Expr *q)
{
	if (q == NULL)
		return p;

	p = updateSymb(p, q->left);
	p = updateSymb(p, q->right);

	if (strcmp(q->op, " = ") == 0) {
		/* add/update symbol */
		p = addSymb(p, q->left->name, q->right->name);
	}

	return p;
}

/* now, updating linked symbols could be tricky.
 * if f = f(g) where g = g(h) where h = h(x, y), x = ..., for example,
 * how do we determine how many times an update should be made?
 * like while (updates-to-be-made)? */
/* f = x + y
 * x = 5
 *
 * y * x * f
 *     y --> look-up symbols --> update not needed
 *     x * f
 *         x --> look-up symbols --> update needed --> replace "x" by "5"
 *         f --> look-up symbols --> update needed --> replace "f" by "x + y"
 *           --> parse "x + y" (so Expr tree also needs to be updated here)
 *               (maybe by adding a parseExpr function in _expression.h
 *               to capsulate the parseExprLeft, parseExprOp,
 *               parseExprRight altogether.)
 *           -->
 *               x --> look-up symbols --> update needed --> replace "x" by "5"
 *               y --> look-up symbols --> update not needed
 */
/* so the key point is when update/replacement happened.
 * parseExpr must follow,
 * and then keep proceeding to the updated q->left, q->right */
int is_updated_needed(Expr *p, Symb *root)
{
	if (p == NULL)
		return 0;

	if (getSymb(root, p->name) != NULL)
		return 1;

	return 0;
}

#endif	/* _SYMBOLIC_H */
