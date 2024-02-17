#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string.h>
#include <stdlib.h>

#include "qol/c/getword.h"
#include "expression.h"

#define MAXVAR 10

typedef struct Var Var;

struct Var {
	char name[MAXCHAR];			// "x", mutable
	char initname[MAXCHAR];		// "x" as originally used when defining the function.
	Expr *expr;					// where this Var shows up in the formula
	Var *right;
};

static Var *varAlloc(void)
{
	return (Var *) malloc(sizeof(Var));
}

Var *addVar(Var *p, char *name, Expr *x)
{
	char *prog = "addVar";
	if (name == NULL || strlen(name) == 0)
		return p;

	if (p == NULL) {
		p = varAlloc();
		strcpy(p->name, name);
		strcpy(p->initname, name);
		p->expr = x;
		p->right = NULL;
	} else {
		if (strcmp(p->name, name) != 0)
			strcpy(p->name, name);
		if (p->expr == NULL) {
			p->expr = x;
		} else {
			p->right = addVar(p->right, name, x);
		}
	}

	return p;
}

/* listVar: in-order print of tree p */
void listVar(Var *p)
{
	static int tabs = 0;

	if (p != NULL) {
		printn("\t", tabs);
		printf("\"%s\"", p->name);
		if (p->expr != NULL)
			printf(" = %s\n", p->expr->name);
		else
			printf(" = (null), not found\n");
		tabs++;
		listVar(p->right);
		tabs--;
	}
}

/* _removeVar: used in removeVar */
void _removeVar(Var *p)
{
	char *prog = "_removeVar";

	if (p == NULL)
		return;

	_removeVar(p->right);

	fprintf(stderr, "%s:_removing var \"%s\" = \"%s\"\n", prog, p->name, p->expr->name);

	p->right = NULL;
	strcpy(p->name, "");
	strcpy(p->initname, "");
	/* variable's expression is just a link to the function expression which is already vanished when removing funciton. it is enough to nullify the link here */
	p->expr = NULL;
	free(p);
}

/* removeVar: frees an expr and its branch below */
void removeVar(Var **p)
{
	_removeVar(*p);
	*p = NULL;
}
void testremoveVar(void)
{
	Var *var = NULL;
	char *name = "x";

	printf("listVar\n");
	var = addVar(var, name, NULL);
	listVar(var);

	printf("testremoveVar:\n");
	removeVar(&var);
	listVar(var);
}

typedef struct Symb Symb;

Symb *symb_tree = NULL;

struct Symb {
	char name[MAXCHAR];							// f
	char func_name[MAXCHAR];					// f(
	char full_name[MAXCHAR];					// f( , , )
	Expr *formula;						// g(x) + x * y + z
	Var *var[MAXVAR];
	Symb *left;
	Symb *right;
};

static Symb *symbAlloc(void)
{
	return (Symb *) malloc(sizeof(Symb));
}

void removeSymb(Symb **p);
void parseSymbName(char w[], char *name);
void parseFuncName(char w[], char *name);
Symb *parseVarName(Symb *p, char *name);		// p->var[i]->name
Symb *wireVar2Formula(Symb *p);					// p->var[i]->location
Symb *updateSymbFullName(Symb *p);

Symb *addSymb(Symb *p, char *name, char *formula)
{
	if (name == NULL || strlen(name) == 0)
		return p;
	if (name[strlen(name) - 1] == '\n')
		name[strlen(name) - 1] = '\0';

	int cond;
	char symb_name[MAXCHAR] = "", func_name[MAXCHAR] = "";
	/* name can be f or f( */
	parseSymbName(symb_name, name);			// f
	parseFuncName(func_name, name);			// f( or ""
											// to signal writeVarName

	if (p == NULL) {
		p = symbAlloc();
		strcpy(p->name, symb_name);
		strcpy(p->func_name, func_name);
		/* full_name updated after variables parsed */
		p->full_name[0] = '\0';
		p->formula = NULL;
		if (formula != NULL)
			p->formula = addExpr(p->formula, formula);
		/* variables parsed in an independant function later */
		for (int i = 0; i < MAXVAR; i++)
			p->var[i] = NULL;

		p = parseVarName(p, name);			// p->var[i]->name
		p = wireVar2Formula(p);				// p->var[i]->location
		p = updateSymbFullName(p);

		p->left = NULL;
		p->right = NULL;
	} else if ((cond = strcmp(symb_name, p->name)) == 0) {
		strcpy(p->name, symb_name);
		strcpy(p->func_name, func_name);
		p->full_name[0] = '\0';
		if (p->formula != NULL)
			removeExpr(&(p->formula));
		p->formula = NULL;
		if (formula != NULL)
			p->formula = addExpr(p->formula, formula);
		for (int i = 0; i < MAXVAR; i++)
			removeVar(&(p->var[i]));
		p = parseVarName(p, name);
		p = wireVar2Formula(p);
		p = updateSymbFullName(p);
	} else if (cond < 0) {
		p->left = addSymb(p->left, name, formula);
	} else {
		p->right = addSymb(p->right, name, formula);
	}

	return p;
}

