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
void testupdateSymb(void)
{
	Symb *p = NULL;
	Expr *q = NULL;

	char *line = NULL;
	line = "x = 5, y = 6, f = (x + y)";
	q = addExpr(q, line);
	listExpr(q);
	listSymb(p);
	p = updateSymb(p, q);
	listSymb(p);
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
/* parseExpr added */

Expr *updateExpr(Expr *p, Symb *root)
{
	if (p == NULL)
		return NULL;

	Symb *q = NULL;
	if ((q = getSymb(root, p->name)) != NULL) {
		strcpy(p->name, q->formula);
		p = parseExpr(p);
	}

	updateExpr(p->left, root);
	updateExpr(p->right, root);

	return p;
}
void testupdateExpr(void)
{
	Symb *root = NULL;
	Expr *p = NULL;

	char *line = NULL;
	line = "x = 5, y = 6, f = (x + y)";
	p = addExpr(p, line);
	printf("symbol registration\n");
	listExpr(p);
	root = updateSymb(root, p);
	printf("\nsymbol tree\n");
	listSymb(root);
	
	removeExpr(&p);
	line = "x * f * z";
	p = addExpr(p, line);
	printf("\nmath expression (before)\n");
	listExpr(p);
	p = updateExpr(p, root);
	printf("\nmath expression (after)\n");
	listExpr(p);
}

void testevalExpr2(void)
{
	Symb *root = NULL;
	Expr *p = NULL;

	char *line = NULL;
	line = "x = 5, y = 6, f = (x + y), g = f / z";
	p = addExpr(p, line);
	printf("symbol registration\n");
	listExpr(p);
	root = updateSymb(root, p);
	printf("\nsymbol tree\n");
	listSymb(root);
	
	removeExpr(&p);
	line = "x * (f * z)^g";
	p = addExpr(p, line);
	printf("\nmath expression (before)\n");
	listExpr(p);
	p = updateExpr(p, root);
	printf("\nmath expression (after)\n");
	listExpr(p);

	p = evalExpr(p);
	printf("\nevaluated math expression (after)\n");
	listExpr(p);
}

#endif	/* _SYMBOLIC_H */
