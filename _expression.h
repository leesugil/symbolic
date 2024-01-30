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

void parseExprOpBy(char [], char [], char **, char **, char **);
void parseExprLeftBy(char [], char *, char **, char **, char **);
void parseExprRightBy(char [], char *, char **, char **, char **);

Expr *addExpr(Expr *p, char *name)
{
	char *prog = "addExpr";

	if (p == NULL) {
		p = exprAlloc();
		strcpy(p->name, name);
		unsigned int index = 0;
		while (is_outer_blocked_blk(p->name, block_start, block_end, &index))
			remove_outer_block(p->name, block_start[index], block_end[index]);
		p->left = NULL;
		p->right = NULL;

		fprintf(stderr, "%s: \"%s\" added to the tree\n", prog, p->name);

		/* see if it's about definition,
		 * first by commas ',' */
		/* if not, see if it has LHS and RHS */
		/* if not, start parsing operations */
		if (strcmp(p->name, "") != 0) {
			char dum_line[MAXCHAR] = "";
			/* check dividers */
			fprintf(stderr, "(1) p->name:\"%s\"\n", p->name);
			parseExprLeftBy(dum_line, p->name, dividers, NULL, NULL);
			fprintf(stderr, "(2)\n");
			if (dum_line[0] != '\0') {
				/* f = (x + y), g = (y + z) */
				fprintf(stderr, "(2.1) \", \"-like divider found in \"%s\"\n", p->name);
				fprintf(stderr, "(3) p->left->name:\"%s\"\n", dum_line);
				p->left = addExpr(p->left, dum_line);
				fprintf(stderr, "(4) back to p->name:\"%s\"\n", p->name);
				/* parseExprOpBy will apply mask... need a fix */
				/* fixed (potentially) */
				parseExprOpBy(p->op, p->name, dividers, NULL, NULL);
				fprintf(stderr, "(5) p->op:\"%s\"\n", p->op);
				parseExprRightBy(dum_line, p->name, dividers, NULL, NULL);
				if (dum_line[0] != '\0') {
					fprintf(stderr, "(6) p->right->name:\"%s\"\n", dum_line);
					p->right = addExpr(p->right, dum_line);
					fprintf(stderr, "(7) back to p->name:\"%s\"\n", p->name);
				}
			} else {
				/* no dividers */
				/* check definitions */
				fprintf(stderr, "(8) no \", \"-like dividers found in \"%s\"\n", p->name);
				parseExprLeftBy(dum_line, p->name, definitions, NULL, NULL);
				fprintf(stderr, "(9)\n");
				if (dum_line[0] != '\0') {
					/* f = (x + y) */
					fprintf(stderr, "(10) \" = \"-like divider found in \"%s\"\n", p->name);
					fprintf(stderr, "(10) p->left->name:\"%s\"\n", dum_line);
					p->left = addExpr(p->left, dum_line);
					fprintf(stderr, "(11) back to p->name:\"%s\"\n", p->name);
					parseExprOpBy(p->op, p->name, definitions, NULL, NULL);
					fprintf(stderr, "(12) p->op:\"%s\"\n", p->op);
					parseExprRightBy(dum_line, p->name, definitions, NULL, NULL);
					if (dum_line[0] != '\0') {
						fprintf(stderr, "(13) p->right->name:\"%s\"\n", dum_line);
						p->right = addExpr(p->right, dum_line);
						fprintf(stderr, "(14) back to p->name:\"%s\"\n", p->name);
					}
				} else {
					/* no definition statements */
					/* check LHS, RHS */
					fprintf(stderr, "(15) no \" = \"-like divider found in \"%s\"\n", p->name);
					parseExprLeftBy(dum_line, p->name, comparisons, NULL, NULL);
					fprintf(stderr, "(16)\n");
					if (dum_line[0] != '\0') {
						/* f + x == g * z */
						fprintf(stderr, "(17) \" == \"-like divider found in \"%s\"\n", p->name);
						fprintf(stderr, "(17) p->left->name:\"%s\"\n", dum_line);
						p->left = addExpr(p->left, dum_line);
						fprintf(stderr, "(18) back to p->name:\"%s\"\n", p->name);
						parseExprOpBy(p->op, p->name, comparisons, NULL, NULL);
						fprintf(stderr, "(19) p->op:\"%s\"\n", p->op);
						parseExprRightBy(dum_line, p->name, comparisons, NULL, NULL);
						if (dum_line[0] != '\0') {
							fprintf(stderr, "(20) p->right->name:\"%s\"\n", dum_line);
							p->right = addExpr(p->right, dum_line);
							fprintf(stderr, "(21) back to p->name:\"%s\"\n", p->name);
						}
					} else {
						/* no comparison statements */
						/* regular math expressions */
						fprintf(stderr, "(22) no \" == \"-like divider found in \"%s\"\n", p->name);
						parseExprLeftBy(dum_line, p->name, operators, block_start, block_end);
						if (dum_line[0] != '\0') {
							fprintf(stderr, "(23) p->left->name:\"%s\"\n", dum_line);
							p->left = addExpr(p->left, dum_line);
							fprintf(stderr, "(24) back to p->name:\"%s\"\n", p->name);
						}
						parseExprOpBy(p->op, p->name, operators, block_start, block_end);
						fprintf(stderr, "(25) p->op:\"%s\"\n", p->op);
						parseExprRightBy(dum_line, p->name, operators, block_start, block_end);
						if (dum_line[0] != '\0') {
							fprintf(stderr, "(26) p->right->name:\"%s\"\n", dum_line);
							p->right = addExpr(p->right, dum_line);
							fprintf(stderr, "(27) back to p->name:\"%s\"\n", p->name);
						}
					}
				}
			}
		} else {
			/* p->name == "" */
			fprintf(stderr, "%s: p->name empty\n", prog);
		}
	}

	return p;
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
	unsigned int index = 0;

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
void parseExprLeftBy(char w[], char *line, char **bulk, char **block_start, char **block_end)
{
	/* do strstrblk with ==, <, >, etc., get n, and do bcutnstr */
	char *prog = "parseExprLeft";
	unsigned int index = 0;
	char *p = NULL;
	
	/* index updated is for bulk */
	p = strstrblkmaskblk(line, bulk, &index, block_start, block_end);

	if (p == NULL) {
		fprintf(stderr, "%s: there is no LHS to parse\n", prog);
		w[0] = '\0';
	}
	else {
		int n = strlen(line) - (p - line);
		strcpy(w, line);
		bcutnstr(w, n);
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

void parseExprRightBy(char w[], char *line, char **bulk, char **block_start, char **block_end)
{
	/* do strstrblk with " = ", get n, do fcutnstr */
	char *prog = "parseExprRight";
	unsigned int index = 0;
	char *p = NULL;

	/* index updated is for bulk */
	p = strstrblkmaskblk(line, bulk, &index, block_start, block_end);

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
	printf("%s\n", p->op);
}

/* listExpr: in-order print of tree p */
void listExpr(Expr *p)
{
	static int tabs = 0;

	if (p != NULL) {
		if (tabs > 0) {
			printf("\n");
			printn("\t", tabs-1);
			printf("\t\b\b\b\b|   ");
			printf("\n");
			printn("\t", tabs-1);
			printf("\t\b\b\b\b+-  ");
		}
		printf("%s\n", p->name);
		tabs++;
		listExpr(p->left);
		//if (strlen(p->op) > 0) {
		//	printn("\t", tabs);
		//	displayExprOp(p);
		//}
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
		//free(p->name);	no longer dynamically allocated
		//free(p->op);		no longer dynamically allocated
		strcpy(p->name, "deleted");
		free(p);
	}
}

/* removeExpr: frees a expr and its branch below */
void removeExpr(Expr **p)
{
	_removeExpr(*p);
	*p = NULL;
}

#endif	/* _EXPRESSION_H */