/* listSymb: in-order print of tree p */
void listSymb(Symb *p)
{
	if (p != NULL) {
		listSymb(p->left);
		printf("****************\n");
		if (strlen(p->full_name) > 0)
			printf("\"%s\" = ", p->full_name);
		else
			printf("\"%s\" = ", p->name);
		if (p->formula != NULL)
			printf("\"%s\"\n", p->formula->name);
		else
			printf("(null)\n");
		printf("--- formula\n");
		if (p->formula != NULL)
			listExpr(p->formula);
		printf("--- variables\n");
		for (int i = 0; i < MAXVAR && p->var[i] != NULL; i++)
			listVar(p->var[i]);
		listSymb(p->right);
	}
}
void testlistSymb(void)
{
	Symb *root = NULL;
	op_tree = loadOps(op_tree);

	root = addSymb(root, "x", "5");
	root = addSymb(root, "y", "-1.2e-3");
	root = addSymb(root, "f(x, y)", "x + y");
	root = addSymb(root, "g(f(x))", "f(x) + x + y");

	listSymb(root);

	root = addSymb(root, "g(y)", "(f(x) + x) + y");

	listSymb(root);

	root = addSymb(root, "g(x)", "(f(x) + x) + y");

	listSymb(root);

	root = addSymb(root, "g(f(x), h(x, i(y)))", "(h(x, i(y)) + f(x)) + y");

	listSymb(root);
}

