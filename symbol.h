#ifndef SYMBOL_H
#define SYMBOL_H

#include <math.h>
#include <complex.h>
#include <string.h>

#include "constants.h"

struct symbol {
	char *name;
	double value;			/* complex to be added later */
	unsigned int power;			/* this is for displaying power correctly */
	struct symbol *left;	/* for binary tree */
	struct symbol *right;	/* for binary tree */
};

typedef struct symbol SYMB;

SYMB *symbAlloc(void)
{
	return (SYMB *) malloc(sizeof(SYMB));
}

SYMB *addSymb(SYMB *p, char *name, double value)
{
	int cond;

	if (p == NULL) {
		p = symbAlloc();
		p->name = strdup(name);
		p->value = value;
		p->power = 1;
		p->left = NULL;
		p->right = NULL;
	} else if ((cond = strcmp(name, p->name)) == 0) {
		p->value = value;
		p->power++;
	} else if (cond < 0)
		p->left = addSymb(p->left, name, value);
	else
		p->right = addSymb(p->right, name, value);

	return p;
}

/* printTree: in-order print of tree p */
void printTree(SYMB *p)
{
	if (p != NULL) {
		printTree(p->left);
		printf("%s", p->name);
		if (p->power > 1)
			printf("^%u", p->power);
		printf(" ");
		printTree(p->right);
	}
}

void printNum(SYMB *p)
{
	if (p != NULL) {
		printTree(p);
	} else
		printf("1 ");
}

void printDenom(SYMB *p)
{
	if (p != NULL) {
		printf("/ ");
		printTree(p);
	}
	printf("\b\n");
}

/* lookUpTree: returns SYMB * if finds a SYMB in a given tree with the same name */
SYMB *lookUpTree(SYMB *p, char* name)
{
	int cond;

	if (p == NULL)
		return NULL;
	else if ((cond = strcmp(p->name, name)) == 0)
		return p;
	else if (cond < 0)
		return lookUpTree(p->left, name);
	else
		return lookUpTree(p->right, name);
}

#endif	/* SYMBOL_H */
