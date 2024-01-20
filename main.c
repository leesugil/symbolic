/* Symbolic Arithmetic */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <complex.h>

#include "symbol.h"
#include "getword.h"

#define MAXLINE 1000
SYMB *getSymb(SYMB *, char *, double *);

int main(int argc, char *argv[])
{
	char c[MAXLINE], name[MAXLINE];
	double value = 0.0;
	SYMB *num = NULL;
	SYMB *denom = NULL;

	printf("input (n)umerator, input (d)enominator\n");
	while (scanf("%s", c) == 1) {
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
			default:
				printf("invalid command! '%s' pressed!\n", c);
				break;
		}
		printf("input (n)umerator, input (d)enominator\n");
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
