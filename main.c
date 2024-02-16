#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbolic.h"
#include "plot/ansi.h"

Expr *processExpr(Expr *p);

int main()
{
	op_tree = loadOps(op_tree);
	symb_tree = loadSymb(symb_tree);
	Expr *p = NULL;
	char line[MAXCHAR] = "";

	/* load pre-load lines */
	FILE *fp;
	FILE *fopen(const char *, const char *);
	fp = fopen("preload.txt", "r");
	fgets(line, MAXCHAR, fp);
	p = addExpr(p, line);
	symb_tree = updateSymb(symb_tree, p);
	removeExpr(&p);

	//clear();
	//print_at(3, 3, "intput:");
	//draw_rect(1, 1, 70, 5, '*');
	//move_to(11, 3);

	char *s = NULL;
	size_t maxline = 0;
	while (getline(&s, &maxline, stdin) > 0) {
		printf("listSymb (before)\n");
		listSymb(symb_tree);
		removeExpr(&p);
		p = addExpr(p, s);
		/* flow control */
		if (strstr(s, " = ") != NULL ||
				strstr(s, ", ") != NULL) {
			p = addExpr(p, line);
			symb_tree = updateSymb(symb_tree, p);
			printf("listSymb (after)\n");
			listSymb(symb_tree);
		} else {
			p = processExpr(p);
		}
		if (strcmp(p->op, ", ") != 0 &&
				strcmp(p->op, " = ") != 0) {
			//clear();
			//move_to(3, 11);
			listExpr(p);
		}
		/* render
		fill_rect(1, 1, 70, 9, ' ');
		draw_rect(1, 1, 70, 5, '*');
		draw_rect(1, 5, 70, 9, '*');
		print_at(3, 7, "output: ");
		if (strcmp(p->op, ", ") != 0 &&
				strcmp(p->op, " = ") != 0)
			print_at(11, 7, "%s", p->name);
		print_at(3, 3, "intput:");

		move_to(11, 3);
		*/
	}

	/* render */
	//clear();
}

Expr *processExpr(Expr *p)
{
	if (p == NULL)
		return NULL;

	//symb_tree = updateSymb(symb_tree, p);

	char prev_p[MAXCHAR] = "";
	do {
		strcpy(prev_p, p->name);
		p = updateExpr(p, symb_tree);
		p = sortExpr(p);
	} while (strcmp(prev_p, p->name) != 0);

	do {
		printf("***************\n***p->name (LOOP START)\"%s\"\n", p->name);
		strcpy(prev_p, p->name);
		p = sortExpr(p);
		printf("***************\n***p->name after sortExpr \"%s\"\n", p->name);
		p = evalExpr(p);
		printf("***************\n***p->name after evalExpr \"%s\"\n", p->name);
		p = altExpr(p);
		printf("***************\n***p->name after altExpr \"%s\"\n", p->name);
		p = evalExpr(p);
		printf("***************\n***p->name after evalExpr \"%s\"\n", p->name);
		p = distExpr(p);
		printf("***************\n***p->name after distExpr \"%s\"\n", p->name);
		p = evalExpr(p);
		printf("***************\n***p->name after evalExpr \"%s\"\n", p->name);
		p = expExpr(p);
		printf("***************\n***p->name after expExpr \"%s\"\n", p->name);
		p = evalExpr(p);
		printf("***************\n***p->name after evalExpr \"%s\"\n", p->name);
		p = commExpr(p);
		printf("***************\n***p->name after commExpr \"%s\"\n", p->name);
		p = evalExpr(p);
		printf("***************\n***p->name after evalExpr \"%s\"\n", p->name);
		p = sortExpr(p);
		printf("***************\n***p->name after sortExpr \"%s\"\n", p->name);
		printf("***************\n***p->name  \"%s\" (LOOP END)\n", p->name);
	} while (strcmp(prev_p, p->name) != 0);

	p = displayExpr(p);

	return p;
}
