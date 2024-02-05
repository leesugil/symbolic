#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "qol/c/getword.h"

#include "operation.h"

extern Op *op_tree;

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

char *block_start[] = {
	"(",
	"[",
	"{",
	NULL
};

char *block_end[] = {
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

static char *operators1[] = {
	" + ",
	" - ",
	NULL
};

static char *operators2[] = {
	" / ",
	NULL
};

static char *operators3[] = {
	" * ",
	NULL
};

static char *operators4[] = {
	" % ",
	NULL
};

static char *operators5[] = {
	"^",
	NULL
};

/* with ", " as a divider (instead of "; ", for example), non-dividing expressions like f = f(x, y) or v = (1.0, 0.0, 0.0) could be challenging. - future project. maybe just use strstrmaskblk */
static char *dividers[] = {
	", ",
	NULL
};

static char *definitions[] = {
	" = ",
	NULL
};

static char *comparisons[] = {
	" == ",
	" > ",
	" >= ",
	" < ",
	" <= ",
	" != ",
	NULL
};

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

Expr *parseExpr(Expr *p);

Expr *addExpr(Expr *p, char *name)
{
	char *prog = "addExpr";

	if (p == NULL) {
		p = exprAlloc();
		strcpy(p->name, name);
		if (p->name[strlen(p->name)-1] == '\n')
			p->name[strlen(p->name)-1] = '\0';
		p->op[0] = '\0';
		p->left = NULL;
		p->right = NULL;

		p = parseExpr(p);

		fprintf(stderr, "%s: \"%s\" added to the tree\n", prog, p->name);
	}

	return p;
}

/* parseExpr: capsulates remove_outer_block, parseExprOp, parseExprLeft, parseExprRight */
Expr *parseExprBinOp(Expr *p, char *op);
void refreshExprTree(Expr **p);

Expr *parseExpr(Expr *p)
{
	char *prog = "parseExpr";

	if (p == NULL)
		return NULL;

	if (!is_blocked_properly_blk(p->name, block_start, block_end, NULL)) {
		return p;
	}

	// make sure there're no redundant outer blocks
	while (is_outer_blocked_blk(p->name, block_start, block_end, NULL)) {
		remove_outer_block_blk(p->name, block_start, block_end);
	}

	/* see if it's about definition,
	 * first by commas ',' */
	/* if not, see if it has LHS and RHS */
	/* if not, start parsing operations */
	if (strcmp(p->name, "") != 0) {
		/* check dividers ", " */
		p = parseExprBinOp(p, ", ");
		if (strcmp(p->op, "") == 0) {
			/* check definitions " = " */
			p = parseExprBinOp(p, " = ");
		}
		if (strcmp(p->op, "") == 0) {
			/* check LHS, RHS, " == " */
			p = parseExprBinOp(p, " == ");
		}
		if (strcmp(p->op, "") == 0) {
			/* regular math expressions */
			p = parseExprBinOp(p, " - ");
			p = parseExprBinOp(p, " + ");
			p = parseExprBinOp(p, " / ");
			p = parseExprBinOp(p, " * ");
			p = parseExprBinOp(p, "^");
			p = parseExprBinOp(p, " % ");
		}
	} else {
		/* p->name == "" */
		fprintf(stderr, "%s: example) F = G * m * M / r^2\n", prog);
	}

	return p;
}

void parseExprOp(char w[], char line[], Op *op, char **pre, char **suf);
void parseExprLeft(char lhs[], char *line, Op *op, char **pre, char **suf);
void parseExprRight(char rhs[], char *line, Op *op, char **pre, char **suf);
/* parseExprBinOp */
Expr *parseExprBinOp(Expr *p, char *op)
{
	char *prog = "parseExprBinOp";

	if (p == NULL)
		return p;
	if (strlen(p->op) > 0)
		return p;

	char binop[MAXCHAR] = "";
	char left[MAXCHAR] = "";
	char right[MAXCHAR] = "";

	/* op parsing first as left- and right-parsing are based on p->op */
	Op *opp = getOp(op_tree, op);
	if (opp != NULL) {
		parseExprOp(binop, p->name, opp, block_start, block_end);
		if (binop[0] != '\0') {
			strcpy(p->op, binop);
			parseExprLeft(left, p->name, opp, block_start, block_end);
			if (left[0] != '\0')
				p->left = addExpr(p->left, left);
			parseExprRight(right, p->name, opp, block_start, block_end);
			if (right[0] != '\0')
				p->right = addExpr(p->right, right);
		}
	}

	return p;
}

void listExpr(Expr *p);
Expr *evalExpr(Expr *p);
Expr *_distExpr(Expr *p, char *prod, char *sum);
void testparseExpr(void)
{
	op_tree = loadOps(op_tree);
	//char *line = "a * x^2 + b * x^1 + c * x^0";
	char *line = "x * (a * (b - c) / d / e) / y";
	//char *line = "a / (b - c)";
	//char *line = "a * ((b / d) - (c / d))";
	//char *line = "(b / d) - (c / d)";
	//char *line = "x * a * (b - c) / d / y * z + x";
	Expr *expr = NULL;

	expr = addExpr(expr, line);

	printf("original expr:\n");
	listExpr(expr);
	printf("---\n");
}

/* parseExprOp: if undetected, w = "" */
// in the current version, every operation is assumed to be right-associative.
// this algo should be updated to be more complicated so that
// left-assoc and right-assoc rules are applied properly.
void parseExprOp(char w[], char line[], Op *op, char **b_start, char **b_end)
{
	char *prog = "parseExprOp";

	char *p = NULL;
	w[0] = '\0';

	if (op == NULL)
		return;
	if (op->right_assoc != NULL) {
		p = strstrmaskblk(line, op->name, NULL, b_start, b_end);
	} else if (op->left_assoc != NULL) {
		p = strrstrmaskblk(line, op->name, NULL, b_start, b_end);
	}
	if (p != NULL) {
		/* this makes parseExprLeft and parseExprRight to trigger */
		strcpy(w, p);
		firstnstr(w, strlen(op->name));
	}
}
void testparseExprOp(void)
{
	char line1[MAXCHAR] = "(((x + y) * y)^(y + z))";
	char line2[MAXCHAR] = "(((x + y) * y) + (y + z))";
	char line3[MAXCHAR] = "(((x + y) * y) % (y + z))";
	char line4[MAXCHAR] = "((((x + y) * y) % (y + z))";
	char line5[MAXCHAR] = "f = (((x + y) * y), g = (y + z))";
	char w[MAXCHAR] = "";
	op_tree = loadOps(op_tree);
	char *name = " + ";
	Op *op = getOp(op_tree, name);

	parseExprOp(w, line1, op, block_start, block_end);
	printf("input: \"%s\", \"%s\"\n", name, line1);
	printf("testparseExprOp: line1, \"%s\"\n\n", w);

	parseExprOp(w, line2, op, block_start, block_end);
	printf("input: \"%s\", \"%s\"\n", name, line2);
	printf("testparseExprOp: line2, \"%s\"\n\n", w);

	parseExprOp(w, line3, op, block_start, block_end);
	printf("input: \"%s\", \"%s\"\n", name, line3);
	printf("testparseExprOp: line3, \"%s\"\n\n", w);
}

/* parseExprLeft: whether the condition given by the " = "-type or the " == "-type, the function parses the relevant LHS part.
 * if undetected, w = "" */
void parseExprLeft(char w[], char *line, Op *op, char **b_start, char **b_end)
{
	char *prog = "parseExprLeft";

	char *p = NULL;
	w[0] = '\0';

	if (op == NULL)
		return;
	if (op->right_assoc != NULL) {
		p = strstrmaskblk(line, op->name, NULL, b_start, b_end);
	} else if (op->left_assoc != NULL) {
		p = strrstrmaskblk(line, op->name, NULL, b_start, b_end);
	}
	if (p != NULL) {	// should be always true if input is correct
		strcpy(w, line);
		firstnstr(w, strlen(line) - strlen(p));
		// make sure there're no redundant outer blocks
		while (is_outer_blocked_blk(w, block_start, block_end, NULL)) {
			remove_outer_block_blk(w, block_start, block_end);
		}
	}
}
void testparseExprLeft(void)
{
	char *line1 = "this is a test LHS = this is a test RHS";
	char *line2 = "this is a test LHS == this is a test RHS";
	char w[MAXCHAR] = "";
	op_tree = loadOps(op_tree);
	char *name = " = ";
	Op *op = getOp(op_tree, name);

	parseExprLeft(w, line1, op, block_start, block_end);
	printf("input: \"%s\", \"%s\"\n", name, line1);
	printf("testparseexprleft: \"%s\"\n", w);

	parseExprLeft(w, line2, op, block_start, block_end);
	printf("input: \"%s\", \"%s\"\n", name, line2);
	printf("testparseexprleft: \"%s\"\n", w);
}

/* parseExprRight: if undetected, w = "" */
// in the current version, every operation is assumed to be right-associative.
// this algo should be updated to be more complicated so that
// left-assoc and right-assoc rules are applied properly.
void parseExprRight(char w[], char *line, Op *op, char **b_start, char **b_end)
{
	char *prog = "parseExprRight";

	char *p = NULL;
	w[0] = '\0';

	if (op == NULL)
		return;
	if (op->right_assoc != NULL) {
		p = strstrmaskblk(line, op->name, NULL, b_start, b_end);
	} else if (op->left_assoc != NULL) {
		p = strrstrmaskblk(line, op->name, NULL, b_start, b_end);
	}
	if (p != NULL) {
		strcpy(w, p);
		fcutnstr(w, strlen(op->name));
		// make sure there're no redundant outer blocks
		while (is_outer_blocked_blk(w, block_start, block_end, NULL)) {
			remove_outer_block_blk(w, block_start, block_end);
		}
	}
}
void testparseExprRight(void)
{
	char *line1 = "this is a test LHS = this is a test RHS";
	char w[MAXCHAR] = "";
	op_tree = loadOps(op_tree);
	char *name = " = ";
	Op *op = getOp(op_tree, name);

	printf("input: \"%s\", \"%s\"\n", name, line1);
	parseExprRight(w, line1, op, block_start, block_end);
	printf("testparseexprright: \"%s\"\n", w);

	char *line2 = "this is a test LHS >= this is a test RHS";
	w[0] = '\0';

	name = " >= ";
	op = getOp(op_tree, name);
	printf("input: \"%s\", \"%s\"\n", name, line2);
	parseExprRight(w, line2, op, block_start, block_end);
	printf("testparseexprright: \"%s\"\n", w);

	char *line3 = "this is a test LHS > this is a test RHS";
	w[0] = '\0';

	name = " >= ";
	op = getOp(op_tree, name);
	printf("input: \"%s\", \"%s\"\n", name, line3);
	parseExprRight(w, line3, op, block_start, block_end);
	printf("testparseexprright: \"%s\"\n", w);
}

/* listExpr: in-order print of tree p */
void listExpr(Expr *p)
{
	static int tabs = 0;

	if (p != NULL) {
		printn("\t", tabs);
		printf("name:\"%s\"\n", p->name);
		tabs++;
		listExpr(p->left);
		if (strlen(p->op) > 0) {
			printn("\t", tabs);
			printf("op:\"%s\"\n", p->op);
		}
		listExpr(p->right);
		tabs--;
	}
}

/* _removeExpr: used in removeExpr */
void _removeExpr(Expr *p)
{
	if (p != NULL) {
		_removeExpr(p->left);
		_removeExpr(p->right);
		p->left = NULL;
		p->right = NULL;
		strcpy(p->name, "");
		strcpy(p->op, "");
		free(p);
	}
}

/* removeExpr: frees an expr and its branch below */
void removeExpr(Expr **p)
{
	_removeExpr(*p);
	*p = NULL;
}

/* evalExpr: evaluates an expr explicitly with parenthesis */
// is this really needed?
// this does the strict parenthesis job.
// a * b * c ==> a * (b * c)
// in the current version, every operation is assumed to be right-associative.
// this algo should be updated to be more complicated so that
// left-assoc and right-assoc rules are applied properly.
Expr *evalExpr(Expr *p)
{
	if (p == NULL)
		return NULL;
	
	p->left = evalExpr(p->left);
	p->right = evalExpr(p->right);

	if (p->left != NULL) {
		strcpy(p->name, p->left->name);
		if ((strcmp(p->left->op, "")
				* strcmp(p->op, ", ")
				* strcmp(p->op, " = ")
				* strcmp(p->op, " == ")
				* strcmp(p->op, " >= ")
				* strcmp(p->op, " > ")
				* strcmp(p->op, " <= ")
				* strcmp(p->op, " < ")) != 0)
			parenthstr(p->name);
	}
	if (strlen(p->op) > 0) {
		strcat(p->name, p->op);
	}
	if (p->right != NULL) {
		char right[MAXCHAR] = "";
		strcpy(right, p->right->name);
		if ((strcmp(p->right->op, "")
				* strcmp(p->op, ", ")
				* strcmp(p->op, " = ")
				* strcmp(p->op, " == ")
				* strcmp(p->op, " >= ")
				* strcmp(p->op, " > ")
				* strcmp(p->op, " <= ")
				* strcmp(p->op, " < ")) != 0)
			parenthstr(right);
		strcat(p->name, right);
	}

	return p;
}
void testevalExpr(void)
{
	//char line[MAXCHAR] = "(x + ((x + y) + (y + z)))";
	char line[MAXCHAR] = "a * x^2 + b * x^1 + c * x^0";
	Expr *p = NULL;
	p = addExpr(p, line);
	printf("\nevalExpr (before)\n");
	listExpr(p);
	p = evalExpr(p);
	printf("\nevalExpr (after)\n");
	listExpr(p);
}

Expr *refreshExpr(Expr *p)
{
	if (p->left == NULL || p-> right == NULL)
		return p;

	if (strcmp(p->op, "") == 0)
		return p;

	char dum_line[MAXCHAR] = "";

	strcpy(p->name, p->left->name);
	if (strcmp(p->op, " * ") == 0 ||
				strcmp(p->op, " / ") == 0) {
		if (strcmp(p->left->op, " + ") == 0 ||
				strcmp(p->left->op, " - ") == 0)
			parenthstr(p->name);
	} else if (strcmp(p->op, " % ") == 0 ||
			strcmp(p->op, "^") == 0)
		if (strcmp(p->left->op, "") != 0)
			parenthstr(p->name);

	strcat(p->name, p->op);

	strcpy(dum_line, p->right->name);
	if (strcmp(p->op, " * ") == 0 ||
				strcmp(p->op, " / ") == 0) {
		if (strcmp(p->right->op, " + ") == 0 ||
				strcmp(p->right->op, " - ") == 0)
			parenthstr(dum_line);
	} else if (strcmp(p->op, " % ") == 0 ||
			strcmp(p->op, "^") == 0)
		if (strcmp(p->left->op, "") != 0)
			parenthstr(dum_line);
	strcat(p->name, dum_line);

	return p;
}

Expr *_distExprLeft2Right(Expr *p, char *prod, char *sum)
{
	char *prog = "_distExprLeft2Right";

	if (p == NULL)
		return NULL;

	if (p->left != NULL)
		p->left = _distExprLeft2Right(p->left, prod, sum);
	if (p->right != NULL)
		p->right = _distExprLeft2Right(p->right, prod, sum);

	char line[MAXCHAR] = "";
	char left[MAXCHAR] = "";
	char right[MAXCHAR] = "";
	char dum_line[MAXCHAR] = "";

	/* just update p->name based on p->left and p->right */
	p = refreshExpr(p);

	if (strcmp(p->op, prod) == 0) {
		if (p->left != NULL && p->right != NULL) {
			/* left-to-right distribution */
			if (strcmp(p->right->op, sum) == 0) {
				/* there is an operation in RHS to distribute the primary operation over */
				/* (p->left->name p->op parseExprLeftBy) parseExprOpBy (p->left->name p->op parseExprRightBy) */
				// "left" will be the p->right->left->name part
				Op *q = getOp(op_tree, sum);
				parseExprLeft(left, p->right->name, q, block_start, block_end);
				// "right" will be the p->right->right->name part
				parseExprRight(right, p->right->name, q, block_start, block_end);
				/* (a + b) * (c + d + e) */
				strcpy(dum_line, p->left->name);	// a + b
				if (strcmp(p->left->op, "") != 0)
					parenthstr(dum_line);			// (a + b)
				strcat(dum_line, p->op);			// (a + b) * 
				if (strcmp(p->right->op, "") != 0)
					parenthstr(left);				// was this right??
				strcat(dum_line, left);				// (a + b) * c
				parenthstr(dum_line);				// ((a+b)*c)
				strcpy(line, dum_line);				// skipping the space

				strcat(line, sum);					// ((a+b)*c)+

				strcpy(dum_line, p->left->name);	// a+b
				if (strcmp(p->left->op, "") != 0)
					parenthstr(dum_line);			// (a+b)
				strcat(dum_line, p->op);			// (a+b)*
				if (strcmp(p->right->op, "") != 0)
					parenthstr(right);
				strcat(dum_line, right);			// (a+b)*d+e
				parenthstr(dum_line);				// ((a+b)*d+e)
				strcat(line, dum_line);				// ((a+b)*c)+((a+b)*d+e)

				removeExpr(&p);
				p = addExpr(p, line);
			}
		}
	}

	/*
	if (p->left != NULL)
		p->left = _distExprLeft2Right(p->left, prod, sum);
	if (p->right != NULL)
		p->right = _distExprLeft2Right(p->right, prod, sum);
	*/

	return p;
}
void test_distExprLeft2Right(void)
{
	char *line = "(x * (y + z)) * (a * (b + c))";
	Expr *p = NULL;

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	p = _distExprLeft2Right(p, " * ", " + ");

	printf("test_distExprLeft2Right\n");
	listExpr(p);
	printf("---\n");
}

Expr *_distExprRight2Left(Expr *p, char *prod, char *sum)
{
	char *prog = "_distExprRight2Left";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s: considering \"%s\"\n", prog, p->name);

	if (p->left != NULL)
		p->left = _distExprRight2Left(p->left, prod, sum);
	if (p->right != NULL)
		p->right = _distExprRight2Left(p->right, prod, sum);

	fprintf(stderr, "%s: back on \"%s\"\n", prog, p->name);

	char line[MAXCHAR] = "";
	char left[MAXCHAR] = "";
	char right[MAXCHAR] = "";
	char dum_line[MAXCHAR] = "";
	/* realized that the function's supposed to update p->name even if there's nothing to distribute at the current level because of updates made in p->left and p->right */

	/* just update p->name based on p->left and p->right */
	p = refreshExpr(p);

	if (strcmp(p->op, prod) == 0) {
		if (p->left != NULL && p->right != NULL) {
			/* right-to-left distribution */
			if (strcmp(p->left->op, sum) == 0) {
				/* there is an operation in LHS to distribute the primary operation over */
				/* (p->right->name p->op parseExprLeftBy) parseExprOpBy (p->right->name p->op parseExprRightBy) */
				// "left" will be the p->left->left->name part
				Op *q = getOp(op_tree, sum);
				parseExprLeft(left, p->left->name, q, block_start, block_end);
				// "right" will be the p->left->right->name part
				parseExprRight(right, p->left->name, q, block_start, block_end);
				/* (a + b + c) * d */
				if (strcmp(p->left->op, "") != 0)
					parenthstr(left);
				strcpy(line, left);
				strcat(line, p->op);
				strcpy(dum_line, p->right->name);
				if (strcmp(p->right->op, "") != 0)
					parenthstr(dum_line);
				strcat(line, dum_line);
				parenthstr(line);

				strcat(line, sum);

				char dum_line2[MAXCHAR] = "";
				if (strcmp(p->left->op, "") != 0)
					parenthstr(right);
				strcpy(dum_line, right);
				strcat(dum_line, p->op);
				strcpy(dum_line2, p->right->name);
				if (strcmp(p->right->op, "") != 0)
					parenthstr(dum_line2);
				strcat(dum_line, dum_line2);
				parenthstr(dum_line);
				strcat(line, dum_line);

				removeExpr(&p);
				p = addExpr(p, line);
			}
		}
	}

	/* postorder traversal needed
	if (p->left != NULL)
		p->left = _distExprRight2Left(p->left, prod, sum);
	if (p->right != NULL)
		p->right = _distExprRight2Left(p->right, prod, sum);
	*/

	return p;
}
void test_distExprRight2Left(void)
{
	//char *line = "((x + y) * z) * ((a + b) * c)";
	char *line = "(((x + y) * z) * (a * b)) + (((x + y) * z) * (a * c))";
	Expr *p = NULL;

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	p = _distExprRight2Left(p, " * ", " + ");

	printf("test_distExprRight2Left\n");
	listExpr(p);
	printf("---\n");
}

