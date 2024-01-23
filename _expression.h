#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

char *parseExpr(char **line)
{
	/* parse f, a, b, for f(a, b) */
	char output[2];
	output[0] = (*line)[0];
	output[1] = '\0';
	if (strlen(*line) > 0) {
		(*line)++;
	}
	fprintf(stderr, "parseExpr: output: \"%s\", p->_content: \"%s\"\n", output, *line);
	return strdup(output);
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
