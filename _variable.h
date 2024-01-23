#ifndef _VARIABLE_H
#define _VARIABLE_H

#include <string.h>
#include <stdlib.h>

struct variable {
	char *name;
	double value;			/* complex to be added later */
	struct variable *left;	/* for binary tree */
	struct variable *right;	/* for binary tree */
};

typedef struct variable VAR;

static VAR *varAlloc(void)
{
	return (VAR *) malloc(sizeof(VAR));
}

VAR *addVar(VAR *p, char *name, double value)
{
	int cond;

	if (p == NULL) {
		p = varAlloc();
		p->name = strdup(name);
		p->value = value;
		p->left = NULL;
		p->right = NULL;
		fprintf(stderr, "addVar: variable added, %s = %g\n", p->name, p->value);
	} else if ((cond = strcmp(name, p->name)) == 0) {
		p->value = value;
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

/* lookUpVar: returns VAR * if finds a VAR in a given tree with the same name */
VAR *lookUpVar(VAR *p, char* name)
{
	int cond;

	if (p == NULL)
		return NULL;
	else if ((cond = strcmp(name, p->name)) == 0)
		return p;
	else if (cond < 0) {
		fprintf(stderr, "lookUpVar: taking left from p->name: %s\n", p->name);
		return lookUpVar(p->left, name);
	} else {
		fprintf(stderr, "lookUpVar: taking right from p->name: %s\n", p->name);
		return lookUpVar(p->right, name);
	}
}

#endif	/* _VARIABLE_H */