Expr *_distExpr(Expr *p, char *prod, char *sum)
{
	char *prog = "_distExpr";

	if (p == NULL)
		return p;

	char dum_line[MAXCHAR] = "";
	strcpy(dum_line, p->name);

	p = _distExprLeft2Right(p, prod, sum);
	p = _distExprRight2Left(p, prod, sum);

	int c = 1;

	fprintf(stderr, "%s:%s\n", prog, dum_line);

	while (strcmp(p->name, dum_line) != 0) {
		strcpy(dum_line, p->name);
		fprintf(stderr, "%s:%s\n", prog, dum_line);
		p = _distExprLeft2Right(p, prod, sum);
		p = _distExprRight2Left(p, prod, sum);
		c++;
	}

	fprintf(stderr, "%s: times right-and-left distribution set executed: %d\n", prog, c);

	refreshExprTree(&p);

	return p;
}
void test_distExpr(void)
{
	//char *line = "((x + y) * z) * (a * (b + c))";
	//char *line = "(x + y) * z";
	//char *line = "(2 * ((5 + 6) * z))";
	//char *line = "a * ((b / d) - (c / d))";
	char *line = "x * (a * (b - c) / d) / y";
	Expr *p = NULL;

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	p = _distExpr(p, " * ", " + ");
	p = _distExpr(p, " * ", " - ");
	p = _distExpr(p, " / ", " + ");
	p = _distExpr(p, " / ", " - ");

	p = _distExpr(p, " * ", " + ");
	p = _distExpr(p, " * ", " - ");
	p = _distExpr(p, " / ", " + ");
	p = _distExpr(p, " / ", " - ");

	printf("test_distExpr\n");
	listExpr(p);
	printf("---\n");
}

