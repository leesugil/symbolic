#ifndef SYMBOL_H
#define SYMBOL_H

#include <math.h>
#include <complex.h>
#include <string.h>

#include "constants.h"

struct symbol {
	char *name;
	double value;			/* complex to be added later */
	unsigned int power;		/* for displaying power correctly */
	int expr;				/* 0: LHS, 1: RHS, -1: neither */
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
		p->expr = -1;
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

/* deprecated? */
void printNum(SYMB *p)
{
	if (p != NULL) {
		printTree(p);
	} else
		printf("1 ");
}

/* deprecated? */
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

SYMB *parseExpr(SYMB *p, char *line, int expr)
{
	/* when variables are initially defined,
	 * power = 1 and expr = 0 (LHS) by default.
	 * how do we know if this new variable is
	 * meant to be power = 1 and expr = 1 from the beginning?
	 * solution: make the default expr value to be -1
	 * for not having been assigned to either side. */
	/* example: G M m r^-23 */
	/* note that getword would consider r^-2.3 as six words */
	/* use getwordg with crit = ismathexpr to take it as
	 * r, ^, -2.3 */
	int max = strlen(line);
	char c, name[max];
	SYMB *ptr = NULL;
	while ((c = sgetwordg(&line, name, max, ismathexpr)) != '\0') {
		/* add a way to flag ^ */
		/* work to be continued from here */
		if (ptr = lookUpTree(p, name)) {
			/* update symb */
		} else {
			/* add symb */
		}
	}
}

SYMB *parseSymb(SYMB *p, char *line)
{
	/* check if the line is for defining variables */
	char *delimiters = ",";
	char token* = strtok(line, delimiters);
	if (strcmp(line, token)) {
		while (token != NULL) {
			/* create a new SYMB called symb */
			char *name = NULL;
			double value = NAN;
			if (sscanf(token, "%c = %g", name, &value) == 2) {
				p = addSymb(p, name, value);
				fprintf(stderr, "%c = %g parsed\n", name, value);
				/* does this accept NAN as value? */
			}
			else
				fprintf(stderr, "error: invalid variable definition\n");
			token = strtok(NULL, delimiter);
		}
		return p;
	} else {
		/* seperate LHS and RHS */
		delimiters = "==";
		token = strtok(line, delimiters);
		int expr = 0;
		while (token != NULL) {
			p = parseExpr(p, token, expr);
			expr++;
		}
		return p;
	}
}

#endif	/* SYMBOL_H */
