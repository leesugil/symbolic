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
	char name[MAXCHAR];
	char op[MAXCHAR];
	Expr *left;
	Expr *right;
};

static Expr *exprAlloc(void)
{
	return (Expr *) malloc(sizeof(Expr));
}

void parseExprOp(char [], char []);
char *parseExprLeft(char *);
char *parseExprRight(char *);

Expr *addExpr(Expr *p, char *name)
{

	if (p == NULL) {
		p = exprAlloc();
		strcpy(p->name, name);
		parseExprOp(p->op, p->name);
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
void parseExprOp(char op[], char line[])
{
	/* (x + y) * z	-> strstrmaskblk will do.
	 * x + (y + z) * z -> strstrmaskblk will do.
	 * (x + y) * (x + z)	-> strstrmaskblk will do.
	 * (x + y * z)	-> strstrmaskblk will handle it!.
	 * */
	/* strstrmaskblk now works properly! */
	char *prog = "parseExprOp";
	char *p, *q = NULL;
	int i, j = 0, k;

	for (i = 0; operators[i] != NULL; i++) {
		fprintf(stderr, "%s: searching \"%s\" in \"%s\"...\n", prog, operators[i], line);
		p = strstrmaskblk(line, operators[i], block_start, block_end);
		if (p != NULL && (q == NULL || p < q)) {
			/* record the best result in q */
			q = p;
			j = i;
		}
		fprintf(stderr, "%s: best candidate so far: \"%s\"\n", prog, q);
	}

	/* q is the best candidate containing operators[j] in front */
	if (q != NULL) {
		fprintf(stderr, "%s: retrieving the first %lu characters from the best candidate...\n", prog, strlen(operators[j]));
		strcpy(op, q);
		k = strlen(q) - strlen(operators[j]);
		bcutnstr(op, k);
	}
}
void testparseExprOp(void)
{
	char line1[MAXCHAR] = "(((x + y) * y)^(y + z))";
	char line2[MAXCHAR] = "(((x + y) * y) + (y + z))";
	char line3[MAXCHAR] = "(((x + y) * y) % (y + z))";
	char line4[MAXCHAR] = "((((x + y) * y) % (y + z))";
	char op[MAXCHAR] = "";

	printf("input: \"%s\"\n", line1);
	parseExprOp(op, line1);
	printf("testparseExprOp: line1, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line2);
	op[0] = '\0';
	parseExprOp(op, line2);
	printf("testparseExprOp: line2, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line3);
	op[0] = '\0';
	parseExprOp(op, line3);
	printf("testparseExprOp: line3, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line4);
	op[0] = '\0';
	parseExprOp(op, line4);
	printf("testparseExprOp: line4, \"%s\"\n\n", op);
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
	static int tabs = 0;

	if (p != NULL) {
		printn("\t", tabs);
		printf("%s\n", p->name);
		tabs++;
		listExpr(p->left);
		listExpr(p->right);
		tabs--;
	}
}

/* removeExpr: frees a expr and its branch below */
void removeExpr(Expr *p)
{
	if (p != NULL) {
		removeExpr(p->left);
		removeExpr(p->right);
		//free(p->name);	no longer dynamically allocated
		//free(p->op);		no longer dynamically allocated
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
