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
	char *prog = "updateSymb";
	fprintf(stdout, "%s: expr->name:\"%s\"\n", prog, q->name);

	if (q == NULL)
		return p;

	p = updateSymb(p, q->left);
	p = updateSymb(p, q->right);

	if (strcmp(q->op, " = ") == 0) {
		/* add/update symbol */
		fprintf(stdout, "%s: add/update a symbol with \"%s\" = \"%s\"\n", prog, q->left->name, q->right->name);
		if (q->right == NULL) {
			/* this is a special case like expr "x = " having a NULL q->right */
			/* we send an empty string "" to addSymb */
			/* need to track how is this going to affect sybstitution */
			/* so far, p->formula == "" is not counted toward substitution in updateExpr */
			p = addSymb(p, q->left->name, "");
		} else
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
	char *prog = "updateExpr";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s:\n", prog);

	Symb *q = NULL;
	if ((q = getSymb(root, p->name)) != NULL) {
		fprintf(stderr, "%s: q->name:\"%s\"\n", prog, q->name);
		if (strcmp(q->formula, "") != 0) {
		//if (strlen(q->formula) > 0) {
			strcpy(p->name, q->formula);
			p = parseExpr(p);
		}
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
