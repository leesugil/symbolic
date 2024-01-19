/* Symbolic Arithmetic */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <complex.h>

#include "symbol.h"
#include "getword.h"

int main(int argc, char *argv[])
{
	char c;
	SYMB *num = NULL;
	SYMB *denom = NULL;

	printf("input (n)umerator, input (d)enominator\n");
	while (scanf("%s", &c) == 1) {
		switch(c) {
			case('n'):
				printf("n pressed!\n");
				break;
			case('d'):
				printf("d pressed!\n");
				break;
			default:
				printf("nothing pressed!\n");
				break;
		}
	}
	/* input numerator (symbols and numbers) */
		/* create binary tree of inputs */

	/* input denominator (symbols and numbers) */
		/* create binary tree of inputs */

	/* display the fraction in reduced form */
		/* calculate numeric part as much as possible */

	exit(0);
}
