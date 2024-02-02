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
	Symb *symb = NULL;
	Expr *expr = NULL;

	/* one cycle */
	removeExpr(&expr);
	/*line = "f = a * x^2 + b * x^1 + c * x^0, "
			"g = y * f, "
			"h = g * z, "
			"i = -1 * h, "
			"j = i * -1, "
			"x = 2, "
			"y = 3, "
			"z = 7";*/
	line = "f = (a - b) * c, "
		"a = 1, "
		"b = 1";
	expr = addExpr(expr, line);
	symb = updateSymb(symb, expr);
	expr = updateExpr(expr, symb);
	expr = evalExpr(expr);
	expr = distExpr(expr);
	expr = commExpr(expr);
	expr = calcExpr(expr);
	//printf("%s\n", expr->name);

	/* one cycle */
	removeExpr(&expr);
	line = "f";
	printf("original expr:\n");
	expr = addExpr(expr, line);
	listExpr(expr);
	printf("---\n");
	printf("symbols:\n");
	symb = updateSymb(symb, expr);
	listSymb(symb);
	printf("---\n");
	printf("updated expr with symbols:\n");
	expr = updateExpr(expr, symb);
	listExpr(expr);
	printf("---\n");
	printf("evaluated expr:\n");
	expr = evalExpr(expr);
	listExpr(expr);
	printf("---\n");
	printf("distributed expr:\n");
	expr = distExpr(expr);
	listExpr(expr);
	printf("---\n");
	printf("commutated expr:\n");
	expr = commExpr(expr);
	listExpr(expr);
	printf("---\n");
	printf("calculated expr:\n");
	expr = calcExpr(expr);
	listExpr(expr);
	printf("---\n");
	printf("%s\n", expr->name);

	line = NULL;
	size_t maxline = 0;
	while (getline(&line, &maxline, stdin) > 0) {
		removeExpr(&expr);
		printf("original expr:\n");
		expr = addExpr(expr, line);
		listExpr(expr);
		printf("---\n");
		printf("symbols:\n");
		symb = updateSymb(symb, expr);
		listSymb(symb);
		printf("---\n");
		printf("updated expr with symbols:\n");
		expr = updateExpr(expr, symb);
		listExpr(expr);
		printf("---\n");
		printf("evaluated expr:\n");
		expr = evalExpr(expr);
		listExpr(expr);
		printf("---\n");
		printf("distributed expr:\n");
		expr = distExpr(expr);
		listExpr(expr);
		printf("---\n");
		printf("commutated expr:\n");
		expr = commExpr(expr);
		listExpr(expr);
		printf("---\n");
		printf("calculated expr:\n");
		expr = calcExpr(expr);
		listExpr(expr);
		printf("---\n");
		printf("%s\n", expr->name);
		printf("symbols:\n");
		listSymb(symb);
		printf("---\n");
	}

	exit(0);
}
