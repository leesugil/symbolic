#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string.h>
#include <stdlib.h>

#define MAXLEN 1000

typedef struct Symb Symb;

struct Symb {
	char *name;
	char formula[MAXLEN];	/* mutable so that it can be updated */
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
		p->formula[MAXLEN-1] = '\0';
		strncpy(p->formula, formula, MAXLEN-1);
		p->left = NULL;
		p->right = NULL;
	} else if ((cond = strcmp(name, p->name)) == 0)
		strncpy(p->formula, formula, MAXLEN-1);
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
		printf("%s = %s\n", p->name, p->formula);
		listSymb(p->right);
	}
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

/* removeSymb: frees a node and its branch below */
void removeSymb(Symb *p)
{
	if (p != NULL) {
		removeSymb(p->left);
		removeSymb(p->right);
		free(p);
	}
}

/* removeSymbBranch: frees child nodes in the branch */
void removeSymbBranch(Symb *p)
{
	removeSymb(p->left);
	removeSymb(p->right);
	p->left = NULL;
	p->right = NULL;
}


#endif	/* _SYMBOL_H */
