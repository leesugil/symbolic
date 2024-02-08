#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbolic.h"
#include "plot/ansi.h"

Expr *processExpr(Expr *p);

int main()
{
	op_tree = loadOps(op_tree);
	Op *op = getOp(op_tree, "^");
	Expr *p = NULL;
	char line[MAXCHAR] = "";

	/* load pre-load lines */
	FILE *fp;
	FILE *fopen(const char *, const char *);
	fp = fopen("preload.txt", "r");
	fgets(line, MAXCHAR, fp);
	p = addExpr(p, line);
	p = processExpr(p);
	removeExpr(&p);
	p = addExpr(p, "g");
	p = processExpr(p);

	clear();
	print_at(3, 3, "intput:");
	draw_rect(1, 1, 70, 5, '*');
	move_to(11, 3);

	char *s = NULL;
	size_t maxline = 0;
	while (getline(&s, &maxline, stdin) > 0) {
		removeExpr(&p);
		p = addExpr(p, s);
		p = processExpr(p);
		if (strcmp(p->op, ", ") != 0 &&
				strcmp(p->op, " = ") != 0) {
			clear();
			move_to(3, 11);
			listExpr(p);
		}
		/* render */
		fill_rect(1, 1, 70, 9, ' ');
		print_at(3, 7, "output: %s", p->name);
		print_at(3, 3, "intput:");
		draw_rect(1, 1, 70, 5, '*');
		draw_rect(1, 5, 70, 9, '*');

		move_to(11, 3);
	}

	/* render */
	//clear();
}

Expr *processExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	symb_tree = updateSymb(symb_tree, p);
	p = updateExpr(p, symb_tree);

	char prev_p[MAXCHAR] = "";
	do {
		strcpy(prev_p, p->name);
		p = evalExpr(p);
		p = altExpr(p);
		p = distExpr(p);
		p = expExpr(p);
		p = commExpr(p);
		p = evalExpr(p);
	} while (strcmp(prev_p, p->name) != 0);

	return p;
}
