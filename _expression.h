#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "operation.h"

struct expression {
	char *_content;				/* original text used to create obj */
	char *content;				/* to be updated after reduction */
	char *op;					/* string temporarily */
	struct expression *left;	/* NULL if atom */
	struct expression *right;	/* NULL if atom */
};

typedef struct expression EXPR;

static EXPR *exprAlloc(void)
{
	return (EXPR *) malloc(sizeof(EXPR));
}

static char *OP_DIV[] = {
	" + ",
	" - ",
	" * ",
	" / ",
	" % ",
	" ^ "
};

static unsigned int OP_DIV_N = sizeof(OP_DIV) / sizeof(OP_DIV[0]);

static char *DEFN_DIV[] = {
	", ",
	" = "
};

static unsigned int DEFN_DIV_N = sizeof(DEFN_DIV) / sizeof(DEFN_DIV[0]);

static char *EQN_DIV[] = {
	" == ",
	" > ",
	" >= ",
	" < ",
	" <= "
};

static unsigned int EQN_DIV_N = sizeof(EQN_DIV) / sizeof(EQN_DIV[0]);

/* parseExpr: divides mathematical expressions or statements and returns the first piece */
char *parseExpr(char **line)
{
	fprintf(stderr, "parseExpr: parsing \"%s\"\n", *line);
	int i, n;
	char *s = NULL, *t = NULL;

	/* remove empty spaces?
	while (isspace(**line))
		(*line)++; */
	
	/* detect definition by " = ", ", " */
	/* x = 1.2, y = -3.4e-5 */
	/* f(x, y) = x^2 + y^2 */
	for (i = 0; i < DEFN_DIV_N; i++) {
		fprintf(stderr, "parseExpr: DEFN_DIV[%d] = \"%s\"\n", i, DEFN_DIV[i]);
		if((s = strstr(*line, DEFN_DIV[i])) != NULL) {
			/* DEFN_DIV[i] detected */
			fprintf(stderr, "parseExpr: DEFN_DIV[%d] = \"%s\" detected, s = \"%s\", *line = \"%s\"\n", i, DEFN_DIV[i], s, *line);
			/* get the first part */
			if ((n = s - *line) == 0) {
				fprintf(stderr, "parseExpr: s == *line, parsing \"%s\"\n", DEFN_DIV[i]);
				/* means the divider should be taken out as the output */
				*line = s + strlen(DEFN_DIV[i]);
				t = strndup(s, strlen(DEFN_DIV[i]));
				fprintf(stderr, "parseExpr: \"%s\" and \"%s\" parsed\n", t, *line);
				return t;
			} else {
				fprintf(stderr, "parseExpr: s != *line, parsing up to (but not including) \"%s\"\n", DEFN_DIV[i]);
				t = *line;
				*line = s;
				s = strndup(t, n);
				fprintf(stderr, "parseExpr: \"%s\" and \"%s\" parsed\n", s, *line);
				return s;
			}
		}
	}
	/* input is not about definition */

	/* detect LHS, RHS by " == ", etc. */


	/* detect the main operation */
		/* by counting total number of operations first? */
	/* parse f, a, b, for f(a, b) */
	return strdup("");
}

EXPR *addExpr(EXPR *p, char *_content)
{
	char *left, *right;
	static unsigned int tab = 0;
	int i;

	if (p == NULL) {
		p = exprAlloc();
		p->_content = strdup(_content);
		if (strcmp(p->_content, "") == 0)
			p->_content = NULL;
		p->left = NULL;
		p->right = NULL;
		for (i = 0; i < tab; i++)
			fprintf(stderr, " ");
		fprintf(stderr, "addExpr: creating expr, _content = \"%s\"\n", p->_content);
		if (p->_content != NULL) {
			right = p->_content;
			for (i = 0; i < tab; i++)
				fprintf(stderr, " ");
			fprintf(stderr, "(1) - parsing left\n");
			left = parseExpr(&right);
			for (i = 0; i < tab; i++)
				fprintf(stderr, " ");
			fprintf(stderr, "(2) - parsing op\n");
			p->op = parseExpr(&right);
			for (i = 0; i < tab; i++)
				fprintf(stderr, " ");
			fprintf(stderr, "(3) - parsing right\n");
			if (strcmp(p->op, "") != 0) {
				for (i = 0; i < tab; i++)
					fprintf(stderr, " ");
				fprintf(stderr, "(4) - creating p->left with left: \"%s\"\n", left);
				tab += 4;
				p->left = addExpr(p->left, left);
				tab -= 4;
				for (i = 0; i < tab; i++)
					fprintf(stderr, " ");
				fprintf(stderr, "(5) - creating p->right with right: \"%s\"\n", right);
				tab += 4;
				p->right = addExpr(p->right, right);
				tab -= 4;
				for (i = 0; i < tab; i++)
					fprintf(stderr, " ");
				fprintf(stderr, "(6)\n");
			}
		}
	}

	return p;
}

char *evalExpr(EXPR *p)
{
	return p->_content;
}

void updateExpr(EXPR *p)
{
	static unsigned int tab = 0;
	int i;

	if (p != NULL) {
		tab += 4;
		updateExpr(p->left);
		updateExpr(p->right);
		tab -= 4;
		p->content = evalExpr(p);	/* reads p->left->content, p->right->content */
		for (i = 0; i < tab; i++)
			fprintf(stderr, " ");
		fprintf(stderr, "updateExpr: _content: \"%s\", content: \"%s\"\n", p->_content, p->content);
	}
}

static void _listExpr(EXPR *p)
{
	static unsigned int tab = 0;
	int i;

	if (p != NULL) {
		for (i = 0; i < tab; i++)
			printf(" ");
		printf("%s\n", p->content);
		tab += 4;
		_listExpr(p->left);
		_listExpr(p->right);
		tab -= 4;
	}
}

/* listExpr: in-order print of tree p */
void listExpr(EXPR *p)
{
	_listExpr(p);
	printf("\b\n");
}

#endif	/* _EXPRESSION_H */
