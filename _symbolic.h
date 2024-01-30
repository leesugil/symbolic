#ifndef _SYMBOLIC_H
#define _SYMBOLIC_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "symbol.h"
#include "expression.h"

/* a function to
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


#endif	/* _SYMBOLIC_H */
