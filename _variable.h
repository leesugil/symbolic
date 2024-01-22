#ifndef _VARIABLE_H
#define _VARIABLE_H

#include <string.h>

struct variable {
	char *name;
	double value;			/* complex to be added later */
	struct variable *left;	/* for binary tree */
	struct variable *right;	/* for binary tree */
};

typedef struct variable VAR;

VAR *varAlloc(void)
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
	} else if ((cond = strcmp(name, p->name)) == 0) {
		p->value = value;
	} else if (cond < 0)
		p->left = addVar(p->left, name, value);
	else
		p->right = addVar(p->right, name, value);

	return p;
}

/* listVar: in-order print of tree p */
void listVar(VAR *p)
{
	if (p != NULL) {
		listVar(p->left);
		printf("%s", p->name);
		printf(" ");
		listVar(p->right);
	}
}

/* lookUpVar: returns VAR * if finds a VAR in a given tree with the same name */
VAR *lookUpVar(VAR *p, char* name)
{
	int cond;

	if (p == NULL)
		return NULL;
	else if ((cond = strcmp(p->name, name)) == 0)
		return p;
	else if (cond < 0)
		return lookUpVar(p->left, name);
	else
		return lookUpVar(p->right, name);
}

#endif	/* _VARIABLE_H */
