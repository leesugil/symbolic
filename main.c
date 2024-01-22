/* Symbolic Arithmetic */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <complex.h>

#include "symbolic.h"
#include "stringe/stringe.h"

#define MAXLINE 1000
SYMB *getSymb(SYMB *, char *, double *);

int main(int argc, char *argv[])
{
	char c[MAXLINE], name[MAXLINE];
	double value = 0.0;
	SYMB *num = NULL;
	SYMB *denom = NULL;
	SYMB *root = NULL;
	SYMB *var = NULL;
	char *line = NULL;
	size_t maxline = 0;

	char *input_command = "input: (n)umerator, (d)enominator, (l)ook-up";

	/* change of structure
	 * accept line and parse */
	
	while (getline(&line, &maxline, stdin) > 0) {
		/* parse line to detect definition, LHS, and RHS */
		parseSymb(root, line);
	}

	while (printf("%s\n", input_command) == strlen(input_command)+1
			&& scanf("%s", c) == 1)
		switch(c[0]) {
			case 'n':
				printf("registering numerator\n");
				num = getSymb(num, name, &value);
				break;
			case 'd':
				printf("registering denominator\n");
				denom = getSymb(denom, name, &value);
				break;
			case 'q':	/* quick quotient */
				/* define a function like 'getsymbs' or 'getvars' to simply accept all the variable names at once separated by empty space.
				 * do this twice for num and denom.
				 * accept them as a line, parse them by empty space,
				 * addSymb one by one with the default value NAN
				 * except those entered with numeric values which
				 * the default value should be obvious. */
				printf("provide the numerator: ");
				break;
			case 'l':
				printf("looking up a variable name\n");
				scanf("%s", name);
				var = lookUpTree(num, name);
				if (var == NULL)
					printf("%s not found\n", name);
				else
					printf("%s found!\n", var->name);
				break;
			default:
				printf("invalid command! '%s' pressed!\n", c);
				break;
	}
	printNum(num);
	printDenom(denom);
	/* input numerator (symbols and numbers) */
		/* create binary tree of inputs */

	/* input denominator (symbols and numbers) */
		/* create binary tree of inputs */

	/* display the fraction in reduced form */
		/* calculate numeric part as much as possible */

	exit(0);
}

/* getSymb: */
SYMB *getSymb(SYMB *p, char *name, double *value)
{
	char *endptr;

	printf("name: ");
	scanf("%s", name);
	*value = strtod(name, &endptr);
	if (*endptr != '\0') {
		printf("value: ");
		scanf("%lg", value);
	}
	printf("the registered value is %s = %g\n", name, *value);
	p = addSymb(p, name, *value);

	return p;
}
