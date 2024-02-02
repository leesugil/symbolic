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

void parseExprOpBy(char op[], char line[], char **op_list, char **pre, char **suf);
void parseExprLeftBy(char lhs[], char *line, char **div_list, char **pre, char **suf);
void parseExprRightBy(char rhs[], char *line, char **div_list, char **pre, char **suf);
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
/* input " = " causes error, keep capturing p->left == " = " causing infinite loop */
Expr *parseExprs(Expr *p, int *flag);		/* check if it has multiple statements */
Expr *parseExprDef(Expr *p, int *flag);		/* check if it is a definition statement */
Expr *parseExprSt(Expr *p, int *flag);	/* check if it is a mathematical statement with LHS and RHS */
Expr *parseExprReg(Expr *p, char **op_list);					/* regular (x + y) / z */
void parseNegSign(char s[]);
void refreshExprTree(Expr **p);

Expr *parseExpr(Expr *p)
{
	char *prog = "parseExpr";

	if (p == NULL)
		return NULL;

	parseNegSign(p->name);
	remove_outer_block_blk(p->name, block_start, block_end);

	fprintf(stderr, "%s: parsing \"%s\"\n", prog, p->name);

	fprintf(stderr, "%s: checking if blocked properly\n", prog);
	int index = 0;
	if (!is_blocked_properly_blk(p->name, block_start, block_end, &index)) {
		fprintf(stderr, "%s: block error(s) found in \"%s\"\n", prog, p->name);
		fprintf(stderr, "%s: returning expr unchanged\n", prog);
		return p;
	}
	fprintf(stderr, "%s: (pass) checking if blocked properly\n", prog);

	fprintf(stderr, "%s: checking if outer blocks are removed\n", prog);
	index = 0;
	// make sure there're no redundant outer blocks
	while (is_outer_blocked_blk(p->name, block_start, block_end, &index)) {
		fprintf(stderr, "%s: outer block exists in \"%s\", removing...\n", prog, p->name);
		remove_outer_block(p->name, block_start[index], block_end[index]);
		fprintf(stderr, "%s: (pass) outer block exists in \"%s\", removing...\n", prog, p->name);
	}
	fprintf(stderr, "%s: (pass) checking if outer blocks are removed\n", prog);

	/* see if it's about definition,
	 * first by commas ',' */
	/* if not, see if it has LHS and RHS */
	/* if not, start parsing operations */
	if (strcmp(p->name, "") != 0) {
		int flag = 0;
		/* check dividers ", " */
		fprintf(stderr, "%s: (1) p->name:\"%s\"\n", prog, p->name);
		p = parseExprs(p, &flag);
		if (flag == 1) {
			/* no dividers */
			/* check definitions " = " */
			/* "x = " or " = " */
			fprintf(stderr, "%s: (8) no \", \"-like dividers found in \"%s\"\n", prog, p->name);
			p = parseExprDef(p, &flag);
		}
		if (flag == 1) {
			/* no definition statements */
			/* check LHS, RHS, " == " */
			fprintf(stderr, "%s: (15) no \" = \"-like divider found in \"%s\"\n", prog, p->name);
			p = parseExprSt(p, &flag);
		}
		if (flag == 1) {
			/* no comparison statements */
			/* regular math expressions */
			fprintf(stderr, "%s: (22) no \" == \"-like divider found in \"%s\"\n", prog, p->name);
			p = parseExprReg(p, operators1);
			p = parseExprReg(p, operators2);
			p = parseExprReg(p, operators3);
			p = parseExprReg(p, operators4);
			p = parseExprReg(p, operators5);
		}
	} else {
		/* p->name == "" */
		fprintf(stderr, "%s: p->name empty, nothing to parse\n", prog);
	}

	return p;
}
/* -x ==> -1 * x */
void parseNegSign(char s[])
{
	convertNegSign(s);
}
/* check if it has multiple statements */
Expr *parseExprs(Expr *p, int *flag)
{
	char *prog = "parseExprs";

	if (p == NULL)
		return p;

	char dum_line[MAXCHAR] = "";

	parseExprLeftBy(dum_line, p->name, dividers, block_start, block_end);
	/* use block_start and block_end to make a distinction when
	 * vector notations are adopted as v = (x, y, z)
	 * in future projects */
	fprintf(stderr, "%s: (2)\n", prog);
	if (dum_line[0] != '\0') {
		*flag = 0;
		/* f = (x + y), g = (y + z) */
		fprintf(stderr, "(2.1) \", \"-like divider found in \"%s\"\n", p->name);
		fprintf(stderr, "(3) candidate for p->left->name:\"%s\"\n", dum_line);
		//parseNegSign(dum_line);
		p->left = addExpr(p->left, dum_line);
		fprintf(stderr, "(4) back to p->name:\"%s\"\n", p->name);
		/* parseExprOpBy will apply mask... need a fix */
		/* fixed (potentially) */
		//parseExprOpBy(p->op, p->name, dividers, NULL, NULL);
		/* wait, why was the masking an issue? */
		parseExprOpBy(p->op, p->name, dividers, block_start, block_end);
		fprintf(stderr, "(5) p->op:\"%s\"\n", p->op);
		//parseExprRightBy(dum_line, p->name, dividers, NULL, NULL);
		// why was the masking an issue?
		parseExprRightBy(dum_line, p->name, dividers, block_start, block_end);
		if (dum_line[0] != '\0') {
			fprintf(stderr, "(6) p->right->name:\"%s\"\n", dum_line);
			//parseNegSign(dum_line);
			p->right = addExpr(p->right, dum_line);
			fprintf(stderr, "(7) back to p->name:\"%s\"\n", p->name);
		} else {
			fprintf(stderr, "%s: p->right = NULL\n", prog);
			if (p->right != NULL) {
				fprintf(stderr, "%s: wait, p->right != NULL. check this out in detail.\n", prog);
				fprintf(stderr, "%s: p->right->name=\"%s\"\n", prog, p->right->name);
			}
		}
	} else {
		fprintf(stderr, "%s: nothing to parse, flag == 1\n", prog);
		*flag = 1;
	}

	return p;
}
/* check if it is a definition statement */
Expr *parseExprDef(Expr *p, int *flag)
{
	char *prog = "parseExprDef";

	char dum_line[MAXCHAR] = "";

	parseExprLeftBy(dum_line, p->name, definitions, block_start, block_end);
	fprintf(stderr, "(9)\n");
	if (dum_line[0] != '\0') {
		*flag = 0;
		/* f = (x + y) */
		fprintf(stderr, "%s: (10) \" = \"-like divider found in \"%s\"\n", prog, p->name);
		fprintf(stderr, "%s: (10) candidate for p->left->name:\"%s\"\n", prog, dum_line);
		//parseNegSign(dum_line);
		p->left = addExpr(p->left, dum_line);
		fprintf(stderr, "%s: (11) back to p->name:\"%s\"\n", prog, p->name);
		parseExprOpBy(p->op, p->name, definitions, block_start, block_end);
		fprintf(stderr, "%s: (12) p->op:\"%s\"\n", prog, p->op);
		parseExprRightBy(dum_line, p->name, definitions, block_start, block_end);
		/* for "x = " or " = ", dum_line == "" */
		fprintf(stderr, "%s: (12.1) p->right candidate:\"%s\"\n", prog, dum_line);
		if (dum_line[0] != '\0') {
			fprintf(stderr, "%s: (13) p->right->name:\"%s\"\n", prog, dum_line);
			//parseNegSign(dum_line);
			p->right = addExpr(p->right, dum_line);
			fprintf(stderr, "%s: (14) back to p->name:\"%s\"\n", prog, p->name);
		} else {
			fprintf(stderr, "%s: p->right = NULL\n", prog);
			if (p->right != NULL) {
				fprintf(stderr, "%s: wait, p->right != NULL. check this out in detail.\n", prog);
				fprintf(stderr, "%s: p->right->name=\"%s\"\n", prog, p->right->name);
			}
		}
	} else {
		fprintf(stderr, "%s: nothing to parse, flag == 1\n", prog);
		*flag = 1;
	}

	return p;
}
/* check if it is a mathematical statement with LHS and RHS */
Expr *parseExprSt(Expr *p, int *flag)
{
	char *prog = "parseExprSt";

	char dum_line[MAXCHAR] = "";

	parseExprLeftBy(dum_line, p->name, comparisons, block_start, block_end);
	fprintf(stderr, "(16)\n");
	if (dum_line[0] != '\0') {
		*flag = 0;
		/* f + x == g * z */
		fprintf(stderr, "(17) \" == \"-like divider found in \"%s\"\n", p->name);
		fprintf(stderr, "(17) cadidate for p->left->name:\"%s\"\n", dum_line);
		//parseNegSign(dum_line);
		p->left = addExpr(p->left, dum_line);
		fprintf(stderr, "(18) back to p->name:\"%s\"\n", p->name);
		parseExprOpBy(p->op, p->name, comparisons, block_start, block_end);
		fprintf(stderr, "(19) p->op:\"%s\"\n", p->op);
		parseExprRightBy(dum_line, p->name, comparisons, block_start, block_end);
		if (dum_line[0] != '\0') {
			fprintf(stderr, "(20) p->right->name:\"%s\"\n", dum_line);
			//parseNegSign(dum_line);
			p->right = addExpr(p->right, dum_line);
			fprintf(stderr, "(21) back to p->name:\"%s\"\n", p->name);
		} else {
			fprintf(stderr, "%s: p->right = NULL\n", prog);
			if (p->right != NULL) {
				fprintf(stderr, "%s: wait, p->right != NULL. check this out in detail.\n", prog);
				fprintf(stderr, "%s: p->right->name=\"%s\"\n", prog, p->right->name);
			}
		}
	} else {
		fprintf(stderr, "%s: nothing to parse, flag == 1\n", prog);
		*flag = 1;
	}

	return p;
}
/* parseExprReg: parse regular mathematical expressions */
// there's gotta be more depth to this algorithm.
// consider the example:
// a * x + b * y
// typing this much is already a huge compromise to users, so don't expect that they'll put
// (a * x) + (b * y)
// this is on of the top urgent future updates.
// a * x + b * y
//   ^
//   |
//   +-- if detected,
//       look left and right to see if
//       ...
// or basically, { +, - } should've been searched first with higher priority, (weaker bond)
// then { *, /, ^ }.
// { % } is a special case, i'll have to deal with it later.
// like a * (b % c) * d + e kind of crazyness.
//  ==> +, *, % order...
Expr *parseExprReg(Expr *p, char **op_list)
{
	char *prog = "parseExprOp";

	char dum_line[MAXCHAR] = "";
	strcpy(dum_line, p->name);

	if (strcmp(p->op, "") != 0) {
		//parseNegSign(dum_line);
		return p;
	}

	parseExprLeftBy(dum_line, p->name, op_list, block_start, block_end);
	if (dum_line[0] != '\0') {
		fprintf(stderr, "(23) p->left->name:\"%s\"\n", dum_line);
		//parseNegSign(dum_line);
		p->left = addExpr(p->left, dum_line);
		fprintf(stderr, "(24) back to p->name:\"%s\"\n", p->name);
	} else {
		fprintf(stderr, "%s: p->left = NULL\n", prog);
		if (p->left != NULL) {
			fprintf(stderr, "%s: wait, p->left != NULL ??\n", prog);
			fprintf(stderr, "%s: p->left->name=\"%s\"\n", prog, p->left->name);
		}
	}
	parseExprOpBy(p->op, p->name, op_list, block_start, block_end);
	fprintf(stderr, "(25) p->op:\"%s\"\n", p->op);
	parseExprRightBy(dum_line, p->name, op_list, block_start, block_end);
	if (dum_line[0] != '\0') {
		fprintf(stderr, "(26) p->right->name:\"%s\"\n", dum_line);
		//parseNegSign(dum_line);
		p->right = addExpr(p->right, dum_line);
		fprintf(stderr, "(27) back to p->name:\"%s\"\n", p->name);
	} else {
		fprintf(stderr, "%s: p->right = NULL\n", prog);
		if (p->left != NULL) {
			fprintf(stderr, "%s: wait, p->right != NULL ??\n", prog);
			fprintf(stderr, "%s: p->right->name=\"%s\"\n", prog, p->right->name);
		}
	}

	return p;
}
void listExpr(Expr *p);
Expr *evalExpr(Expr *p);
Expr *_distExpr(Expr *p, char *prod, char *sum);
void testparseExpr(void)
{
	//char *line = "a * x^2 + b * x^1 + c * x^0";
	//char *line = "x * (a * (b - c) / d) / y";
	//char *line = "a / (b - c)";
	//char *line = "a * ((b / d) - (c / d))";
	//char *line = "(b / d) - (c / d)";
	char *line = "x * a * (b - c) / d / y * z + x";
	Expr *expr = NULL;

	expr = addExpr(expr, line);

	printf("original expr:\n");
	listExpr(expr);
	printf("---\n");
}

