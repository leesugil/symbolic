#ifndef _EXPRESSION_H
#define _EXPRESSION_H

#include <stdio.h>
#include <string.h>
#include "operation.h"
#include "variable.h"

struct expression {
	char *_content;				/* original text used to create obj */
	char *content;				/* to be updated after reduction */
	struct expression *left;	/* NULL if atom */
	struct expression *right;	/* NULL if atom */
};

typedef struct expression EXPR;

static VAR *exprAlloc(void)
{
	return (VAR *) malloc(sizeof(EXPR));
}

VAR *addExpr(EXPR *p, char *_content)
{
	int cond;

	if (p == NULL) {
		p = exprAlloc();
		p->_content = strdup(_content);
		p->left = NULL;
		p->right = NULL;
		fprintf(stderr, "addExpr: expr added, %s\n", p->_content);
	} else if ((cond = strcmp(name, p->name)) == 0) {
		p->value = value;
		/* i see the problem. no comparing conditions */
		/* how would i keep updating the root tree by providing
		 * token expressions? */
		/* (a + (a + b)) + ((a + b) + b) */
		/* how would addTree(root, "a + b") determine which expr should it be? */
	} else if (cond < 0) {
		fprintf(stderr, "addVar: taking left at p->name = %s\n", p->name);
		p->left = addVar(p->left, name, value);
	}
	else {
		fprintf(stderr, "addVar: taking right at p->name = %s\n", p->name);
		p->right = addVar(p->right, name, value);
	}

	return p;
}

static void _listVar(VAR *p)
{
	if (p != NULL) {
		_listVar(p->left);
		printf("(%s, %g) ", p->name, p->value);
		_listVar(p->right);
	}
}

/* listVar: in-order print of tree p */
void listVar(VAR *p)
{
	_listVar(p);
	printf("\b\n");
}

#endif	/* _EXPRESSION_H */