/* getSymb: returns a pointer to the Symb found */
Symb *getSymb(Symb *p, char *name)
{
	int cond;

	if (p == NULL || name == NULL) {
		fprintf(stderr, "getSymb: NULL pointer!\n");
		return NULL;
	} else if ((cond = strcmp(name, p->name)) == 0)
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
void _removeSymb(Symb *p)
{
	if (p != NULL) {
		_removeSymb(p->left);
		_removeSymb(p->right);
		p->left = NULL;
		p->right = NULL;
		removeExpr(&(p->formula));
		for (int i = 0; i < MAXVAR; i++)
			removeVar(&(p->var[i]));
		free(p);
	}
}

void removeSymb(Symb **p)
{
	_removeSymb(*p);
	*p = NULL;
}

/*
void parseSymbName(char w[], char *name)
{
	char *prog = "parseSymbName";

	if (w == NULL)
		return;
	w[0] = '\0';
	if (name == NULL)
		return;
	// case f(x, y) * z //
	// case f(x + dx) - f(x) //

	char *s = strstr(name, "(");
	if (s != NULL) {
		parseFuncName(w, name);
		if (strlen(w) > 0) {
			fprintf(stdout, "%s: bcutstr: \"%s\"\t(before)\n", prog, w);
			bcutstr(w);
			fprintf(stdout, "%s: bcutstr: \"%s\"\t(after)\n", prog, w);
		}
	} else
		strcpy(w, name);
}
void parseFuncArg(char arg[], char *name)
{
	char *prog = "parseFuncArg";

	arg[0] = '\0';
	if (name == NULL || strlen(name) == 0)
		return ;
	if (name[strlen(name) - 1] != ')')
		return ;
	char line[MAXCHAR] = "";
	strcpy(line, name);
	while (is_outer_blocked_blk(line, block_start, block_end, NULL))
		remove_outer_block_blk(line, block_start, block_end);
	if (!is_blocked_properly_blk(line, block_start, block_end, NULL))
		return;
	char *s = strstr(line, "(");
	if (s == NULL)
		return ;
	strcpy(arg, line);
	fcutnstr(arg, s - line + 1);
	fprintf(stdout, "%s: bcutstr: \"%s\"\t(before)\n", prog, arg);
	bcutstr(arg);
	fprintf(stdout, "%s: bcutstr: \"%s\"\t(after)\n", prog, arg);
	if (!is_blocked_properly_blk(arg, block_start, block_end, NULL))
		arg[0] = '\0';
}
void parseFuncName(char w[], char *name)
{
	if (w == NULL)
		return;
	w[0] = '\0';
	if (name == NULL)
		return;
	// case f(x, y) * z //
	if (strlen(name) == 0 || name[strlen(name) - 1] != ')')
		return;

	// case f((x)) //
	// case f(x + dx) - f(x) /
	char *s = strstr(name, "(");
	if (s != NULL) {					// f(
		char arg[MAXCHAR] = "";
		parseFuncArg(arg, name);
		if (strlen(arg) == 0)
			return;
		strncpy(w, name, s - name + 1);
		w[s - name + 1] = '\0';
	} else
		w[0] = '\0';
}
*/
char *_parseVarName(char w[], char *name);
Symb *parseVarName(Symb *p, char *name)		// p->var[i]->name
{
	if (p == NULL || name == NULL || strlen(name) == 0)
		return p;
	char *s = strstr(name, "(");
	if (s == NULL)
		return p;
	if (name[strlen(name) - 1] != ')')
		return p;
	char var_name[MAXVAR] = "";

	/* parse variable names only from name */
	for (int i = 0; i < MAXVAR && strlen(name) > 0; i++) {
		name = _parseVarName(var_name, name);
		if (name == NULL)
			return p;
		if (strlen(var_name) > 0)
			p->var[i] = addVar(p->var[i], var_name, NULL);
	}

	return p;
}
char *_parseVarName(char w[], char *name)
{
	char *prog = "_parseVarName";
	/* consider complicated case such as f(g(x), h(x, i(y)), z) */
	/* or f((x)) which only x should be parsed */
	w[0] = '\0';
	if (name == NULL || strlen(name) == 0)
		return name;
	if (name[strlen(name) - 1] != ')')
		return name;
	char *p = name;
	/* f((x), (y), z) or  (y), z) */
	if (is_blocked_properly_blk(name, block_start, block_end, NULL)) {
		/* f(g(x), h(x, i(y)), z) */
		/* skip f( */
		p = strstr(name, "(");			// ((x))
		if (p != NULL)
			p++;						//  (x))
		else
			p = name;
	}
	/* g(x), h(x, i(y)), z) */
	/* parse g(x) */
	while(isspace(*p))
		p++;
	char *q = strstrmaskblk(p, ",", NULL, block_start, block_end);
	if (q == NULL) {
		q = strrstr(p, ")");
		if (q == NULL) {
			w[0] = '\0';
			return name;
		}
	}
	strncpy(w, p, q - p);
	w[q - p] = '\0';
	while (isspace(w[strlen(w) - 1]))
		w[strlen(w) - 1] = '\0';
	fprintf(stderr, "%s: parsed variable name: \"%s\"\n", prog, w);
	while (is_outer_blocked_blk(w, block_start, block_end, NULL))
		remove_outer_block_blk(w, block_start, block_end);
	fprintf(stderr, "%s: blocks removed: \"%s\"\n", prog, w);
	
	return ++q;
}
// loop through Expr, detect, set var[0]->expr = 0x00
Expr *_wireVar2Formula(Expr *p, Var **v)
{
	char *prog = "_wireFuncVar2Formula";
	
	// given *v = var[i],
	// *v->expr == NULL,
	// traverse all nodes in function Expr,
	// get addresses whenever the name matches.
	// if not detected

	if (p == NULL || *v == NULL)
		return p;

	p->left = _wireVar2Formula(p->left, v);
	p->right = _wireVar2Formula(p->right, v);

	if (strcmp(p->name, (*v)->name) == 0) {
		//if (p->left == NULL)
		*v = addVar(*v, (*v)->name, p);
	}

	return p;
}
Symb *wireVar2Formula(Symb *p)					// p->var[i]->location
{
	char *prog = "wireVar2Formula";

	Var *v;

	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++)
		if (strlen(v->name) > 0)
			_wireVar2Formula(p->formula, &v);

	return p;
}
Symb *updateSymbFullName(Symb *p)
{
	char *prog = "updateSymbFullName";

	if (p == NULL)
		return NULL;
	if (strlen(p->func_name) == 0) {
		strcpy(p->full_name, p->name);
		return p;
	}
	strcpy(p->full_name, p->func_name);
	Var *v;
	char line[MAXCHAR] = "", var_name[MAXCHAR] = "";
	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++) {
		//sprintf(line, "%s, ", v->initname);	this feature has been replaced by running restoreVarName and then updateSymbFullName
		strcpy(var_name, v->name);
		while (is_outer_blocked_blk(var_name, block_start, block_end, NULL))
			remove_outer_block_blk(var_name, block_start, block_end);
		sprintf(line, "%s, ", var_name);
		strcat(p->full_name, line);
	}
	fprintf(stdout, "%s: bcutnstr: \"%s\"\t(before)\n", prog, p->full_name);
	bcutnstr(p->full_name, 2);
	fprintf(stdout, "%s: bcutnstr: \"%s\"\t(after)\n", prog, p->full_name);
	strcat(p->full_name, ")");

	return p;
}

