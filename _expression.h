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
}

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

static const char *const block_start[] = {
	"(",
	"[",
	"{"
};

static const char *const block_end[] = {
	")",
	"]",
	"}"
};

char *parseExprOp(char *line)
{
	/* (x + y) * z	-> strstrmaskblk will do.
	 * x + (y + z) * z -> strstrmaskblk will do.
	 * (x + y) * (x + z)	-> strstrmaskblk will fail.
	 * (x + y * z)	-> strstrmaskblk will fail.
	 * */
	/* strstrmasblk now works properly! */
}

char *parseExprLeft(char *line)
{
}

char *parseExprRight(char *line)
{
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
