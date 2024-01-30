#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string.h>
#include <stdlib.h>

#include "qol/c/getword.h"

typedef struct Symb Symb;

struct Symb {
	char *name;
	char formula[MAXCHAR+1];
	Symb *left;
	Symb *right;
};

static Symb *symbAlloc(void)
{
	return (Symb *) malloc(sizeof(Symb));
}

Symb *addSymb(Symb *p, char *name, char *formula)
{
	int cond;

	if (p == NULL) {
		p = symbAlloc();
		p->name = strdup(name);
		//p->formula[MAXCHAR] = '\0';
		strncpy(p->formula, formula, MAXCHAR);
		p->left = NULL;
		p->right = NULL;
	} else if ((cond = strcmp(name, p->name)) == 0)
		strncpy(p->formula, formula, MAXCHAR);
	else if (cond < 0)
		p->left = addSymb(p->left, name, formula);
	else
		p->right = addSymb(p->right, name, formula);

	return p;
}

/* listSymb: in-order print of tree p */
void listSymb(Symb *p)
{
	if (p != NULL) {
		listSymb(p->left);
		printf("\"%s\" = \"%s\"\n", p->name, p->formula);
		listSymb(p->right);
	}
}
void testlistSymb(void)
{
	Symb *root = NULL;

	root = addSymb(root, "x", "5");
	root = addSymb(root, "y", "-1.2e-3");
	root = addSymb(root, "f", "x + y");

	listSymb(root);
}

/* getSymb: returns a pointer to the Symb found */
Symb *getSymb(Symb *p, char *name)
{
	int cond;

	if (p == NULL) {
		fprintf(stderr, "getSymb: NULL pointer!\n");
		return NULL;
	}
	else if ((cond = strcmp(name, p->name)) == 0)
		return p;
	else if (cond < 0) {
		fprintf(stderr, "getSymb: taking left from p->name: %s\n", p->name);
		return getSymb(p->left, name);
	} else {
		fprintf(stderr, "getSymb: taking right from p->name: %s\n", p->name);
		return getSymb(p->right, name);
	}
}

/* resetSymb: resets fomula for a variable to null */
Symb *resetSymb(Symb *p)
{
	if (p == NULL)
		return NULL;

	p->formula[0] = '\0';
	return p;
}
void testresetSymb(void)
{
	Symb *p = NULL;
	char *name = "x";
	char *formula = "3.14";

	p = addSymb(p, name, formula);
	listSymb(p);
	
	Symb *q = getSymb(p, name);
	q = resetSymb(q);
	listSymb(p);
}

/* removeSymb: frees a node and its branch below */
/* see also: resetSymb */
void _removeSymb(Symb *p)
{
	if (p != NULL) {
		_removeSymb(p->left);
		_removeSymb(p->right);
		p->left = NULL;
		p->right = NULL;
		free(p);
	}
}

void removeSymb(Symb **p)
{
	_removeSymb(*p);
	*p = NULL;
}

#endif	/* _SYMBOL_H */
