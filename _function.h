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
		printf("\"%s\"\n", p->name);
		if (p->expr != NULL)
			printf("%s\n", p->expr->name);
		tabs++;
		listVar(p->right);
		tabs--;
	}
}

/* _removeVar: used in removeVar */
void _removeVar(Var *p)
{
	if (p == NULL)
		return;

	_removeVar(p->right);

	printf("_removing var %s\n", p->name);

	p->right = NULL;
	strcpy(p->name, "");
	removeExpr(&(p->expr));
	free(p);
}

/* removeVar: frees an expr and its branch below */
void removeVar(Var **p)
{
	_removeVar(*p);
	*p = NULL;
}


typedef struct Func Func;

Func *func_tree = NULL;			// root functoin tree

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

Func *addFunc(Func *p, char *name, char *formula)
{
	char *prog = "addFunc";

	int cond;

	char func_name[MAXCHAR] = "";
	parseFuncName(func_name, name);		// with '('
	
	char symb_name[MAXCHAR] = "";
	strcpy(symb_name, func_name);
	bcutstr(symb_name);					// without '('

	if (p == NULL) {
		p = funcAlloc();
		strcpy(p->name, func_name);
		//if (p->name[strlen(p->name)-1] == '\n')
		//	p->name[strlen(p->name)-1] = '\0';
		p->expr = NULL;
		for (int i = 0; i < MAXVAR; i++)
			p->var[i] = NULL;
		p->left = NULL;
		p->right = NULL;

		p->expr = addExpr(p->expr, formula);
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
		printf("\"%s\"\n", p->name);
		tabs++;
		listFunc(p->left);
		listFunc(p->right);
		tabs--;
	}
}

/* _removeFunc: used in removeFunc */
void _removeFunc(Func *p)
{
	if (p == NULL)
		return;

	_removeFunc(p->left);
	_removeFunc(p->right);

	p->left = NULL;
	p->right = NULL;
	strcpy(p->name, "");
	removeExpr(&(p->expr));
	for (int i = 0; i < MAXVAR; i++)
		removeVar(&(p->var[i]));
	free(p);
}

/* removeFunc: frees an expr and its branch below */
void removeFunc(Func **p)
{
	_removeFunc(*p);
	*p = NULL;
}

void testaddFunc(void)
{
	op_tree = loadOps(op_tree);
	Func *p = NULL;
	char *name = "f(x, y, z)";
	char *formula = "x^2 + x * y + y^2";

	printf("input: %s = %s\n", name, formula);
	p = addFunc(p, name, formula);
	printf("testaddFunc: listFunc\n");
	listFunc(p);

	printf("testaddFunc: listVar\n");
	for (int i = 0; i < MAXVAR; i++)
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
			return NULL;
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

	if (p == NULL)
		return NULL;

	p->left = _wireFuncVar2Formula(p->left, v);
	p->right = _wireFuncVar2Formula(p->right, v);

	if (strcmp(p->name, (*v)->name) == 0) {
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

	for (int i = 0; i < MAXVAR; i++) {
		v = p->var[i];
		while (v != NULL) {
			removeExpr(&(v->expr));
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
	bcutnstr(w, strlen(p) - 1);
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
	parseFuncName(func_name, name);		// with '('
	
	char symb_name[MAXCHAR] = "";
	strcpy(symb_name, func_name);
	bcutstr(symb_name);					// without '('
	
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
	printf("updateFunc\n");
	p = updateFunc(p, name);
	listFunc(p);

	printf("listVar\n");
	for (int i = 0; i < MAXVAR; i++)
		listVar(p->var[i]);

	printf("f->expr\n");
	printf("%s\n", p->expr->name);
}


#endif	// _FUNCTION_H