Expr *distExpr(Expr *p)
{
	char *prog = "distExpr";

	if (p == NULL)
		return p;

	char dum_line[MAXCHAR] = "";
	strcpy(dum_line, p->name);

	p = _distExpr(p, " * ", " + ");
	p = _distExpr(p, " * ", " - ");
	p = _distExpr(p, " / ", " + ");
	p = _distExpr(p, " / ", " - ");

	int c = 1;

	fprintf(stderr, "%s:%s\n", prog, dum_line);

	while (strcmp(p->name, dum_line) != 0) {
		strcpy(dum_line, p->name);
		fprintf(stderr, "%s:%s\n", prog, dum_line);
		p = _distExpr(p, " * ", " + ");
		p = _distExpr(p, " * ", " - ");
		p = _distExpr(p, " / ", " + ");
		p = _distExpr(p, " / ", " - ");
		c++;
	}

	fprintf(stderr, "%s: times of distribution executed: %d\n", prog, c);

	return p;
}
void testdistExpr(void)
{
	//char *line = "((x + y) * z) * (a * (b + c))";
	//char *line = "(x + y) * z";
	//char *line = "(2 * ((5 + 6) * z))";
	//char *line = "a * ((b / d) - (c / d))";
	//char *line = "x * (a * (b - c) / d) / y";
	//char *line = "-1 * y * (a * x^2 + b * x^1 + c * x^0)";
	//char *line = "-1 * (a + b + c)";
	//char *line = "(a + b + c) * -1";
	char *line = "a * x + (x - y) * x";
	Expr *p = NULL;

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	p = distExpr(p);

	printf("testdistExpr\n");
	listExpr(p);
	printf("---\n");
}

