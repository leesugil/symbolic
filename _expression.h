#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "qol/c/getword.h"
#include "qol/c/getmath.h"
#include "qol/c/tree.h"

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

extern Expr *parseExpr(Expr *p);

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
Expr *parseExprFunc(Expr *p);		// to be updated with more arguments on need

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
			p = parseExprBinOp(p, " + ");
			p = parseExprBinOp(p, " - ");
			p = parseExprBinOp(p, " / ");
			p = parseExprBinOp(p, " * ");
			p = parseExprBinOp(p, "^");
			p = parseExprBinOp(p, " % ");
		}
		if (strcmp(p->op, "") == 0) {
			/* unary operations like functions */
			p = parseExprFunc(p);
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
/* parseExprFunc: parses functions like f(x, y) */
/* design choice: save the operator p->op as " () " with the symb_name "f" on LHS (p->left) and argument list "x, y" on RHS (p->right). when calling refreshExprNode or refreshExprNodeName, when " () " is detected, implement a separate code reconstruct the name back to f(x ,y). make " () " immune to dist, comm, etc. */
Expr *parseExprFunc(Expr *p)
{
	char *prog = "parseExprFunc";

	if (p == NULL) {
		fprintf(stderr, "%s: NULL input, returning NULL\n", prog);
		return NULL;
	}
	if (strlen(p->name) == 0) {
		fprintf(stderr, "%s: strlen(p->name) == 0, returning p\n", prog);
		return p;
	}
	if (p->name[strlen(p->name) - 1] != ')') {
		fprintf(stderr, "%s: p->name[last] != ')', not a proper format to parse function notation in it:%s\n", prog, p->name);
		return p;
	}

	char func_name[MAXCHAR] = "", symb_name[MAXCHAR] = "", arg[MAXCHAR] = "";
	/* f */
	parseFuncName(func_name, p->name);
	parseSymbName(symb_name, p->name);
	/* x */
	parseFuncArg(arg, p->name);

	if (strlen(func_name) == 0) {
		fprintf(stderr, "%s: no function detected, returning p\n", prog);
		return p;
	}

	strcpy(p->op, " () ");
	p->left = addExpr(p->left, symb_name);
	p->right = addExpr(p->right, arg);

	return p;
}

void listExpr(Expr *p);
Expr *evalExpr(Expr *p);
void testparseExpr(void)
{
	op_tree = loadOps(op_tree);
	char *line = "a * x^2 + b * x^1 + c * x^0";
	line = "x * (a * (b - c) / d / e) / y";
	line = "(a - c) / (b - c) - c";
	line = "a * ((b / d) - (c / d))";
	line = "(b / d) - (c / d)";
	line = "x * a * (b - c) / d / y * z + x";
	line = "f(x + dx, y + dy)";
	line = "f(g(x + dx), y) + x + y";
	line = "x - y + 1";
	line = "x * y^-1 * z";
	line = "x + -1 * y + 1";
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

	char *q = NULL;
	w[0] = '\0';

	if (op == NULL)
		return;
	if (op->right_assoc != NULL) {
		q = strstrmaskblk(line, op->name, NULL, b_start, b_end);
	} else if (op->left_assoc != NULL) {
		q = strrstrmaskblk(line, op->name, NULL, b_start, b_end);
	}
	if (q == NULL)
		return;
	strcpy(w, line);
	firstnstr(w, strlen(line) - strlen(q));
	// make sure there're no redundant outer blocks
	while (is_outer_blocked_blk(w, block_start, block_end, NULL)) {
		remove_outer_block_blk(w, block_start, block_end);
	}
	// determine if parentheses are needed
	//if (p->left == NULL)
	//	return;
	//for (int k = 0; op->right_dist_over[k] != NULL; k++)
	//	if (strcmp(op->right_dist_over[k], p->left->op) == 0)
	//		parenthstr(x);
	//if (op->right_assoc != NULL && op->left_assoc == NULL)
	//	if (strcmp(p->op, p->left->op) == 0)
	//		parenthstr(w);			/* because otherwise
	//									   x^y^z == z^(y^z)
	//										  ^             */
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

/* parenthExprLeft: given a parsed left, determine if parentheses are needed */
int parenthExprLeft(char left[], Expr *p)
{
	char *prog = "parenthExprLeft";

	if (p == NULL)
		return 0;
	Op *op = getOp(op_tree, p->op);
	// determine if parentheses are needed
	if (p->left == NULL)
		return 0;
	for (int k = 0; op->right_dist_over[k] != NULL; k++)
		if (strcmp(op->right_dist_over[k], p->left->op) == 0) {
			parenthstr(left);
			return 1;
		}
	if (op->right_assoc != NULL && op->left_assoc == NULL)
		if (strcmp(p->op, p->left->op) == 0) {
			parenthstr(left);			/* because otherwise
										   x^y^z == z^(y^z)
											  ^             */
			return 1;
		}

	return 0;
}

/* parseExprRight: if undetected, w = "" */
// in the current version, every operation is assumed to be right-associative.
// this algo should be updated to be more complicated so that
// left-assoc and right-assoc rules are applied properly.
void parseExprRight(char w[], char *line, Op *op, char **b_start, char **b_end)
{
	char *prog = "parseExprRight";

	char *q = NULL;
	w[0] = '\0';

	if (op == NULL)
		return;
	if (op->right_assoc != NULL) {
		q = strstrmaskblk(line, op->name, NULL, b_start, b_end);
	} else if (op->left_assoc != NULL) {
		q = strrstrmaskblk(line, op->name, NULL, b_start, b_end);
	}
	if (q == NULL)
		return;
	strcpy(w, q);
	fcutnstr(w, strlen(op->name));
	// make sure there're no redundant outer blocks
	while (is_outer_blocked_blk(w, block_start, block_end, NULL)) {
		remove_outer_block_blk(w, block_start, block_end);
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

/* parenthExprRight: given a parsed right, determine if parentheses are needed */
int parenthExprRight(char right[], Expr *p)
{
	char *prog = "parenthExprRight";

	if (p == NULL)
		return 0;
	Op *op = getOp(op_tree, p->op);
	// determine if parentheses are needed
	if (p->right == NULL)
		return 0;
	for (int k = 0; op->left_dist_over[k] != NULL; k++)
		if (strcmp(op->left_dist_over[k], p->right->op) == 0) {
			parenthstr(right);
			return 1;
		}
	if (op->left_assoc != NULL && op->right_assoc == NULL)
		if (strcmp(p->op, p->right->op) == 0) {
			parenthstr(right);			/* because otherwise
										   x/y/z == (x/y)/z
											^               */
			return 1;
		}

	return 0;
}

/* listExpr: in-order print of tree p */
void listExpr(Expr *p)
{
	static int tabs = 0;

	if (p != NULL) {
		printn("\t", tabs);
		printf("\"%s\"\n", p->name);
		tabs++;
		listExpr(p->left);
		if (strlen(p->op) > 0) {
			printn("\t", tabs);
			printf("\"%s\"\n", p->op);
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

void testaddExpr(void)
{
	char *line = "y^(-1 * b)";
	line = "4^-2";
	op_tree = loadOps(op_tree);
	Expr *p = NULL;

	printf("%s\n", line);
	p = addExpr(p, line);
	listExpr(p);
}

Expr *refreshExprNode(Expr *p);
/* refreshExpr: refresh node names (and other necessary things) based on reading p->left and p->right. postorder traversal. */
// needed after distExpr
Expr *refreshExpr(Expr *p)
{
	if (p == NULL)
		return NULL;
	
	p->left = refreshExpr(p->left);
	p->right = refreshExpr(p->right);

	p = refreshExprNode(p);

	return p;
}
void testrefreshExpr(void)
{
	//char line[MAXCHAR] = "(x + ((x + y) + (y + z)))";
	//char line[MAXCHAR] = "a * x^2 + b * x^1 + c * x^0";
	char line[MAXCHAR] = "(x + x * (a - b) - x * (a + b) * y / a * b \% e)";	
	char *test_lines[] = {
		"x^y^z",
		"(x^y)^z",
		"x^(y^z)",
		"a / b / c",
		"(a / b) / c",
		"a / (b / c)",
		"a - (b - c)",
		"(a - b) - c",
		"f(x) = x + 5, x = 2, y = 3",
		"f () x, y",
		"f(x + dx, y + dy) * g(h(z + dz), 5)",
		"(x + dx) - f(x) + 1",
		NULL
	};
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	for (int i = 0; test_lines[i] != NULL; i++) {
		removeExpr(&p);
		p = addExpr(p, test_lines[i]);
		printf("\nrefreshExpr (before)\n");
		listExpr(p);
		printf("\nrefreshExpr (after)\n");
		p = refreshExpr(p);
		listExpr(p);
	}
}

/* refreshExprNode: used to update p->name when changes are made postorder traversal */
Expr *refreshExprNodeName(Expr *p);
Expr *refreshExprNode(Expr *p)
{
	if (p->left == NULL || p-> right == NULL)
		return p;

	if (strcmp(p->op, "") == 0)
		return p;

	p = refreshExprNodeName(p);

	char line[MAXCHAR] = "";
	strcpy(line, p->name);
	removeExpr(&p);
	p = addExpr(p, line);

	return p;
}
/* refreshExprNodeName: refreshExprNode but only updates name, no re-branching */
Expr *refreshExprNodeName(Expr *p)
{
	char *prog = "refreshExprNodeName";

	if (p->left == NULL || p-> right == NULL)
		return p;

	if (strcmp(p->op, "") == 0)
		return p;

	char left[MAXCHAR] = "", right[MAXCHAR] = "", line[MAXCHAR] = "";
	strcpy(left, p->left->name);
	if (strlen(p->left->op) > 0 &&
			strcmp(p->left->op, ", ") != 0 &&
			strcmp(p->left->op, " () ") != 0 &&
			strcmp(p->left->op, " = ") != 0)
		parenthstr(left);
	strcpy(right, p->right->name);
	if (strlen(p->right->op) > 0 &&
			strcmp(p->right->op, ", ") != 0 &&
			strcmp(p->right->op, " () ") != 0 &&
			strcmp(p->right->op, " = ") != 0)
		parenthstr(right);
	Op *op = getOp(op_tree, p->op);
	if (op != NULL) {
		op->char_f(p->name, left, right);
		fprintf(stderr, "%s: left: \"%s\"\n", prog, left);
		fprintf(stderr, "%s: op: \"%s\"\n", prog, op->name);
		fprintf(stderr, "%s: right: \"%s\"\n", prog, right);
	} else {
		strcpy(p->name, left);
		strcat(p->name, p->op);
		strcat(p->name, right);
	}

	return p;
}

/* altExpr: replaces the "inverse" operations of field by commutable form */
//x - y ==> x + -1 * y*/
// x / y ==> x * y^-1
Expr *altExprNode(Expr *p)
{
	char *prog = "altExprNode";

	if (p->left == NULL || p-> right == NULL)
		return p;

	if (strcmp(p->op, "") == 0)
		return p;

	Op *op = getOp(op_tree, p->op);
	if (op->char_f_alt != NULL) {
		char left[MAXCHAR] = "";
		char right[MAXCHAR] = "";
		strcpy(left, p->left->name);
		if (strlen(p->left->op) > 0)
			parenthstr(left);
		strcpy(right, p->right->name);
		if (strlen(p->right->op) > 0)
			parenthstr(right);
		char line[MAXCHAR] = "";
		op->char_f_alt(line, left, right);

		removeExpr(&p);
		p = addExpr(p, line);
	}

	return p;
}
Expr *_altExpr(Expr *p)
{
	char *prog = "altExpr";

	if (p == NULL)
		return NULL;
	
	p->left = _altExpr(p->left);
	p->right = _altExpr(p->right);

	p = refreshExprNode(p);	// update p->name and re-branch
	p = altExprNode(p);

	return p;
}
Expr *altExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	char prev[MAXCHAR] = "";

	do {
		strcpy(prev, p->name);
		p = _altExpr(p);
	} while (strcmp(prev, p->name) != 0);

	return p;
}
void testaltExpr(void)
{
	//char *line = "(a - b) * (x - y)";
	//char *line = "(a - b) * (x - y) - x / (y - z)";
	char *line = "(x * y)^a - (x / y)^b";
	line = "x - y + 1";
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	printf("testaltExpr\n");
	p = altExpr(p);
	listExpr(p);
	printf("---\n");
}

/* expExpr: exponent laws */
Expr *expExprNode(Expr *p)
{
	char *prog = "expExprNode";

	if (p->left == NULL || p-> right == NULL)
		return p;

	if (strcmp(p->op, "") == 0)
		return p;

	// x^(a + b) or (x^a)^b
	// p->op = "^"
	// check
	//       left2right_assoc_by
	//       right2left_assoc_by
	//       left_dist_over_by
	//       right_dist_over_by
	//
	//       left2right_assoc_by[0] = { "^", " * " }
	//       check p->left->op = "^"
	//             use " * " in op->right_assoc_by
	//       left_dist_over_by[0] = { " + ", " * " }
	//       check p->right->op = " + "
	//             use " * " in op->left_dist_over_char_f
	// (x^a)^(b + c)

	Op *op = getOp(op_tree, p->op);
	char line[MAXCHAR] = "", left[MAXCHAR] = "", right[MAXCHAR] = "";
	char left_left[MAXCHAR] = "", left_right[MAXCHAR] = "", right_left[MAXCHAR] = "", right_right[MAXCHAR] = "";
	int i, j = -1;

	if (op->left2right_assoc_by[0][0] != NULL && strlen(line) == 0) {
		/* (x^a)^b ==> x^(a * b) */
		for (i=0; op->left2right_assoc_by[i][0] != NULL; i++)
			if (strcmp(p->left->op, op->left2right_assoc_by[i][0]) == 0) {
				j = i;
				break;
			}
		if (j != -1) {
			/* (x^a)^b ==> x^(a * b)
			 *                  ^   */
			char *op2 = op->left2right_assoc_by[j][1];
			strcpy(right, p->right->name);				// b
			if (strlen(p->right->op) > 0)				// (b)
				parenthstr(right);
			strcpy(left_left, p->left->left->name);		// x
			if (strlen(p->left->left->op) > 0)			// (x)
				parenthstr(left_left);
			strcpy(left_right, p->left->right->name);	//	a
			if (strlen(p->left->right->op) > 0)			// (a)
				parenthstr(left_right);
			op->right_assoc_by(line, left_left, left_right, right, op2);
		}
	}
	if (op->right2left_assoc_by[0][0] != NULL && strlen(line) == 0) {
		/* hypothetically a^(b^x) ==> (a * b)^x */
		for (i=0; op->right2left_assoc_by[i][0] != NULL; i++)
			if (strcmp(p->right->op, op->right2left_assoc_by[i][0]) == 0) {
				j = i;
				break;
			}
		if (j != -1) {
			char *op2 = op->right2left_assoc_by[j][1];
			strcpy(left, p->left->name);				// a
			if (strlen(p->left->op) > 0)				// (a)
				parenthstr(left);
			strcpy(right_left, p->right->left->name);	// b
			if (strlen(p->right->left->op) > 0)			// (b)
				parenthstr(right_left);
			strcpy(right_right, p->right->right->name);	// x
			if (strlen(p->right->right->name) > 0)		// (x)
				parenthstr(right_right);
			op->left_assoc_by(line, left, right_left, right_right, op2);
		}
	}
	if (op->left_dist_over_by[0][0] != NULL && strlen(line) == 0) {
		/* x^(a + b) ==> x^a * x^b */
		for (i=0; op->left_dist_over_by[i][0] != NULL; i++)
			if (strcmp(p->right->op, op->left_dist_over_by[i][0]) == 0) {
				j = i;
				break;
			}
		if (j != -1) {
			char *op2 = op->left_dist_over_by[j][1];
			strcpy(left, p->left->name);				// x
			if (strlen(p->left->op) > 0)				// (x)
				parenthstr(left);
			strcpy(right_left, p->right->left->name);	// a
			if (strlen(p->right->left->op) > 0)			// (a)
				parenthstr(right_left);
			strcpy(right_right, p->right->right->name);	// b
			if (strlen(p->right->right->op) > 0)		// (b)
				parenthstr(right_right);
			op->left_dist_over_char_f(line, left, right_left, right_right, op2);
		}
	}
	if (op->right_dist_over_by[0][0] != NULL && strlen(line) == 0) {
		/* hypothetically (a + b)^x ==> a^x * b^x */
		for (i=0; op->right_dist_over_by[i][0] != NULL; i++)
			if (strcmp(p->left->op, op->right_dist_over_by[i][0]) == 0) {
				j = i;
				break;
			}
		if (j != -1) {
			char *op2 = op->right_dist_over_by[j][1];
			strcpy(right, p->right->name);					// x
			if (strlen(p->right->op) > 0)					// (x)
				parenthstr(right);
			strcpy(left_left, p->left->left->name);			// a
			if (strlen(p->left->left->op) > 0)				// (a)
				parenthstr(left_left);
			strcpy(left_right, p->left->right->name);		// b
			if (strlen(p->left->right->op) > 0)				// (b)
				parenthstr(left_right);
			op->right_dist_over_char_f(line, right, left_left, left_right, op2);
		}
	}

	if (strlen(line) > 0) {
		removeExpr(&p);
		p = addExpr(p, line);
		strcpy(line, p->name);
	}

	return p;
}
Expr *_expExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	p->left = _expExpr(p->left);
	p->right = _expExpr(p->right);

	p = refreshExprNode(p);
	p = expExprNode(p);

	return p;
}
Expr *expExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	char prev[MAXCHAR] = "";
	do {
		strcpy(prev, p->name);
		p = _expExpr(p);
	} while (strcmp(prev, p->name) != 0);

	return p;
}
void testexpExpr(void)
{
	char *line = "(x^a)^b";
	//line = "x^(a - b)";
	line = "(x^(a + b))^(c + d)";
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	printf("testexpExpr\n");
	p = expExpr(p);
	listExpr(p);
	printf("---\n");
}

/* distExpr: x * (a - b) ==> x * a - x * b */
Expr *distExprNode(Expr *p)
{
	char *prog = "distExprNode";

	if (p->left == NULL || p-> right == NULL)
		return p;

	if (strcmp(p->op, "") == 0)
		return p;

	// (a - b) * (x - y)
	// 			(a - b)
	// 			(x - y)
	//
	// (a - b) * x - (a - b) * y	left-dist
	// 			(a - b) * x
	// 			(a - b) * y
	// 								this is how tree gets longer necessarily,
	// 								so it's gotta be preorder traversal.
	// hold the distributed result in buffer,
	// remove the current p-branch,
	// addExpr with the line in the buffer.

	Op *op = getOp(op_tree, p->op);
	int l = -1, r = -1;
	if (op->right_dist_over[0] != NULL) {
		for (int i = 0; op->right_dist_over[i] != NULL; i++)
			if (strcmp(p->left->op, op->right_dist_over[i]) == 0) {
				l = i;
				fprintf(stderr, "%s: right distribution detected at \"%s\"\n", prog, p->name);
				break;
			}
	}
	if (op->left_dist_over[0] != NULL) {
		for (int i = 0; op->left_dist_over[i] != NULL; i++)
			if (strcmp(p->right->op, op->left_dist_over[i]) == 0) {
				r = i;
				fprintf(stderr, "%s: left distribution detected at \"%s\"\n", prog, p->name);
				break;
			}
	}

	char x[MAXCHAR] = "", y[MAXCHAR] = "";
	char left[MAXCHAR] = "", right[MAXCHAR] = "";
	char left_left[MAXCHAR] = "", left_right[MAXCHAR] = "", right_left[MAXCHAR] = "", right_right[MAXCHAR] = "";

	char line[MAXCHAR] = "";
	if (l != -1) {
		/* (a - b) * x ==> a * x - b * x */
		Op *op_left = getOp(op_tree, p->left->op);
		strcpy(right, p->right->name);				// x
		if (strlen(p->right->op) > 0)				// (x)
			parenthstr(right);
		strcpy(left_left, p->left->left->name);		// a
		if (strlen(p->left->left->op) > 0)			// (a)
			parenthstr(left_left);
		strcpy(left_right, p->left->right->name);	// b
		if (strlen(p->left->right->op) > 0)			// (b)
			parenthstr(left_right);
		op->char_f(x, left_left, right);			// a * x
		op->char_f(y, left_right, right);			// b * x
		parenthstr(x);								// (a * x)
		parenthstr(y);								// (b * x)
		op_left->char_f(line, x, y);				// a * x - b * x
	} else if (r != -1) {
		/* a * (x - y) == > a * x - b * y */
		Op *op_right = getOp(op_tree, p->right->op);
		strcpy(left, p->left->name);					// a
		if (strlen(p->left->op) > 0)					// (a)
			parenthstr(left);
		strcpy(right_left, p->right->left->name);		// x
		if (strlen(p->right->left->op) > 0)				// (x)
			parenthstr(right_left);
		strcpy(right_right, p->right->right->name);		// y
		if (strlen(p->right->right->op) > 0)			// (y)
			parenthstr(right_right);
		op->char_f(x, left, right_left);				// a * x
		op->char_f(y, left, right_right);				// a * y
		parenthstr(x);									// (a * x)
		parenthstr(y);									// (b * x)
		op_right->char_f(line, x, y);					// a * x - a * y
	}

	if (strlen(line) > 0) {
		removeExpr(&p);
		p = addExpr(p, line);
	}

	return p;
}
Expr *_distExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	p->left = _distExpr(p->left);
	p->right = _distExpr(p->right);

	p = refreshExprNode(p);
	p = distExprNode(p);

	return p;
}
Expr *distExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	char prev[MAXCHAR] = "";

	do {
		strcpy(prev, p->name);
		p = _distExpr(p);
	} while (strcmp(prev, p->name) != 0);

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
	//char *line = "a * x + (x - y) * x";
	//char *line = "(a - b) * (x - y)";
	//char *line = "(1 + (-1 * 1)) * c";
	char *line = "(x / y)^a";
	line = "(x * (y^-1))^b";
	line = "(a + b) * (c + d)";
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	printf("testdistExpr\n");
	p = distExpr(p);
	listExpr(p);
	printf("---\n");
}

int treeLength(Expr *p);
/* commutativity, associativity */
/* commutativity
 * (a * b) * (c * d)
 * (a * b) * (d * c)
 * (b * a) * (c * d)
 * (b * a) * (d * c)
 * (c * d) * (a * b)
 * (c * d) * (b * a)
 * (d * c) * (a * b)
 * (d * c) * (b * a)
 * */
/* associativity
 * (x * y) * (u * v)
 * x * (y * (u * v))
 * x * ((y * u) * v)
 * ((x * y) * u) * v
 * (x * (y * u)) * v
 * */
Expr *_commExpr(Expr *p)
{
	char *prog = "_commExpr";

	if (p == NULL)
		return NULL;

	Op *op = getOp(op_tree, p->op);
	if (op == NULL)
		return p;
	if (op->comm == NULL)
		return p;

	/* obtain variables */
	/* commutativity
	 * (a * b) * (c * d)
	 * (a * b) * (d * c)
	 * (b * a) * (c * d)
	 * (b * a) * (d * c)
	 * (c * d) * (a * b)
	 * (c * d) * (b * a)
	 * (d * c) * (a * b)
	 * (d * c) * (b * a)
	 * */
	char a[MAXCHAR] = "", b[MAXCHAR] = "", c[MAXCHAR] = "", d[MAXCHAR] = "";
	char line[MAXCHAR] = "";

	if (strcmp(p->left->op, p->op) == 0) {
		strcpy(a, p->left->left->name);
		parenthstr(a);
		strcpy(b, p->left->right->name);
		parenthstr(b);
	} else {
		strcpy(a, p->left->name);
		parenthstr(a);
		strcpy(b, op->right_unit);
	}
	if (strcmp(p->right->op, p->op) == 0) {
		strcpy(c, p->right->left->name);
		parenthstr(c);
		strcpy(d, p->right->right->name);
		parenthstr(d);
	} else {
		strcpy(c, p->right->name);
		parenthstr(c);
		strcpy(d, op->right_unit);
	}

	char x[MAXCHAR] = "", y[MAXCHAR] = "", u[MAXCHAR] = "", v[MAXCHAR] = "";
	/* check commutativity before having (b * a) */
	char *var[] = { a, b, c, d, NULL };
	char best_line[MAXCHAR] = "";
	strcpy(best_line, p->name);
	int best_length = treeLength(p);
	int length = 0;
	Expr *q = NULL;
	for (int i = 0; var[i] != NULL; i++)
		for (int j = 0; var[j] != NULL; j++)
			if (i != j)
				for (int k = 0; var[k] != NULL; k++)
					for (int l = 0; var[l] != NULL; l++)
						if (k != l && i != k && i != l && j != k && j != l) {
							strcpy(x, var[i]); strcpy(y, var[j]);
							strcpy(u, var[k]); strcpy(v, var[l]);
							fprintf(stderr, "%s: %s%s%s%s%s%s%s\n", prog, x, p->op, y, p->op, u, p->op, v);
							sprintf(line, "%s%s%s%s%s%s%s\n", x, p->op, y, p->op, u, p->op, v);
							q = addExpr(q, line);
							q = evalExpr(q);
							length = treeLength(q);
							if (length < best_length)
								strcpy(best_line, q->name);
							removeExpr(&q);
						}
	removeExpr(&p);
	p = addExpr(p, best_line);

	return p;
}
Expr *commExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	p->left = commExpr(p->left);
	p->right = commExpr(p->right);

	p = refreshExprNode(p);

	char prev[MAXCHAR] = "";

	do {
		strcpy(prev, p->name);
		p = _commExpr(p);
	} while (strcmp(prev, p->name) != 0);

	return p;
}
void testcommExpr(void)
{
	op_tree = loadOps(op_tree);
	char *line = "1 + x - 1";
	line = "(a - b) * (c * d)";
	line = "(a + b) * (1 * d)";
	line = "(1 * b) * (2 * d) * (5 * b)";
	line = "-1000 * x + x * 1000 + -999 + 1000";
	line = "x + -1 * x";
	line = "-2 * x + 2 * x";
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

int treeLength(Expr *p)
{
	char *prog = "treeLength";

	static int n = 0;
	static int length = 0;
	if (n == 0)
		length = 0;

	if (p == NULL)
		return n;
	fprintf(stderr, "%s:%d@%s, best:%d\n", prog, n, p->name, length);

	if (p->left != NULL && p->right != NULL) {
		n++;
		fprintf(stderr, "%s:%d->%d@%s\n", prog, n-1, n, p->name);
	}
	treeLength(p->left);
	treeLength(p->right);
	length++;
	if (p->left != NULL && p->right != NULL) {
		n--;
		fprintf(stderr, "%s:%d->%d@%s\n", prog, n+1, n, p->name);
	}

	return length;
}
void testtreeLength(void)
{
	Expr *p = NULL;
	char *line = "1 + 1 + 1";
	line = "1 + -1";
	line = "(a + b) * c / d";
	line = "-1 * (x * y^-1)^b + (x * y)^a";
	op_tree = loadOps(op_tree);

	p = addExpr(p, line);
	listExpr(p);
	printf("%d\n", treeLength(p));
}

/* sortExpr: rearranges variables/numbers combined with commutative binary operator so that numbers and variables are (ideally) shown in a lexiconal order. */
/* the current version does not support a perfect lexiconal order.
 * for example,
 * (a * c) * b * a
 * (a * c) * (b * a)
 * (a * c) * (a * b)
 * (a * b) * (a * c)
 * or
 * (a * c) * b
 * */
Expr *removeExprParenth(Expr *p);
Expr *_sortExpr(Expr *p);
Expr *sortExpr(Expr *p)
{
	char *prog = "sortExpr";

	if (p == NULL)
		return NULL;

	p->left = sortExpr(p->left);
	p->right = sortExpr(p->right);

	p = refreshExprNodeName(p);
	fprintf(stderr, "%s: p->name: \"%s\" (BEFORE)\n", prog, p->name);
	p = removeExprParenth(p);
	fprintf(stderr, "%s: p->name: \"%s\" (BEFORE but parentheses removed)\n", prog, p->name);
	p = _sortExpr(p);
	p = refreshExprNode(p);		// to restore parentheses
	fprintf(stderr, "%s: p->name: \"%s\" (AFTER)\n", prog, p->name);

	return p;
}
void genSV(char w[], Node *node, char *delimiter);
Expr *_sortExpr(Expr *p)
{
	char *prog = "_sortExpr";

	if (p == NULL)
		return NULL;

	Op *op = getOp(op_tree, p->op);
	if (op == NULL)
		return p;
	if (op->comm == NULL)
		return p;

	/* the input p->name
	 * 		c * a * b		==>		O
	 * 		c * (a * b)		==>		X
	 * 		c + a + b		==>		O
	 * 		(c + a) + b		==>		O
	 * desired outcome
	 * 		c * a^2 * a * 7 * a	==>		7 * a * a * a^2 * c
	 * 		c + a + 7 + b		==>		a + b + c + 7 (future update)
	 * detect Op first and run custom binary tree sort */
	Node *node = NULL;
	char *line = p->name;
	char w[MAXCHAR] = "";
	while (strlen(line) > 0) {
		fprintf(stderr, "%s: parsing variables from \"%s\"\n", prog, line);
		line = parseSVmaskblk(w, line, op->name, block_start, block_end);
		while (is_outer_blocked_blk(w, block_start, block_end, NULL))
			remove_outer_block_blk(w, block_start, block_end);
		node = addNode(node, w);
	}
	//listNode(node);
	genSV(w, node, op->name);
	removeNode(node);
	node = NULL;
	removeExpr(&p);
	p = addExpr(p, w);

	return p;
}
void _genSV(char w[], Node *node, char *delimiter);
void genSV(char w[], Node *node, char *delimiter)
{
	char *prog = "genSV";

	w[0] = '\0';
	if (delimiter == NULL)
		delimiter = "";
	_genSV(w, node, delimiter);
	if (strlen(w) > strlen(delimiter)) {
		fprintf(stdout, "%s: bcutnstr: \"%s\"\t(before)\n", prog, w);
		bcutnstr(w, strlen(delimiter));
		fprintf(stdout, "%s: bcutnstr: \"%s\"\t(after)\n", prog, w);
	}
}
void _genSV(char w[], Node *node, char *delimiter)
{
	if (node == NULL)
		return ;
	_genSV(w, node->left, delimiter);
	while (node->count-- > 0) {
		strcat(w, node->name);
		strcat(w, delimiter);
	}
	_genSV(w, node->right, delimiter);
}
void testsortExpr(void)
{
	char *line = "x * -.14 / G * m * F";
	line = "1 * a * 3 * b";
	line = "1 + ((b * a) * 3)";
	line = "(((dx + x)^2) * (dx^-1)) + ((-1 * (x^2)) * (dx^-1))";
	line = "(c + (((dx * b) + (x * b)) + (((dx + x)^2) * a))) + ((-1 * (a * (x^2))) + ((-1 * (b * x)) + (-1 * c)))";
	//line = "a * -1 * b^1";
	Expr *p = NULL;
	op_tree = loadOps(op_tree);
	p = addExpr(p, line);

	listExpr(p);
	p = sortExpr(p);
	listExpr(p);
}



/* to be updated, combined with Op later */
Expr *_calcExprMult(Expr *p);
Expr *_calcExprDiv(Expr *p);
Expr *_calcExprAdd(Expr *p);
Expr *_calcExprSub(Expr *p);
Expr *_calcExprPow(Expr *p);
Expr *_calcExprMod(Expr *p);

Expr *evalExpr(Expr *p)
{
	char *prog = "calcExpr";

	if (p == NULL)
		return NULL;

	// postorder traverse
	fprintf(stderr, "%s: (start) p->name:%s\n", prog, p->name);

	fprintf(stderr, "%s: p->name:%s checking left\n", prog, p->name);
	p->left = evalExpr(p->left);
	fprintf(stderr, "%s: (pass) p->name:%s checking left\n", prog, p->name);
	fprintf(stderr, "%s: p->name:%s checking right\n", prog, p->name);
	p->right = evalExpr(p->right);
	fprintf(stderr, "%s: (pass) p->name:%s checking right\n", prog, p->name);

	p = refreshExprNode(p);

	p = _calcExprMult(p);
	p = _calcExprDiv(p);
	p = _calcExprAdd(p);
	p = _calcExprSub(p);
	p = _calcExprPow(p);
	p = _calcExprMod(p);

	char line[MAXCHAR] = "";
	strcpy(line, p->name);
	removeExpr(&p);
	p = addExpr(p, line);

	return p;
}
void testevalExpr(void)
{
	//char *line = "2 * (3^1 % 2) + 4 / 2^2 * x - 7^0";
	//char *line = "a * x^2 + b * x^1 + c * x^0";
	//char *line = "-1 * 3 * a * 4 * 7 * -1";
	//char *line = "-1 * 3 * a * 4 * 7 * -1 + -1 * 3 * b * 2 * 7 * -1";
	//char *line = "-1 * 3 * a * 2^2 * 7 * -1";
	//char *line = "-2 * (a + b) * -1";
	char *line = "-1 * 2 * 3^2 * 4";
	line = "x^(y - y)";
	line = "f = a * x^2 + b * x^1 + c * x^0, g = G * M * m / r^2";
	line = "((G * (M * m)) * (r^-2))";
	line = "((a * (x^2)) + ((b * x) + (c * 1)))";
	Expr *p = NULL;
	op_tree = loadOps(op_tree);

	p = addExpr(p, line);

	printf("original\n");
	listExpr(p);
	printf("---\n");

	//printf("after distributing\n");
	//p = distExpr(p);
	//listExpr(p);
	//printf("---\n");

	printf("testevalExpr\n");
	p = evalExpr(p);
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
		if (is_pure_number(p->left->name, NULL) == 1) {		// left number
			// unit check
			if (strcmp(p->left->name, "1") == 0) {
				/* unit */
				char line[MAXCHAR] = "";
				strcpy(line, p->right->name);
				removeExpr(&p);
				p = addExpr(p, line);
				return p;
			}
			/* 0 * x = 0 */
			if (strcmp(p->left->name, "0") == 0) {
				/* unit */
				removeExpr(&p);
				p = addExpr(p, "0");
				return p;
			}
			int c = is_pure_number(p->right->name, NULL);
			char *q = NULL;
			double value = strtod(p->right->name, &q);
			if (c == 1) {	// left number, right also number
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
		/* x * x^-1 */
		if (is_pure_number(p->left->name, NULL) == 0)
			if (strlen(p->left->op) == 0 &&
					strcmp(p->right->op, "^") == 0)
				if (strcmp(p->right->right->name, "-1") == 0 &&
						strcmp(p->left->name, p->right->left->name) == 0) {
					strcpy(p->name, "1");
					p->op[0] = '\0';
					return p;
				}
		/* x^-1 * x */
		if (is_pure_number(p->right->name, NULL) == 0)
			if (strlen(p->right->op) == 0 &&
					strcmp(p->left->op, "^") == 0)
				if (strcmp(p->left->right->name, "-1") == 0 &&
						strcmp(p->right->name, p->left->left->name) == 0) {
					strcpy(p->name, "1");
					p->op[0] = '\0';
					return p;
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
		// unit check
		if (strcmp(p->left->name, "0") == 0) {
			/* unit */
			char line[MAXCHAR] = "";
			strcpy(line, p->right->name);
			removeExpr(&p);
			p = addExpr(p, line);
			return p;
		}
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
		/* x + -1 * x */
		if (is_pure_number(p->left->name, NULL) == 0)
			if (strlen(p->left->op) == 0 &&
					strcmp(p->right->op, " * ") == 0)
				if (strcmp(p->right->left->name, "-1") == 0 &&
						strcmp(p->left->name, p->right->right->name) == 0) {
					strcpy(p->name, "0");
					p->op[0] = '\0';
					return p;
				}
		/* -1 * x + x */
		if (strcmp(p->left->op, " * ") == 0)
			if (strcmp(p->left->left->name, "-1") == 0 &&
					strcmp(p->right->name, p->left->right->name) == 0) {
				strcpy(p->name, "0");
				p->op[0] = '\0';
				return p;
			}
		/* c * x + -c * x */
		if (strcmp(p->left->op, " * ") == 0 &&
				strcmp(p->right->op, " * ") == 0) {
			char shortline[MAXCHAR] = "", longline[MAXCHAR] = "";
			if (strcmp(p->left->right->name, p->right->right->name) == 0) {
				if (strlen(p->left->left->name) < strlen(p->right->left->name)) {
					strcpy(shortline, p->left->left->name);
					strcpy(longline, p->right->left->name);
				} else {
					strcpy(shortline, p->right->left->name);
					strcpy(longline, p->left->left->name);
				}
				if (*longline == '-') {
					fcutstr(longline);
					if (strcmp(shortline, longline) == 0) {
						strcpy(p->name, "0");
						p->op[0] = '\0';
						return p;
					}
				}
			}
		}
		/* -c * x + c * x */
		/* c * x + -1 * c * x */
		/* -1 * c * x + c * x */
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
	char *prog = "_calcExprPow";

	char *op_name = "^";

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		/* x^1 = x */
		if (strcmp(p->right->name, "1") == 0) {
			char line[MAXCHAR] = "";
			strcpy(line, p->left->name);
			removeExpr(&p);
			p = addExpr(p, line);
			return p;
		}
		/* x^0 = 1 */
		if (strcmp(p->right->name, "0") == 0) {
			removeExpr(&p);
			p = addExpr(p, "1");
			return p;
		}
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

Expr *_displayExpr(Expr *p)
{
	char *prog = "_displayExpr";

	if (p == NULL)
		return NULL;

	Op *op = getOp(op_tree, p->op);
	if (op == NULL)
		return p;

	/* a - (b + c) vs a - b + c		use rank diff. actually, this will be gone by altExpr
	 * a - (b - c) vs a - b - c		use left-assoc and not right. actually, this will be gone by altExpr
	 * x/(y/z) vs x/y/z				use left-assoc and not right. actually, this will be gone by altExpr
	 * (x^y)^z vs x^y^z				use right-assoc and not left
	 * x^-1 * b vs x^(-1 * b)		use rank diff
	 * (a - b) * c vs a - b * c		use left_dist_over
	 * */
	Op *op_left = getOp(op_tree, p->left->op);
	Op *op_right = getOp(op_tree, p->right->op);
	char line[MAXCHAR] = "";
	strcpy(line, p->left->name);
	if (op_left != NULL) {
		if (strcmp(p->op, p->left->op) == 0) {
			if (op->right_assoc != NULL && op->left_assoc == NULL)
				parenthstr(line);	// right-assoc but not left, so
									// got to be empahsized
		} else if (op_left->order < op->order) {	// rank diff
			parenthstr(line);
		} else {
			for (int i = 0; op->left_dist_over[i] != NULL; i++)	// left_dist_over
				if (strcmp(op->left_dist_over[i], op_left->name) == 0)
					parenthstr(line);
		}
	}
	strcpy(p->name, line);
	strcat(p->name, p->op);
	strcpy(line, p->right->name);
	if (op_right != NULL) {
		if (strcmp(p->op, p->right->op) == 0) {
			if (op->left_assoc != NULL && op->right_assoc == NULL)
				parenthstr(line);	// left-assoc but not right, so
									// got to be empahsized
		} else if (op_right->order < op->order) {	// rank diff
			parenthstr(line);
		} else {
			for (int i = 0; op->right_dist_over[i] != NULL; i++)	// left_dist_over
				if (strcmp(op->right_dist_over[i], op_right->name) == 0)
					parenthstr(line);
		}
	}
	strcat(p->name, line);
	replacestr(p->name, " * ", " ");

	return p;
}
Expr *removeExprParenth(Expr *p)
{
	char *prog = "removeExprParenth";

	if (p == NULL)
		return NULL;

	Op *op = getOp(op_tree, p->op);
	if (op == NULL)
		return p;

	/* a - (b + c) vs a - b + c		use rank diff. actually, this will be gone by altExpr
	 * a - (b - c) vs a - b - c		use left-assoc and not right. actually, this will be gone by altExpr
	 * x/(y/z) vs x/y/z				use left-assoc and not right. actually, this will be gone by altExpr
	 * (x^y)^z vs x^y^z				use right-assoc and not left
	 * x^-1 * b vs x^(-1 * b)		use rank diff
	 * (a - b) * c vs a - b * c		use left_dist_over
	 * */
	Op *op_left = getOp(op_tree, p->left->op);
	Op *op_right = getOp(op_tree, p->right->op);
	char line[MAXCHAR] = "";
	strcpy(line, p->left->name);
	if (op_left != NULL) {
		if (strcmp(p->op, p->left->op) == 0) {
			if (op->right_assoc != NULL && op->left_assoc == NULL)
				parenthstr(line);	// right-assoc but not left, so
									// got to be empahsized
		} else if (op_left->order < op->order) {	// rank diff
			parenthstr(line);
		} else {
			for (int i = 0; op->left_dist_over[i] != NULL; i++)	// left_dist_over
				if (strcmp(op->left_dist_over[i], op_left->name) == 0)
					parenthstr(line);
		}
	}
	strcpy(p->name, line);
	strcat(p->name, p->op);
	strcpy(line, p->right->name);
	if (op_right != NULL) {
		if (strcmp(p->op, p->right->op) == 0) {
			if (op->left_assoc != NULL && op->right_assoc == NULL)
				parenthstr(line);	// left-assoc but not right, so
									// got to be empahsized
		} else if (op_right->order < op->order) {	// rank diff
			parenthstr(line);
		} else {
			for (int i = 0; op->right_dist_over[i] != NULL; i++)	// left_dist_over
				if (strcmp(op->right_dist_over[i], op_right->name) == 0)
					parenthstr(line);
		}
	}
	strcat(p->name, line);
	//replacestr(p->name, " * ", " ");

	return p;
}
void testremoveExprParenth(void)
{
	op_tree = loadOps(op_tree);
	Expr *p = NULL;
	char *line = "a * b + c * (d + e)";
	line = "(((dx + x)^2) * (dx^-1)) + ((-1 * (x^2)) * (dx^-1))";
	line = "a * (b * (c + d))";
	line = "((dx + x)^2) * (a * dx^-1) + (-1 * (b * x * dx^-1)  + -1 * (a * x^2 * dx^-1) + -1 * (c * dx^-1) + b + b * (dx^-1 * x) + c * (dx^-1)";
	line = "((dx + x)^2) * (a * dx^-1) + (-1 * (b * x * dx^-1)) + -1 * (a * x^2 * dx^-1) + -1 * (c * dx^-1) + b + b * (dx^-1 * x) + c * (dx^-1)";
	line = "(c + (((dx * b) + (x * b)) + (((dx + x)^2) * a))) + ((-1 * (a * (x^2))) + ((-1 * (b * x)) + (-1 * c)))";

	p = addExpr(p, line);
	printf("listExpr before removeExprParenth\n");
	listExpr(p);

	printf("listExpr after removeExprParenth\n");
	p = removeExprParenth(p);
	listExpr(p);
}
Expr *removeExprProd(Expr *p)
{
	char *prog = "removeExprProd";

	if (p == NULL)
		return NULL;

	Op *op = getOp(op_tree, p->op);
	if (op == NULL)
		return p;

	replacestr(p->name, " * ", " ");

	return p;
}
Expr *displayExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	p->left = displayExpr(p->left);
	p->right = displayExpr(p->right);

	p = refreshExprNodeName(p);

	char prev[MAXCHAR] = "";

	do {
		strcpy(prev, p->name);
		//p = _displayExpr(p);
		p = removeExprParenth(p);
		p = removeExprProd(p);
	} while (strcmp(prev, p->name) != 0);

	return p;
}
void testdisplayExpr(void)
{
	op_tree = loadOps(op_tree);
	Expr *p = NULL;

	char *line = "1 + x - 1";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "(a - b) * (c * d)";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "x^y^z";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "(x^y)^z";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "x/y/z";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "x^(a * b)";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "x^a * b";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "x^a^b";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");
	
	line = "(x^a)^b";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "a - (b % c)";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "a - b % c";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");

	line = "(a - b) % c";
	printf("original\n");
	p = addExpr(p, line);
	printf("%s\n", line);
	printf("testdisplayExpr\n");
	p = displayExpr(p);
	printf("%s\n", p->name);
	removeExpr(&p);
	printf("---\n");
}

Expr *refreshExprName(Expr *p)
{
	if (p == NULL)
		return NULL;
	
	p->left = refreshExprName(p->left);
	p->right = refreshExprName(p->right);

	p = refreshExprNodeName(p);

	return p;
}


#endif	/* _EXPRESSION_H */
