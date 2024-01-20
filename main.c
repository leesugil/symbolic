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

int main(int argc, char *argv[])
{
	char c[MAXLINE], name[MAXLINE];
	double value;
	SYMB *num = NULL;
	SYMB *denom = NULL;

	printf("input (n)umerator, input (d)enominator\n");
	while (scanf("%s", c) == 1) {
		switch(c[0]) {
			case 'n':
				printf("registering numerator\n");
				printf("\tname: ");
				scanf("%s", name);
				printf("\tvalue: ");
				scanf("%lg", &value);
				printf("\tthe registered value is %s = %g\n", name, value);
				num = addSymb(num, name, value);
				break;
			case 'd':
				printf("registering denominator\n");
				printf("\tname: ");
				scanf("%s", name);
				printf("\tvalue: ");
				scanf("%lg", &value);
				printf("\tthe registered value is %s = %g\n", name, value);
				denom = addSymb(denom, name, value);
				break;
			default:
				printf("invalid command! '%s' pressed!\n", c);
				break;
		}
		printf("input (n)umerator, input (d)enominator\n");
	}
	printTree(num);
	printf("/ ");
	printTree(denom);
	printf("\n");
	/* input numerator (symbols and numbers) */
		/* create binary tree of inputs */

	/* input denominator (symbols and numbers) */
		/* create binary tree of inputs */

	/* display the fraction in reduced form */
		/* calculate numeric part as much as possible */

	exit(0);
}