Expr *_calcExprMult(Expr *p);
Expr *_calcExprDiv(Expr *p);
Expr *_calcExprAdd(Expr *p);
Expr *_calcExprSub(Expr *p);
Expr *_calcExprPow(Expr *p);
Expr *_calcExprMod(Expr *p);

Expr *calcExpr(Expr *p)
{
	char *prog = "calcExpr";

	if (p == NULL)
		return NULL;

	// postorder traverse
	fprintf(stderr, "%s: (start) p->name:%s\n", prog, p->name);

	fprintf(stderr, "%s: p->name:%s checking left\n", prog, p->name);
	p->left = calcExpr(p->left);
	fprintf(stderr, "%s: (pass) p->name:%s checking left\n", prog, p->name);
	fprintf(stderr, "%s: p->name:%s checking right\n", prog, p->name);
	p->right = calcExpr(p->right);
	fprintf(stderr, "%s: (pass) p->name:%s checking right\n", prog, p->name);

	p = refreshExpr(p);

	p = _calcExprMult(p);
	p = _calcExprDiv(p);
	p = _calcExprAdd(p);
	p = _calcExprSub(p);
	p = _calcExprPow(p);
	p = _calcExprMod(p);

	return p;
}
void testcalcExpr(void)
{
	//char *line = "2 * (3^1 % 2) + 4 / 2^2 * x - 7^0";
	//char *line = "a * x^2 + b * x^1 + c * x^0";
	//char *line = "-1 * 3 * a * 4 * 7 * -1";
	//char *line = "-1 * 3 * a * 4 * 7 * -1 + -1 * 3 * b * 2 * 7 * -1";
	//char *line = "-1 * 3 * a * 2^2 * 7 * -1";
	//char *line = "-2 * (a + b) * -1";
	char *line = "-1 * 2 * 3^2 * 4";
	Expr *p = NULL;

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	printf("after distributing\n");
	p = distExpr(p);
	listExpr(p);
	printf("---\n");

	printf("testcalcExpr\n");
	p = calcExpr(p);
	listExpr(p);
	printf("---\n");
}
Expr *_calcExprMult(Expr *p)
{
	char *prog = "_calcExprMult";

	char *op_name = " * ";

	if (p == NULL)
		return NULL;

	//fprintf(stderr, "%s: p->name:%s\n", prog, p->name);
	//fprintf(stderr, "%s: p->op:%s\n", prog, p->op);

	if (strcmp(p->op, op_name) == 0) {
		fprintf(stderr, "%s: p->name:%s\n", prog, p->name);
		fprintf(stderr, "%s: p->right->op:%s\n", prog, p->right->op);
		// after commExpr, p->left must be a number with p->op == *
		if (is_pure_number(p->left->name, NULL) == 1) {		// number
			int c = is_pure_number(p->right->name, NULL);
			char *q = NULL;
			double value = strtod(p->right->name, &q);
			if (c == 1) {	// number
				sprintf(p->name, "%g", strtod(p->left->name, NULL) * value);
				fprintf(stderr, "%s: returning \"%s\"\n", prog, p->name);
				return p;
			} else if (c == 2) {	// mixed
				if (strcmp(p->right->op, op_name) == 0) {
					sprintf(p->name, "%g", strtod(p->left->name, NULL) * value);
					// p->op == p->right->op == " * "
					strcat(p->name, q);
					fprintf(stderr, "%s: returning \"%s\"\n", prog, p->name);
					return p;
				}
			}
		}
	}

	return p;
}
Expr *_calcExprMult2(Expr *p)
{
	char *prog = "_calcExprMult";

	char *op_name = " * ";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);	// p != NULL => p->left != NULL
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {			// p->left is a number
			char *rightp = NULL;
			double right = strtod(p->right->name, &rightp);
			fprintf(stderr, "%s: rightp:%s\n", prog, rightp);
			if (strlen(rightp) == 0) {
				/* double-to-str */
				double output = left * right;
				sprintf(p->name, "%g", output);
				p->op[0] = '\0';
			}
		}
	}

	return p;
}
Expr *_calcExprDiv(Expr *p)
{
	char *prog = "_calcExprDiv";

	char *op_name = " / ";

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {
			char *rightp = NULL;
			double right = strtod(p->right->name, &rightp);
			fprintf(stderr, "%s: rightp:%s\n", prog, rightp);
			if (strlen(rightp) == 0) {
				/* double-to-str */
				double output = left / right;
				sprintf(p->name, "%g", output);
				p->op[0] = '\0';
			}
		}
	}

	return p;
}
Expr *_calcExprAdd(Expr *p)
{
	char *prog = "_calcExprAdd";

	char *op_name = " + ";

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {
			char *rightp = NULL;
			double right = strtod(p->right->name, &rightp);
			fprintf(stderr, "%s: rightp:%s\n", prog, rightp);
			if (strlen(rightp) == 0) {
				/* double-to-str */
				double output = left + right;
				sprintf(p->name, "%g", output);
				p->op[0] = '\0';
			}
		}
	}

	return p;
}
Expr *_calcExprSub(Expr *p)
{
	char *prog = "_calcExprSub";

	char *op_name = " - ";

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {
			char *rightp = NULL;
			double right = strtod(p->right->name, &rightp);
			fprintf(stderr, "%s: rightp:%s\n", prog, rightp);
			if (strlen(rightp) == 0) {
				/* double-to-str */
				double output = left - right;
				sprintf(p->name, "%g", output);
				p->op[0] = '\0';
			}
		}
	}

	return p;
}
Expr *_calcExprPow(Expr *p)
{
	char *prog = "_calcExprSub";

	char *op_name = "^";

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {
			char *rightp = NULL;
			double right = strtod(p->right->name, &rightp);
			fprintf(stderr, "%s: rightp:%s\n", prog, rightp);
			if (strlen(rightp) == 0) {
				/* double-to-str */
				double output = pow(left, right);
				sprintf(p->name, "%g", output);
				p->op[0] = '\0';
			}
		}
	}

	return p;
}
Expr *_calcExprMod(Expr *p)
{
	char *prog = "_calcExprSub";

	char *op_name = " % ";

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {
			char *rightp = NULL;
			double right = strtod(p->right->name, &rightp);
			fprintf(stderr, "%s: rightp:%s\n", prog, rightp);
			if (strlen(rightp) == 0) {
				/* double-to-str */
				double output = (int) left % (int) right;
				sprintf(p->name, "%g", output);
				p->op[0] = '\0';
			}
		}
	}

	return p;
}

