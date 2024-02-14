#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "expression.h"
#include "operation.h"

#define MAXVAR 10

typedef struct Var Var;

struct Var {
	char name[MAXCHAR];			// "x", mutable
	Expr *expr;					// where this Var shows up in the formula
	Var *right;
};

static Var *varAlloc(void)
{
	return (Var *) malloc(sizeof(Var));
}

/* *v = addVar(*v, (*v)->name, p); */
// *v: var[i] or var[i]->right
// (*v)->name: search word
// p: Expr node
Var *addVar(Var *p, char *name, Expr *x)
{
	char *prog = "addVar";

	if (p == NULL) {
		p = varAlloc();
		strcpy(p->name, name);
		p->expr = x;	// if x == NULL, ?
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
		//printn("\t", tabs);
		printf("\"%s\"", p->name);
		if (p->expr != NULL)
			printf(" = %s", p->expr->name);
		printf("\n");
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
	/* so it looks like they have an empty expression? */
	/* i see the problem. variable's expression is just a link to the function expression which is already vanished when removing funciton. it is enough to nullify the link here */
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


typedef struct Func Func;

Func *func_tree = NULL;			// root functoin tree

Func *loadFunc(Func *p)
{
	return p;
}

void removeFunc(Func **p);

struct Func {
	char name[MAXCHAR];			// "f("
	Expr *expr;					// the formula parsed
	Var *var[MAXVAR];			// name, left for location address, right for next appearance inside expr
	Func *left;
	Func *right;
};

static Func *funcAlloc(void)
{
	return (Func *) malloc(sizeof(Func));
}

void parseFuncName(char w[], char *name);
Func *writeFuncVarName(Func *p, char *name);	// loop through words in line, var[0]->name = "x"
Func *wireFuncVar2Formula(Func *p);				// loop through Expr, detect, set var[0]->expr = 0x00
Func *updateFuncFormula(Func *p);				// loop through var[i]->right, update name at 0x00, re-branching there, refreshExpr(p->expr) postorder traversal

/* addFunc */
/* f(x, y) = x^2 + x * y + y^2 */
/* g(x) = f(x) + 7 */
Func *addFunc(Func *p, char *name, char *formula)
{
	char *prog = "addFunc";

	int cond;

	char func_name[MAXCHAR] = "";
	parseFuncName(func_name, name);		// with '(' if f(x, y)
	
	char symb_name[MAXCHAR] = "";
	strcpy(symb_name, func_name);
	if (symb_name[strlen(symb_name) - 1] == '(')
		bcutstr(symb_name);					// without '('
	

	if (p == NULL) {
		p = funcAlloc();
		strcpy(p->name, func_name);
		p->expr = NULL;
		for (int i = 0; i < MAXVAR; i++)
			p->var[i] = NULL;
		p->left = NULL;
		p->right = NULL;

		p->expr = addExpr(p->expr, formula);	// what if formula == NULL?
		p = writeFuncVarName(p, name);
		p = wireFuncVar2Formula(p);
		symb_tree = addSymb(symb_tree, symb_name, p->expr->name);

		fprintf(stderr, "%s: \"%s\" added to the tree\n", prog, p->name);
	} else if ((cond = strcmp(func_name, p->name)) == 0) {
		/* f could be assigned to a whole different function */
		removeFunc(&p);
		p = addFunc(p, name, formula);
		/* or f(x, y) just needed to be updated to f(x + dx, y + dy) */
		// workflow
		// f = ... or f(x, y) = ...
		// defining a function with a formula.
		// call addFunc
		//
		// just f or f(x + dx, y)
		// f calls f in Symb
		// f(x + dx, y) calls updateFunc which updates var names which updates f->formula which updates f in Symb
	} else if (cond < 0)
		p->left = addFunc(p->left, name, formula);
	else
		p->right = addFunc(p->right, name, formula);

	return p;
}

/* listFunc: in-order print of tree p */
void listFunc(Func *p)
{
	static int tabs = 0;

	if (p != NULL) {
		printn("\t", tabs);
		printf("\"%s\" = \"%s\"\n", p->name, p->expr->name);
		tabs++;
		listFunc(p->left);
		listFunc(p->right);
		tabs--;
	}
}

/* _removeFunc: used in removeFunc */
void _removeFunc(Func *p)
{
	char *prog = "_removeFunc";

	if (p == NULL)
		return;

	_removeFunc(p->left);
	_removeFunc(p->right);

	p->left = NULL;
	p->right = NULL;
	strcpy(p->name, "");
	fprintf(stderr, "%s: removing formula expression %s\n", prog, p->expr->name);
	removeExpr(&(p->expr));
	for (int i = 0; i < MAXVAR && (p->var[i] != NULL); i++) {
		fprintf(stderr, "%s: removing variable %s\n", prog, p->var[i]->name);
		removeVar(&(p->var[i]));
		fprintf(stderr, "%s: variable removed\n", prog);
	}
	free(p);
}

/* removeFunc: frees an expr and its branch below */
void removeFunc(Func **p)
{
	char *prog = "removeFunc";

	Var *v = NULL;
	for (int i = 0; i < MAXVAR && *p != NULL && ((v = (*p)->var[i]) != NULL); i++) {
		while (v != NULL) {
			fprintf(stderr, "%s: removing variable \"%s\" = \"%s\"\n", prog, (*p)->var[i]->name, (*p)->var[i]->expr->name);
		v = v->right;
		}
	}

	_removeFunc(*p);
	*p = NULL;
}

Func *getFunc(Func *p, char *name);
void testaddFunc(void)
{
	op_tree = loadOps(op_tree);
	Func *p = NULL;
	char *name = "f(x, y, z)";
	char *formula = "x^2 + x * y + y^2";
	name = "f(x)";
	formula = "5";

	printf("input: %s = %s\n", name, formula);
	p = addFunc(p, name, formula);
	printf("testaddFunc: listFunc\n");
	listFunc(p);

	printf("testaddFunc: listVar\n");
	for (int i = 0; i < MAXVAR; i++)
		listVar(p->var[i]);

	printf("-------------------------\n");
	name = "g(x, y, z)";
	//formula = "f(x) + y^2 + y";
	formula = "5";

	printf("input: %s = %s\n", name, formula);
	p = addFunc(p, name, formula);
	printf("testaddFunc: listFunc\n");
	listFunc(p);

	printf("testaddFunc: listVar\n");
	Func *f = getFunc(p, "g(");
	for (int i = 0; i < MAXVAR; i++)
		listVar(f->var[i]);

	printf("-------------------------\n");
	printf("removeFunc\n");
	removeFunc(&p);
	printf("listFunc\n");
	listFunc(p);
	printf("listVar of f\n");
	for (int i = 0; i < MAXVAR && p != NULL && p->var[i] != NULL; i++)
		listVar(p->var[i]);
}

char *parseVarName(char w[], char *line);

/* writeFuncVarName: parse function names and register var[0], var[1], ...*/
Func *writeFuncVarName(Func *p, char *name)
{
	if (p == NULL)
		return NULL;

	char var_name[MAXCHAR] = "";

	/* parse variable names only from name */
	for (int i = 0; i < MAXVAR && strlen(name) > 0; i++) {
		name = parseVarName(var_name, name);
		if (name == NULL)
			return p;
		if (strlen(var_name) > 0) {
			p->var[i] = addVar(p->var[i], var_name, NULL);
		}
	}

	return p;
}

// loop through Expr, detect, set var[0]->expr = 0x00
Expr *_wireFuncVar2Formula(Expr *p, Var **v)
{
	char *prog = "_wireFuncVar2Formula";
	
	// given *v = var[i],
	// *v->expr == NULL,
	// traverse all nodes in function Expr,
	// get addresses whenever the name matches.
	// if not detected

	if (p == NULL)
		return NULL;

	p->left = _wireFuncVar2Formula(p->left, v);
	p->right = _wireFuncVar2Formula(p->right, v);

	if (strcmp(p->name, (*v)->name) == 0) {
		if (p->left == NULL)
			*v = addVar(*v, (*v)->name, p);
	}

	return p;
}
Func *wireFuncVar2Formula(Func *p)
{
	char *prog = "wireFuncVar2Formula";

	Var *v;

	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++) {
		if (strlen(v->name) > 0) {
			_wireFuncVar2Formula(p->expr, &v);
		}
	}

	return p;
}
// loop through var[i]->right, update name at 0x00, re-branching there, refreshExpr(p->expr) postorder traversal
Func *updateFuncFormula(Func *p)
{
	Var *v;

	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++) {
		removeExpr(&(v->expr));
		while (v != NULL) {
			v->expr = addExpr(v->expr, v->name);
			v = v->right;
		}
	}

	return p;
}

