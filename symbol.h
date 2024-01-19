#ifndef SYMBOL_H
#define SYMBOL_H

#include <math.h>
#include <complex.h>
#include <string.h>

struct symbol {
	char *name;
	double value;			/* complex to be added later */
	struct symbol *left;		/* for binary tree */
	struct symbol *right;		/* for binary tree */
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
	} else if ((cond = strcmp(name, p->name)) == 0) {
		fprintf(stderr, "updating %s: %g -> %g\n", p->name, p->value, value);
		p->value = value;
	} else if (cond < 0)
		p->left = addSymb(p->left, name, value);
	else
		p->right = addSymb(p->right, name, value);

	return p;
}

#endif	/* SYMBOL_H */