Var *_updateVarName2Formula(Var *p)
{
	if (p == NULL || p->expr == NULL)
		return p;

	p->right = _updateVarName2Formula(p->right);

	char line[MAXCHAR] = "";
	strcpy(line, p->name);
	parenthstr(line);
	strcpy(p->expr->name, line);		// parenthstr needed because this update doesn't change p->expr->op which is the determinant of putting parenthesis when refreshing an Expr.
										// this yields f((x))

	return p;
}
Var *_updateVarName(Var *v, char *name)
{
	if (v == NULL || name == NULL || strlen(name) == 0)
		return v;

	v->right = _updateVarName(v->right, name);

	strcpy(v->name, name);

	return v;
}
Symb *updateVarName(Symb *p, char *name)
{
	char *prog = "updateVarName";

	if (p == NULL || name == NULL || strlen(name) == 0)
		return p;

	Var *v;
	char var_name[MAXCHAR] = "";
	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++) {
		name = _parseVarName(var_name, name);
		if (name == NULL) {
			fprintf(stderr, "%s: there is a mismatch in the number of arguments provided in name and the actual number of argument registered in the symbol.\n", prog);
			return p;
		}
		v = _updateVarName(v, var_name);
		/* update them in formula Expr as well */
		v = _updateVarName2Formula(v);
	}

	p->formula = refreshExprName(p->formula);

	return p;
}
Symb *restoreVarName(Symb *p)
{
	if (p == NULL || strlen(p->full_name) == 0)
		return p;

	Var *v = NULL;
	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++)
		strcpy(v->name, v->initname);
	p = updateSymbFullName(p);
	p = updateVarName(p, p->full_name);

	return p;
}

