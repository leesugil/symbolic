#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "qol/c/getword.h"

/* Expression
 * +-----------------------------------+
 * | Divider(Expression1, Expression2) |
 * +-----------------------------------+
 *                       |
 *                       |   +-------------+
 *                       +---| Expression1 |
 *                       |   +-------------+
 *                       |   +---------+
 *                       +---| Divider |
 *                       |   +---------+
 *                       |   +-------------+
 *                       +---| Expression2 |
 *                           +-------------+
 */

/* Expression
 * +-----------------------------+
 * | x = 1.2, y = -3.4e-5, z = 3 |
 * +-----------------------------+
 *          |
 *          |   +---------+
 *          +---| x = 1.2 |
 *          |   +---------+
 *          |       |    
 *          |       |    +---+
 *          |       +----| x |
 *          |       |    +---+
 *          |       |    +-----+
 *          |       +----|  =  | Register/Update Symbol
 *          |       |    +-----+
 *          |       |    +-----+
 *          |       +----| 1.2 |
 *          |            +-----+
 *          |   +----+
 *          +---| ,  |
 *          |   +----+
 *          |   +--------------------+
 *          +---| y = -3.4e-5, z = 3 |
 *              +--------------------+
 *                  |
 *                  |
 *                 ...
 */

/* Expression
 * +---------------------------------+
 * | f(x, y) = (x^2 + y^(1 - z)) * 3 |
 * +---------------------------------+
 *           |
 *           |   +---------+
 *           +---| f(x, y) |
 *           |   +---------+
 *           |        |
 *           |        |   +---+
 *           |        +---| x |
 *           |        |   +---+
 *           |        |   +-------+
 *           |        +---| f(, ) |
 *           |        |   +-------+
 *           |        |   +---+
 *           |        +---| y |
 *           |            +---+
 *           |   +-----+
 *           +---|  =  | Register/Update Symbol
 *           |   +-----+
 *           |   +-----------------------+
 *           +---| (x^2 + y^(1 - z)) * 3 |
 *               +-----------------------+
 *                                   |
 *                                   |   +-------------------+
 *                                   +---| (x^2 + y^(1 - z)) |
 *                                   |   +-------------------+
 *                                   |          |
 *                                   |          |
 *                                   |         ...
 *                                   |   +-----+
 *                                   +---|  *  |
 *                                   |   +-----+
 *                                   |   +---+
 *                                   +---| 3 |
 *                                       +---+
 */

typedef struct Expr Expr;

struct Expr {
	char *name;
	char *op;
	Expr *left;
	Expr *right;
};

static Expr *exprAlloc(void)
{
	return (Expr *) malloc(sizeof(Expr));
}

char *parseExprOp(char *);
char *parseExprLeft(char *);
char *parseExprRight(char *);

Expr *addExpr(Expr *p, char *name)
{

	if (p == NULL) {
		p = exprAlloc();
		p->name = strdup(name);
		p->op = parseExprOp(p->name);
		p->left = NULL;
		p->right = NULL;
		p->left = addExpr(p->left, parseExprLeft(p->name));
		p->right = addExpr(p->right, parseExprRight(p->name));
	}

	return p;
}

static char *block_start[] = {
	"(",
	"[",
	"{",
	NULL
};

static char *block_end[] = {
	")",
	"]",
	"}",
	NULL
};

static char *operators[] = {
	" + ",
	" - ",
	" * ",
	" / ",
	" % ",
	"^",
	NULL
};

/* parseExprOp: from a formula, parse the primary operator part */
char *parseExprOp(char *line)
{
	/* (x + y) * z	-> strstrmaskblk will do.
	 * x + (y + z) * z -> strstrmaskblk will do.
	 * (x + y) * (x + z)	-> strstrmaskblk will do.
	 * (x + y * z)	-> strstrmaskblk will handle it!.
	 * */
	/* strstrmasblk now works properly! */
	char *prog = "parseExprOp";
	char *p, *q = NULL;
	int i, j = 0, k;

	if (line == NULL)
		return NULL;
	
	for (i = 0; operators[i] != NULL; i++) {
		fprintf(stderr, "%s: searching \"%s\" in \"%s\"...\n", prog, operators[i], line);
		p = strstrmaskblk(line, operators[i], block_start, block_end);
		fprintf(stderr, "%s: found \"%s\"\n", prog, p);
		if (p != NULL && (q == NULL || p < q)) {
			q = p;
			j = i;
		}
		fprintf(stderr, "%s: best candidate so far: \"%s\"\n", prog, q);
	}

	if (q != NULL) {
		k = strlen(q) - strlen(operators[j]);
		q = bcutnstr(q, k);

		return q;
	} else
		return NULL;
}
void testparseExprOp(void)
{
	char *line = "(((x + y) * y) + (y + z))";
	char *output = parseExprOp(line);

	printf("input: \"%s\"\n", line);
	printf("testparseExprOp: \"%s\"\n", output);

	free(output);
}

char *parseExprLeft(char *line)
{
	return NULL;
}

char *parseExprRight(char *line)
{
	return NULL;
}

/* listExpr: in-order print of tree p */
void listExpr(Expr *p)
{
	if (p != NULL) {
		printf("%s\n", p->name);
		listExpr(p->left);
		listExpr(p->right);
	}
}

/* removeExpr: frees a expr and its branch below */
void removeExpr(Expr *p)
{
	if (p != NULL) {
		removeExpr(p->left);
		removeExpr(p->right);
		free(p->name);
		free(p->op);
		free(p);
	}
}

/* removeExprBranch: frees child exprs in the branch */
void removeExprBranch(Expr *p)
{
	removeExpr(p->left);
	removeExpr(p->right);
	p->left = NULL;
	p->right = NULL;
}


#endif	/* _EXPRESSION_H */