void refreshExprTree(Expr **p)
{
	char line[MAXCHAR] = "";

	strcpy(line, (*p)->name);
	removeExpr(p);
	*p = addExpr(*p, line);
}

/* commExpr: would eventually handle all the commutative work.
 * current version requires distExpr running ahead of this */
// 3 * c * 4 * b * 5 ==> 3 * 4 * 5 * b * c
// 3 * c * 4 * (5 * b)		easy
// 3 * c * (4 * (5 * b))	nothing
// 3 * ((4 * (5 * b)) * c)	easy
// is_pure_number(right) == 1 ==> switch
// is_pure_number(left) == 0 ==> switch
Expr *_commExpr(Expr *p, char *op)
{
	char *prog = "_commExpr";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s: considering \"%s\" \"%s\"\n", prog, op, p->name);

	// postorder traversal
	if (p->left != NULL)
		p->left = _commExpr(p->left, op);
	if (p->right != NULL)
		p->right = _commExpr(p->right, op);

	fprintf(stderr, "%s: back on \"%s\" \"%s\"\n", prog, op, p->name);

	// refresh node to update name
	p = refreshExpr(p);

	// postorder traversal work
	if (strcmp(p->op, op) == 0) {
		if (p->left != NULL && p->right != NULL) {
			if (is_pure_number(p->right->name, NULL) == 1 ||
					is_pure_number(p->left->name, NULL) == 0) {
				/* switch */
				Expr *q = p->left;
				p->left = p->right;
				p->right = q;
				p = refreshExpr(p);
			}
		}
	}

	return p;
}
Expr *commExpr(Expr *p)
{
	char *prog = "commExpr";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s: considering \"%s\"\n", prog, p->name);

	p = _commExpr(p, " * ");
	p = _commExpr(p, " + ");

	return p;
}
void testcommExpr(void)
{
	//char *line = "((x + y) * z) * ((a + b) * c)";
	//char *line = "3 * a * 4 * b + c * 5 * d * 7";
	//char *line = "3 * a * 4 * b / c * 5 * d * 7";
	//char *line = "-1 * a * b * -1";
	//char *line = "1 + x * 2 * y - 1";
	char *line = "1 + x + -1";
	Expr *p = NULL;

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	p = commExpr(p);

	printf("testcommExpr\n");
	listExpr(p);
	printf("---\n");
}

#endif	/* _EXPRESSION_H */