/* evalSymb: "f(y, g)" ==> scan "f(", eval y, eval g, eval f(eval(y), eval(g)), record in w */
void evalSymb(char w[], char *name, Symb *tree)
{
	char *prog = "evalSymb";
	fprintf(stderr, "%s: START \n", prog);

	if (w == NULL || name == NULL || tree == NULL)
		return ;
	w[0] = '\0';

	while (is_outer_blocked_blk(w, block_start, block_end, NULL))
		remove_outer_block_blk(w, block_start, block_end);

	char func_name[MAXCHAR] = "", symb_name[MAXCHAR] = "";
	parseFuncName(func_name, name);		// f( or ""
	parseSymbName(symb_name, name);

	fprintf(stderr, "%s: input name: \"%s\"\n", prog, name);
	fprintf(stderr, "%s: func_name: \"%s\"\n", prog, func_name);
	fprintf(stderr, "%s: symb_name: \"%s\"\n", prog, symb_name);

	Symb *p = getSymb(tree, symb_name);
	if (p == NULL) {
		strcpy(w, symb_name);	// no need to parenthesize
		fprintf(stderr, "%s: \"%s\" not found in the symbol tree\n", prog, symb_name);
		return ;
	}
	/* g = G * M * m / r^2
	 * g */
	if (strlen(p->func_name) == 0) {
		if (p->formula != NULL) {
			if (strlen(p->formula->name) > 0) {
				evalSymb(w, p->formula->name, tree);
				parenthstr(w);
			} else
				strcpy(w, symb_name);
			fprintf(stderr, "%s: evaluated formula = \"%s\"\n", prog, w);
			fprintf(stderr, "%s: END \n", prog);
			return;
		} else {
			strcpy(w, symb_name);	// no need to parenthesize
			fprintf(stderr, "%s: evaluated formula = \"%s\"\n", prog, w);
			fprintf(stderr, "%s: END \n", prog);
			return ;
		}
	}
	if (strcmp(func_name, p->func_name) != 0) {
		/* return the initially registered form */
		fprintf(stderr, "%s: symbol info found but argument information mismatch, returning the originally registered form\n", prog);
		fprintf(stderr, "%s: input \"%s\"\n", prog, name);
		fprintf(stderr, "%s: func_name \"%s\"\n", prog, func_name);
		fprintf(stderr, "%s: p->func_name \"%s\"\n", prog, p->func_name);
		p = restoreVarName(p);
		fprintf(stderr, "%s: returning p->formula->name \"%s\"\n", prog, p->formula->name);
		strcpy(w, p->formula->name);
		fprintf(stderr, "%s: END \n", prog);
		return ;
	}

	/* make a copy of s? maybe not needed. instead call restoreVarName at the end of each of this call.
	 * restoreVarName added.
	 * (without restoreVarName, the expression for the formula of a Symb from listSymb could be funny as the static full_name and dynamic formula->name are used at the same time. */
	p = updateVarName(p, name);		/* s->expr->name now updated, like
									 f(x) = x + 2
									 ==> f(y) = (y) + 2.

									 next step: see if y = g(z) in Symb,
									 if so,
									 f(y) = (y) + 2
									 ==> f(g(z)) = (g(z)) + 2
									 
									 this could've been done at once
									 by just working on
									 f(y)
									 ==> f(g(z))
									 ==> f(g(z(x)))
									 ==> f(g(x^2 + 1))
									 ==> f(2 * (x^2 + 1) + 3)
									 and running updateVarName once. */
	fprintf(stderr, "%s:listSymb of p->name:%s(before)\n", prog, p->name);
	//listSymb(p);
	//listSymb(p);
	Var *v = NULL;
	char line[MAXCHAR] = "";
	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++) {
		evalSymb(line, v->name, tree);
		fprintf(stderr, "%s:at p:%s, var[i]->name:%s(before)\n", prog, p->name, v->name);
		if (strlen(line) > 0)
			strcpy(v->name, line);
		// this is a problem when v->name is like "x + dx",
		// and evalSymb returning "" because they couldn't find anything.
		fprintf(stderr, "%s:at p:%s, var[i]->name:%s(after)\n", prog, p->name, v->name);
	}
	fprintf(stderr, "%s:full_name:%s(before)\n", prog, p->full_name);
	p = updateSymbFullName(p);
	fprintf(stderr, "%s:full_name:%s(after)\n", prog, p->full_name);
	p = updateVarName(p, p->full_name);
	fprintf(stderr, "%s:listSymb of p->name:%s(after)\n", prog, p->name);
	//listSymb(p);
	//listSymb(p);
	strcpy(w, p->formula->name);
	p = restoreVarName(p);

	fprintf(stderr, "%s: END \n", prog);
}
void testevalSymb(void)
{
	op_tree = loadOps(op_tree);
	Symb *p = NULL;
	char *name = "f(x, y, z)";
	char *line = "2 * x + 3 * x - z";
	p = addSymb(p, name, line);

	name = "g(x, y, z)";
	line = "x^2 + 1 - z";
	p = addSymb(p, name, line);

	listSymb(p);

	char w[MAXCHAR] = "";
	evalSymb(w, "f(g(x, y, z), y, z)", p);
	printf("testevalSymb:\"%s\"\n", w);

	evalSymb(w, "f(x + 1, y, z)", p);
	printf("testevalSymb:\"%s\"\n", w);
}




/* run at the beginning of the code to pre-load symbols */
Symb *loadSymb(Symb *s)
{
	return s;
}

#endif	/* _SYMBOL_H */