/* parseFuncName: f(x, y) ==> f( */
void parseFuncName(char w[], char *name)
{
	char *p = strstr(name, "(");
	strcpy(w, name);
	if (p != NULL)
		bcutnstr(w, strlen(p) - 1);
	for (int i = 0; i < strlen(w); i++) {
		if (isalnum(w[i]) || (i == strlen(w)-1 && (isalnum(w[i]) || w[i] == '(')))
			;
		else {
			w[0] = '\0';
			return;
		}
	}
}
/* parseVarName: writes current variable name, returns pointer before the variable name */
/* f(x, y) ==> x, y)
 * y)      ==> )
 * )       ==> "" */
char *parseVarName(char w[], char *line)
{
	if (*(line + strlen(line) - 1) != ')')
		return NULL;
	char *p = strstr(line, "(");		// (x, y)
	if (p != NULL)
		p++;							//  x, y)
	else
		p = line;
	while (isspace(*p))					// x, y)
		p++;
	char *q = strstr(p, ",");			// , y)
	if (q == NULL)							// y)
		q = strstr(p, ")");					// )
	strcpy(w, p);						// x, y)
	firstnstr(w, (q - p));				// x
	q++;								//  y) or ""
	return q;
}

/* getFunc: returns a pointer to the Func found */
Func *getFunc(Func *p, char *name)
{
	int cond;

	if (p == NULL) {
		fprintf(stderr, "getFunc: NULL pointer!\n");
		return NULL;
	}
	else if ((cond = strcmp(name, p->name)) == 0)
		return p;
	else if (cond < 0) {
		fprintf(stderr, "getFunc: taking left from p->name: %s\n", p->name);
		return getFunc(p->left, name);
	} else {
		fprintf(stderr, "getFunc: taking right from p->name: %s\n", p->name);
		return getFunc(p->right, name);
	}
}