/* parseExprOpBy: from a formula, parse the primary operator part */
//void parseExprOpBy(char op[], char line[], char **op_list)
void parseExprOpBy(char op[], char line[], char **op_list, char **b_start, char **b_end)
{
	/* (x + y) * z	-> strstrmaskblk will do.
	 * x + (y + z) * z -> strstrmaskblk will do.
	 * (x + y) * (x + z)	-> strstrmaskblk will do.
	 * (x + y * z)	-> strstrmaskblk will handle it!.
	 * */
	/* strstrmaskblk now works properly! */
	char *prog = "parseExprOp";
	char *p, *q = NULL;
	int i;
	int index = 0;

	/* index updated to the one in op_list */
	q = strstrblkmaskblk(line, op_list, &index, b_start, b_end);

	/* q is the best candidate containing op_list[index] in front */
	if (q != NULL) {
		fprintf(stderr, "%s: retrieving the first %lu characters from the best candidate...\n", prog, strlen(op_list[index]));
		strcpy(op, q);
		i = strlen(q) - strlen(op_list[index]);
		bcutnstr(op, i);
	} else {
		fprintf(stderr, "%s: there's no op to retrieve\n", prog);
		op[0] = '\0';
	}
}
void testparseExprOpBy(void)
{
	char line1[MAXCHAR] = "(((x + y) * y)^(y + z))";
	char line2[MAXCHAR] = "(((x + y) * y) + (y + z))";
	char line3[MAXCHAR] = "(((x + y) * y) % (y + z))";
	char line4[MAXCHAR] = "((((x + y) * y) % (y + z))";
	char line5[MAXCHAR] = "f = (((x + y) * y), g = (y + z))";
	char op[MAXCHAR] = "";

	printf("input: \"%s\"\n", line1);
	parseExprOpBy(op, line1, operators, block_start, block_end);
	printf("testparseExprOpBy: line1, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line2);
	op[0] = '\0';
	parseExprOpBy(op, line2, operators, block_start, block_end);
	printf("testparseExprOpBy: line2, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line3);
	op[0] = '\0';
	parseExprOpBy(op, line3, operators, block_start, block_end);
	printf("testparseExprOpBy: line3, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line4);
	op[0] = '\0';
	parseExprOpBy(op, line4, operators, block_start, block_end);
	printf("testparseExprOpBy: line4, \"%s\"\n\n", op);

	printf("input: \"%s\"\n", line5);
	op[0] = '\0';
	parseExprOpBy(op, line5, dividers, block_start, block_end);
	printf("testparseExprOpBy: line5, \"%s\"\n\n", op);
}

/* parseExprLeftBy: whether the condition given by the " = "-type or the " == "-type, the function parses the relevant LHS part */
void parseExprLeftBy(char w[], char *line, char **bulk, char **b_start, char **b_end)
{
	/* do strstrblk with ==, <, >, etc., get n, and do bcutnstr */
	char *prog = "parseExprLeft";
	int index = 0;
	char *p = NULL;

	fprintf(stderr, "%s: parse subject:\"%s\"\n", prog, line);
	
	/* index updated is for bulk */
	p = strstrblkmaskblk(line, bulk, &index, b_start, b_end);

	if (p == NULL) {
		fprintf(stderr, "%s: there is no LHS to parse\n", prog);
		w[0] = '\0';
	}
	else {
		int n = strlen(p);
		fprintf(stderr, "%s: p:\"%s\", n:%d\n", prog, p, n);
		strcpy(w, line);
		bcutnstr(w, n);
		fprintf(stderr, "%s: parsed:\"%s\"\n", prog, w);
	}
}
void testparseExprLeftBy(void)
{
	char *line1 = "this is a test LHS = this is a test RHS";
	char w[MAXCHAR] = "";

	printf("input: \"%s\"\n", line1);
	parseExprLeftBy(w, line1, comparisons, NULL, NULL);
	printf("testparseexprleft: \"%s\"\n", w);

	char *line2 = "this is a test LHS >= this is a test RHS";
	w[0] = '\0';

	printf("input: \"%s\"\n", line2);
	parseExprLeftBy(w, line2, comparisons, NULL, NULL);
	printf("testparseexprleft: \"%s\"\n", w);
}

void parseExprRightBy(char w[], char *line, char **bulk, char **b_start, char **b_end)
{
	/* do strstrblk with " = ", get n, do fcutnstr */
	char *prog = "parseExprRight";
	int index = 0;
	char *p = NULL;

	fprintf(stderr, "%s: parse subject:\"%s\"\n", prog, line);

	/* index updated is for bulk */
	p = strstrblkmaskblk(line, bulk, &index, b_start, b_end);
	/* when line = " = ", this gave p == NULL */

	if (p == NULL) {
		fprintf(stderr, "%s: there is no RHS to parse\n", prog);
		w[0] = '\0';
	}
	else {
		fprintf(stderr, "%s: RHS before removing the operator part:\"%s\"\n", prog, p);
		int n = strlen(bulk[index]);
		fprintf(stderr, "%s: deleting the first %d characters\n", prog, n);
		strcpy(w, p);
		fcutnstr(w, n);
		fprintf(stderr, "%s: output:\"%s\"\n", prog, w);
	}
}
void testparseExprRightBy(void)
{
	char *line1 = "this is a test LHS = this is a test RHS";
	char w[MAXCHAR] = "";

	printf("input: \"%s\"\n", line1);
	parseExprRightBy(w, line1, comparisons, NULL, NULL);
	printf("testparseexprright: \"%s\"\n", w);

	char *line2 = "this is a test LHS >= this is a test RHS";
	w[0] = '\0';

	printf("input: \"%s\"\n", line2);
	parseExprRightBy(w, line2, comparisons, NULL, NULL);
	printf("testparseexprright: \"%s\"\n", w);
}

void displayExprOp(Expr *p)
{
	printf("op:\"%s\"\n", p->op);
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
			displayExprOp(p);
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
		//free(p->name);	//no longer dynamically allocated
		//free(p->op);		//no longer dynamically allocated
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

/* evalExpr: evaluates an expr */
Expr *evalExpr(Expr *p)
{
	if (p == NULL)
		return NULL;
	
	p->left = evalExpr(p->left);
	p->right = evalExpr(p->right);

	if (p->left != NULL)
		strcpy(p->name, p->left->name);
	strcat(p->name, p->op);
	if (p->right != NULL)
		strcat(p->name, p->right->name);
	if ((strcmp(p->op, "")
			* strcmp(p->op, ", ")
			* strcmp(p->op, " = ")
			* strcmp(p->op, " == ")
			* strcmp(p->op, " >= ")
			* strcmp(p->op, " > ")
			* strcmp(p->op, " <= ")
			* strcmp(p->op, " < ")) != 0)
		parenthstr(p->name);

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

/* reduceExpr */
/* this is the real challenge. given an arithmetic expression - yes, start with the arithmetic case first - evaluate as much as possible based on known values. */
/* this could've been implemeted as a stand-alone feature, and this is what essentially doing the algebra */
/* an idea:
 * 1) make lists of operations by their properties, like distributive, commutative, associative, etc.
 * 2) turn an expr into the most possibly distributed order
 * 3) don't mind the associativity for now, we're not seeking the fastest way to solve it for now
 * 4) using commutativity whenever applicable, each term in the most distributed form should be symbolically ordered?
 * 5) how would i detect (number) * (number) * (letter)?
 * ... */
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
	int index = 0;
	char *sum2[] = {
		sum,
		NULL
	};

	/* just update p->name based on p->left and p->right */
	p = refreshExpr(p);

	if (strcmp(p->op, prod) == 0) {
		if (p->left != NULL && p->right != NULL) {
			/* left-to-right distribution */
			if (strcmp(p->right->op, sum) == 0) {
				/* there is an operation in RHS to distribute the primary operation over */
				/* (p->left->name p->op parseExprLeftBy) parseExprOpBy (p->left->name p->op parseExprRightBy) */
				// "left" will be the p->right->left->name part
				parseExprLeftBy(left, p->right->name, sum2, block_start, block_end);
				// "right" will be the p->right->right->name part
				parseExprRightBy(right, p->right->name, sum2, block_start, block_end);
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
	int index = 0;
	char *sum2[] = {
		sum,
		NULL
	};
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
				parseExprLeftBy(left, p->left->name, sum2, block_start, block_end);
				// "right" will be the p->left->right->name part
				parseExprRightBy(right, p->left->name, sum2, block_start, block_end);
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
	char *line = "-2 * (a + b) * -1";
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

	fprintf(stderr, "%s: p->op:%s\n", prog, p->op);
	if (strcmp(p->op, op_name) == 0) {
		char *leftp = NULL;
		double left = strtod(p->left->name, &leftp);	// p != NULL => p->left != NULL
		fprintf(stderr, "%s: leftp:%s\n", prog, leftp);
		if (strlen(leftp) == 0) {
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
Expr *commExpr(Expr *p)
{
	char *prog = "commExpr";

	if (p == NULL)
		return NULL;

	fprintf(stderr, "%s: considering \"%s\"\n", prog, p->name);

	// postorder traversal
	if (p->left != NULL)
		p->left = commExpr(p->left);
	if (p->right != NULL)
		p->right = commExpr(p->right);

	fprintf(stderr, "%s: back on \"%s\"\n", prog, p->name);

	// refresh node to update name
	p = refreshExpr(p);

	// postorder traversal work
	if (strcmp(p->op, " * ") == 0) {
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
void testcommExpr(void)
{
	//char *line = "((x + y) * z) * ((a + b) * c)";
	//char *line = "3 * a * 4 * b + c * 5 * d * 7";
	//char *line = "3 * a * 4 * b / c * 5 * d * 7";
	char *line = "-1 * a * b * -1";
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
