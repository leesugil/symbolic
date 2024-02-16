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
	fprintf(stderr, "%s: expr->name:\"%s\"\n", prog, q->name);

	if (q == NULL)
		return p;

	p = updateSymb(p, q->left);
	p = updateSymb(p, q->right);

	char line[MAXCHAR] = "";
	char *right = (q->right == NULL) ? NULL : q->right->name;

	if (strcmp(q->op, " = ") == 0 || strcmp(q->op, " =") == 0) {
		/* add/update symbol */
		fprintf(stderr, "%s: add/update a symbol with \"%s\" = \"%s\"\n", prog, q->left->name, q->right->name);
		p = addSymb(p, q->left->name, right);
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
	printf("--testupdateSymb\n");
	symb_tree = updateSymb(symb_tree, p);
	printf("-- listSymb\n");
	listSymb(symb_tree);

	line = "f(x, y) = x^2 + x * y + y^2";
	printf("-- the case %s\n", line);
	removeExpr(&p);
	p = addExpr(p, line);
	symb_tree = updateSymb(symb_tree, p);
	printf("-- listExpr\n");
	listExpr(p);
	printf("-- listSymb\n");
	listSymb(symb_tree);
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

Expr *__updateExpr(Expr *p, Symb *root)
{
	char *prog = "__updateExpr";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s: START\n", prog);
	fprintf(stderr, "%s: p->name:\"%s\"\n", prog, p->name);

	Symb *q = NULL;
	/* check if of the form f(x + dx, y)
	 * if so, update the function, update f in Symb, and call f */
	/* g = G * M * m / r^2
	 * g */
	char func_name[MAXCHAR] = "", symb_name[MAXCHAR] = "";
	parseFuncName(func_name, p->name);
	parseSymbName(symb_name, p->name);
	/* h(x) = f(x) */
	fprintf(stderr, "%s: func_name \"%s\" is parsed from p->name \"%s\"\n", prog, func_name, p->name);
	fprintf(stderr, "%s: symb_name \"%s\"\n", prog, symb_name);
	if (strlen(symb_name) > 0) {
		if ((q = getSymb(root, symb_name)) != NULL) {			/* f */
			fprintf(stderr, "%s: symb_name \"%s\" detected in the symbol tree\n", prog, symb_name);
			char line[MAXCHAR] = "", prev[MAXCHAR] = "";
			fprintf(stderr, "%s: evaluating \"%s\"...\n", prog, p->name);
			// the problem is when
			// the evaluated result is
			// like h ==> f((x))
			// and then reading left from here
			// withouth further updating f((x)).
			// what if h(x) = f(g(x)) so
			// h ==> f(g(x))
			//   ==> g(x)
			//   ==> x^2 + 1
			// evalSymb is required?
			// no, when evalSymb(x^2 + 1), this logic fails.
			//do {
			//	strcpy(prev, line);
			//	evalSymb(line, p->name, root);
			//	parenthstr(line);
			//	removeExpr(&p);
			//	p = addExpr(p, line);
			//} while (strcmp(prev, line) != 0);
			evalSymb(line, p->name, root);
			fprintf(stderr, "%s: ########### evaluated \"%s\":%s\n", prog, symb_name, line);
			parenthstr(line);
			removeExpr(&p);
			p = addExpr(p, line);
			listExpr(p);
		} else
			fprintf(stderr, "%s: symbol not found, no changes made\n", prog);
	}

	//fprintf(stderr, "%s: moving to left from \"%s\"\n", prog, p->name);
	//p->left = __updateExpr(p->left, root);
	//fprintf(stderr, "%s: moving to right from \"%s\"\n", prog, p->name);
	//p->right = __updateExpr(p->right, root);

	fprintf(stderr, "%s: END\n", prog);

	return p;
}
Expr *_updateExpr(Expr *p, Symb *root)
{
	char *prog = "_updateExpr";

	if (p == NULL)
		return p;

	char prev[MAXCHAR] = "";

	do {
		strcpy(prev, p->name);
		p = __updateExpr(p, root);
	} while (strcmp(prev, p->name) != 0);

	fprintf(stderr, "%s: moving to left from \"%s\"\n", prog, p->name);
	p->left = __updateExpr(p->left, root);
	fprintf(stderr, "%s: moving to right from \"%s\"\n", prog, p->name);
	p->right = __updateExpr(p->right, root);

	return p;
}
Expr *updateExpr(Expr *p, Symb *root) {
	if (p == NULL)
		return NULL;

	p = _updateExpr(p, root);
	//listExpr(p);
	p = refreshExpr(p);

	/* because it's reading the tree postorder traversally,
	 * for the following example
	 * f(x + 1, y)
	 *    /     \
	 *   f      x + 1, y
	 *            /   \
	 *         x + 1   y
	 *          / \
	 *         x   1
	 * f is read first and replaced by f(x, y).
	 * at f, there's no way to read the upper node, suggesting that
	 * postorder traversal doesn't work.
	 * should progress preorder traversally. */

	return p;
}
void testupdateExpr(void)
{
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	char *line = NULL;
	line = "x = 5, y = 6, f(x, y) = a * x^2 + b * x^1 + y";
	p = addExpr(p, line);
	printf("initial Expr provided:\n");
	listExpr(p);
	printf("\nupdated Symb tree\n");
	symb_tree = updateSymb(symb_tree, p);
	listSymb(symb_tree);
	
	removeExpr(&p);
	line = "x * f * z";
	p = addExpr(p, line);
	printf("\nmath expression (before)\n");
	listExpr(p);
	p = updateExpr(p, symb_tree);
	printf("\nmath expression (after)\n");
	listExpr(p);

	removeExpr(&p);
	line = "x * f(x + 1, y) * z";
	p = addExpr(p, line);
	printf("\nmath expression (before)\n");
	listExpr(p);
	p = updateExpr(p, symb_tree);
	printf("\nmath expression (after)\n");
	listExpr(p);
}


#endif	/* _SYMBOLIC_H */