/* updateFuncVarName */
Func *rewireFuncVar2Formula(Func *p);

Func *updateFuncVarName(Func *p, char *name)
{
	Var *v;
	char w[MAXCHAR] = "";

	for (int i = 0; i < MAXVAR && (v = p->var[i]) != NULL; i++) {
		name = parseVarName(w, name);
		parenthstr(w);
		while (v != NULL) {
			/* parse new variable name from the input */
			//strcpy(v->name, w);
			/* should check if v->expr is null.
			 * where are functions with redundant variables like
			 * f(x) = 5 */
			if (v->expr != NULL)
				strcpy(v->expr->name, w);
			v = v->right;
		}
	}

	return p;
}

/* updateFunc: f(x + dx, y) */
Func *updateFunc(Func *root, char *name)
{
	char *prog = "updateFunc";

	char func_name[MAXCHAR] = "";
	parseFuncName(func_name, name);		// with '(' if f(x, y)
	
	char symb_name[MAXCHAR] = "";
	strcpy(symb_name, func_name);
	if (symb_name[strlen(symb_name) - 1] == '(')
		bcutstr(symb_name);				// without '('
	
	Func *p = getFunc(root, func_name);

	if (p == NULL) {
		root = addFunc(root, name, "");
		return root;
	}

	p = updateFuncVarName(p, name);		// update var[i]->name and re-branch
	p->expr = refreshExprName(p->expr);
	symb_tree = addSymb(symb_tree, symb_name, p->expr->name);	// signal to Symb

	return root;
}
void testupdateFunc(void)
{
	op_tree = loadOps(op_tree);
	Func *p = NULL;
	char *name = "f(x, y)";
	char *formula = "x^2 + x * y + y^2";

	printf("input: %s = %s\n", name, formula);
	p = addFunc(p, name, formula);
	printf("listFunc\n");
	listFunc(p);

	printf("listVar\n");
	for (int i = 0; i < MAXVAR; i++)
		listVar(p->var[i]);

	printf("f->expr\n");
	printf("%s\n", p->expr->name);

	printf("-----------------------\n");

	name = "f(x + dx, y - dy)";
	printf("testupdateFunc\n");
	p = updateFunc(p, name);
	listFunc(p);

	printf("listVar\n");
	for (int i = 0; i < MAXVAR; i++)
		listVar(p->var[i]);

	printf("f->expr\n");
	printf("%s\n", p->expr->name);
}

