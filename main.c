/* Symbolic Arithmetic */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <complex.h>

#include "symbolic.h"
#include "qol/c/getword.h"

int main(int argc, char *argv[])
{
	char *line = NULL;
	size_t maxline = 0;
	Symb *symb = NULL;
	Expr *expr = NULL;

	while (getline(&line, &maxline, stdin) > 0) {
		/* parse line */
		expr = addExpr(expr, line);
		printf("original expr:\n");
		listExpr(expr);
		printf("---\n");

		symb = updateSymb(symb, expr);
		printf("symbols:\n");
		listSymb(symb);
		printf("---\n");

		expr = updateExpr(expr, symb);
		printf("symbols updated expr:\n");
		listExpr(expr);
		printf("---\n");

		expr = evalExpr(expr);
		printf("evaluated expr:\n");
		listExpr(expr);
		printf("---\n");

		expr = distExpr(expr);
		printf("distributed expr:\n");
		listExpr(expr);
		printf("---\n");

		expr = calcExpr(expr);
		printf("calced expr:\n");
		listExpr(expr);
		printf("---\n");

		removeExpr(&expr);
	}

	exit(0);
}
