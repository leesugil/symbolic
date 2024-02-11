#ifndef _SYMBOLIC_H
#define _SYMBOLIC_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "symbol.h"
#include "expression.h"
#include "function.h"










/* updateSymb: a function to
 * read an Expr-tree,
 * recognize p->op == " = ",
 * look up the Symb-tree,
 * add/update a Symb node */
Symb *updateSymb(Symb *p, Expr *q)
{
	char *prog = "updateSymb";
	fprintf(stderr, "%s: expr->name:\"%s\"\n", prog, q->name);

	if (q == NULL)
		return p;

	p = updateSymb(p, q->left);
	p = updateSymb(p, q->right);

	char line[MAXCHAR] = "";
	char func_name[MAXCHAR] = "", symb_name[MAXCHAR] = "";
	Func *r;

	if (strcmp(q->op, " = ") == 0 || strcmp(q->op, " =") == 0) {
		/* add/update symbol and function */
		fprintf(stderr, "%s: add/update a symbol with \"%s\" = \"%s\"\n", prog, q->left->name, q->right->name);
		sprintf(line, "%s", q->left->name);
		parseFuncName(func_name, line);		// f or f(
		strcpy(symb_name, func_name);
		if (symb_name[strlen(symb_name) - 1] == '(')	// f(
			bcutstr(symb_name);							// f
		if (q->right == NULL) {
			/* f or f(x, y) = */
			p = addSymb(p, symb_name, "");
			r = getFunc(func_tree, func_name);
			removeFunc(&r);
		} else {
			/* f = or f(x, y) = x^2 + x * y + y^2 */
			p = addSymb(p, symb_name, q->right->name);
			r = getFunc(func_tree, func_name);
			removeFunc(&r);
			if (strcmp(symb_name, func_name) != 0)
				func_tree = addFunc(func_tree, q->left->name, q->right->name);
		}
	}

	return p;
}
void testupdateSymb(void)
{
	Expr *p = NULL;
	op_tree = loadOps(op_tree);
	symb_tree = loadSymb(symb_tree);

	char *line = NULL;
	line = "x = 5, y = 6, f = (x + y)";
	p = addExpr(p, line);
	printf("--listExpr\n");
	listExpr(p);
	printf("--before updating symb\n");
	printf("-- listSymb\n");
	listSymb(symb_tree);
	printf("-- listFunc\n");
	listFunc(func_tree);
	printf("--testupdateSymb\n");
	symb_tree = updateSymb(symb_tree, p);
	printf("-- listSymb\n");
	listSymb(symb_tree);
	printf("-- listFunc\n");
	listFunc(func_tree);

	line = "f(x, y) = x^2 + x * y + y^2";
	printf("-- the case %s\n", line);
	removeExpr(&p);
	p = addExpr(p, line);
	symb_tree = updateSymb(symb_tree, p);
	printf("-- listExpr\n");
	listExpr(p);
	printf("-- listSymb\n");
	listSymb(symb_tree);
	printf("-- listFunc\n");
	listFunc(func_tree);
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

Expr *_updateExpr(Expr *p, Symb *root)
{
	char *prog = "_updateExpr";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s:\n", prog);

	Symb *q = NULL;
	/* check if of the form f(x + dx, y)
	 * if so, update the function, update f in Symb, and call f */
	char func_name[MAXCHAR] = "", symb_name[MAXCHAR] = "";
	parseFuncName(func_name, p->name);
	fprintf(stdout, "func_name \"%s\" is parsed from p->name \"%s\"\n", func_name, p->name);
	strcpy(symb_name, func_name);
	if (symb_name[strlen(symb_name) - 1] == '(')
		bcutstr(symb_name);
	printf("symb_name \"%s\"\n", symb_name);
	if ((q = getSymb(root, symb_name)) != NULL) {			/* f */
		fprintf(stdout, "symb_name detected in the symbol tree\n");
		/* update function */
		fprintf(stderr, "%s: q->name:\"%s\"\n", prog, q->name);
		/* update Func formula and Symb before calling it */
		func_tree = updateFunc(func_tree, p->name);

		if (strcmp(q->formula, "") != 0) {
			strcpy(p->name, q->formula);
			p = parseExpr(p);
		}
	}


	return p;
}
Expr *updateExpr(Expr *p, Symb *root) {
	if (p == NULL)
		return NULL;

	p->left = updateExpr(p->left, root);
	p->right = updateExpr(p->right, root);

	p = refreshExprNode(p);
	p = _updateExpr(p, root);

	return p;
}
void testupdateExpr(void)
{
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	char *line = NULL;
	line = "x = 5, y = 6, f = (x + y)";
	p = addExpr(p, line);
	printf("symbol registration\n");
	listExpr(p);
	symb_tree = updateSymb(symb_tree, p);
	printf("\nsymbol tree\n");
	listSymb(symb_tree);
	
	removeExpr(&p);
	line = "x * f * z";
	p = addExpr(p, line);
	printf("\nmath expression (before)\n");
	listExpr(p);
	p = updateExpr(p, symb_tree);
	printf("\nmath expression (after)\n");
	listExpr(p);
}


#endif	/* _SYMBOLIC_H */