/* updateFuncComp: for functions like g(x) = f(x + dx) - f(x),
 * replace the f part with its original formula */
Expr *_updateFuncComp(Expr *p, Func *f);
Func *copyFunc(Func *p, char *func_name, char *new_name);

Func *updateFuncComp(Func *p)
{
	char *prog = "updateFuncComp";

	if (p == NULL)
		return NULL;

	p->expr = _updateFuncComp(p->expr, p);
	p = wireFuncVar2Formula(p);		// wire x once having the (x + dx)^2 - x^2 form
	return p;
}
Expr *_updateFuncComp(Expr *p, Func *f)
{
	char *prog = "_updateFuncComp";

	if (p == NULL)
		return NULL;

	p->left = _updateFuncComp(p->left, f);
	p->right = _updateFuncComp(p->right, f);

	p = refreshExprNode(p);
	printf("%s: %s\n", prog, p->name);

	if (p->left != NULL)
		return p;
	if (p->name[strlen(p->name) - 1] != ')')
		return p;
	/* p->name: f(x + dx) */
	char func_name[MAXCHAR] = "";
	parseFuncName(func_name, p->name);
	char line[MAXCHAR] = "";
	strcpy(line, "d29384hfiue203dwoa09c2wdas(");
	func_tree = copyFunc(func_tree, func_name, line);	// create or overwrite
	/* d29384hfiue203dwoa09c2wdas(x + dx) */
	bcutstr(line);
	strcat(line, strstr(p->name, "("));
	func_tree = updateFunc(func_tree, line);	// (x + dx)^2
	listFunc(func_tree);
	bcutnstr(line, strlen(strstr(line, "(")) - 1);
	Func *q = getFunc(func_tree, line);
	printf("%s\n", line);
	strcpy(p->name, q->expr->name);

	return p;
}
Func *copyFunc(Func *p, char *func_name, char *new_name)
{
	if (p == NULL || func_name == NULL || new_name == NULL)
		return NULL;
	if (strlen(func_name) == 0 || strlen(new_name) == 0)
		return NULL;
	/* create or overwrite */
	Func *f = getFunc(p, new_name);
	if (f == NULL) {
		/* prepare for creating a new function */
		func_tree = addFunc(func_tree, new_name, "");
		f = getFunc(p, new_name);
		removeExpr(&(f->expr));
	} else {
		/* prepare for overwriting a new function */
		removeExpr(&(f->expr));
		for (int i = 0; i < MAXVAR && f->var[i] != NULL; i++)
			removeVar(&(f->var[i]));
	}
	/* copy */
	Func *t = getFunc(p, func_name);
	if (t == NULL)
		return p;
	/* new Expr generated for f */
	f->expr = addExpr(f->expr, t->expr->name);
	/* new Var names added for f */
	for (int i = 0; i < MAXVAR && t->var[i] != NULL; i++)
		f->var[i] = addVar(f->var[i], t->var[i]->name, NULL);
	/* wiring Var names to formula for f */
	f = wireFuncVar2Formula(f);
	/* no need to update the Symb tree */

	return p;
}
void testupdateFuncComp(void)
{
	op_tree = loadOps(op_tree);
	char *name = "f(x)";
	char *formula = "x + 7";
	func_tree = addFunc(func_tree, name, formula);

	name = "g(a)";
	formula = "f(a) - 3";
	func_tree = addFunc(func_tree, name, formula);

	printf("listFunc:\n");
	listFunc(func_tree);

	printf("testupdateFuncComp\n");
	Func *f = getFunc(func_tree, "g(");
	f = updateFuncComp(f);
	listFunc(func_tree);
	printf("listExpr of g\n");
	if (f != NULL)
		listExpr(f->expr);
}

#endif	// _FUNCTION_H
