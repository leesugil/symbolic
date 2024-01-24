#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "qol/c/getword.h"

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

/* for using with strpbrk */
static char *BLOCK_START = {
	"(",		/* parenthesis */
	"[",		/* bracket */
	"{"			/* curly brace */
}

/* for using with strpbrk */
static char *BLOCK_END = {
	")",		/* parenthesis */
	"]",		/* bracket */
	"}"			/* curly brace */
}

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

static char *OP_DIV[] = {
	" + ",
	" - ",
	" * ",
	" / ",
	" % ",
	" ",
	"^",
};

static unsigned int OP_DIV_N = sizeof(OP_DIV) / sizeof(OP_DIV[0]);

/* getDIV: all the dividers for parsing expr as a union */
static char **getDIV(void)
{
	unsigned int DIV_N = DEFN_DIV_N + EQN_DIV_N + OP_DIV_N;
	char *tmp_union[DIV_N];
	char **output = malloc(DIV_N * sizeof(char *));
	int i, j = 0;

	if (output == NULL) {
		fprintf(stderr, "getDIV: malloc failure, returning NULL\n");
		return NULL;
	}

	/* make temporary char *[] first */
	for (i = 0; i < DEFN_DIV_N; i++, j++)
		tmp_union[j] = DEFN_DIV[i];
	for (i = 0; i < EQN_DIV_N; i++, j++)
		tmp_union[j] = EQN_DIV[i];
	for (i = 0; i < OP_DIV_N; i++, j++)
		tmp_union[j] = OP_DIV[i];

	/* dup to output */
	for (i = 0; i < DIV_N; i++) {
		output[i] = strdup(tmp_union[i]);
		if (output[i] == NULL) {
			for (j = 0; j < i; j++)
				free(output[j]);
			free(output);
			fprintf(stderr, "getDIV: dup failure, returning NULL\n");
			return NULL;
		}
		fprintf(stderr, "getDIV: output[%d] = \"%s\"\n", i, output[i]);
	}

	fprintf(stderr, "getDIV: returning valid char *output[n]\n");
	return output;
}

/* parseExpr: divides mathematical expressions or statements and returns the first piece */
char *parseExpr(char **line)
{
	fprintf(stderr, "parseExpr: parsing \"%s\"\n", *line);
	int i, n;
	char *s = NULL, *t = NULL;
	char **DIV;
	unsigned int DIV_N = DEFN_DIV_N + EQN_DIV_N + OP_DIV_N;

	/* use of DIV */
	DIV = getDIV();
	/* "(a + b) * c"
	 * !=
	 * "(a", " + ", "b) * c"
	 * find a way to mask the parenthesis when testing against strstr */
	for (i = 0; i < DIV_N; i++) {
		fprintf(stderr, "parseExpr: DIV[%d] = \"%s\"\n", i, DIV[i]);
		if((s = strstr(*line, DIV[i])) != NULL) {
			/* DEFN_DIV[i] detected */
			fprintf(stderr, "parseExpr: DIV[%d] = \"%s\" detected, s = \"%s\", *line = \"%s\"\n", i, DIV[i], s, *line);
			/* get the first part */
			if ((n = s - *line) == 0) {
				fprintf(stderr, "parseExpr: s == *line, parsing \"%s\"\n", DIV[i]);
				/* means the divider should be taken out as the output */
				*line = s + strlen(DIV[i]);
				t = strndup(s, strlen(DIV[i]));
				fprintf(stderr, "parseExpr: \"%s\" and \"%s\" parsed\n", t, *line);
				return t;
			} else {
				fprintf(stderr, "parseExpr: s != *line, parsing up to (but not including) \"%s\"\n", DIV[i]);
				t = *line;
				*line = s;
				s = strndup(t, n);
				fprintf(stderr, "parseExpr: \"%s\" and \"%s\" parsed\n", s, *line);
				return s;
			}
		} else {
			fprintf(stderr, "parseExpr: \"%s\" (DIV[%d]) not detected in \"%s\" (*line). (%d/%d)\n", DIV[i], i, *line, i, DIV_N - 1);
		}
	}

	return NULL;
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
		p->content = p->_content;
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
			left = parseExpr(&right);	/* this makes unary operations
										 like '-' difficult to */
			for (i = 0; i < tab; i++)
				fprintf(stderr, " ");
			fprintf(stderr, "(2) - parsing op\n");
			p->op = parseExpr(&right);	/* be captured */
			for (i = 0; i < tab; i++)
				fprintf(stderr, " ");
			fprintf(stderr, "(3) - parsing right\n");
			/*if (strcmp(p->op, "") != 0) {*/
			if (p->op != NULL) {
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

/* evalExpr: update p->content */
	/* the display of the (binary) operator p->op will be checked here.
	 * the string p->op will be compared to OP to determine its display form?
	 * future work required here */
char *evalExpr(EXPR *p)
{
	fprintf(stderr, "evalExpr: p->_content = \"%s\"\n", p->_content);

	if (p->op == NULL)
		return p->_content;

	char *left = NULL;
	char *op;
	char *right = NULL;
	
	if (p->left != NULL) {
		/*left = parenthword(p->left->content);*/
		left = p->left->content;
	}
	op = p->op;
	if (p->right != NULL) {
		/*right = parenthword(p->right->content);*/
		right = p->right->content;
	}
	fprintf(stderr, "evalExpr: left, op, right = \"%s\", \"%s\", \"%s\" at \"%s\".\n", left, op, right, p->content);
	int n = 0;
	if (left != NULL)
		n += strlen(left);
	n += strlen(op);
	if (right != NULL)
		n += strlen(left);
	char output[n+2+1];
	sprintf(output, "%s %s %s", left, op, right);
	fprintf(stderr, "evalExpr: outcome = \"%s\"\n", output);

	return strdup(output);
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
