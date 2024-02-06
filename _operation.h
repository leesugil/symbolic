#ifndef _OPERATION_H
#define _OPERATION_H

#include "qol/c/getword.h"
#include "expression.h"

// function signature
typedef double (*BinaryFunctionPointer)(double, double);
typedef void (*BinaryCharFunctionPointer)(char w[], char *x, char *y);
typedef void (*CommBinaryCharFunctionPointer)(char w[], char *x, char *y);
typedef void (*LeftAssocCharFunctionPointer)(char w[], char *a, char *b, char *c);
typedef void (*RightAssocCharFunctionPointer)(char w[], char *a, char *b, char *c);
typedef void (*LeftAssocByCharFunctionPointer)(char w[], char *a, char *b, char *c, char *op2);
typedef void (*RightAssocByCharFunctionPointer)(char w[], char *a, char *b, char *c, char *op2);
typedef void (*AssocCharFunctionPointer)(char [], char *, char *);
typedef void (*LeftDistOverCharFunctionPointer)(char w[], char *x, char *a, char *b, char *op2);
typedef void (*RightDistOverCharFunctionPointer)(char w[], char *x, char *a, char *b, char *op2);
typedef struct Op Op;

Op *op_tree = NULL;

extern char *block_start[];
extern char *block_end[];
extern void parseExprLeft(char [], char *, Op *, char **, char **);
extern void parseExprRight(char [], char *, Op *, char **, char **);

struct Op {
	/* assume binary */
	char *name;							// like " + "
	char *short_name;					// like "+"
	BinaryFunctionPointer f;			// like add(x, y)
	BinaryCharFunctionPointer char_f;	// like charAdd(w, x, y)
	BinaryCharFunctionPointer char_f_alt;	// like charSubtractAlt(w, x, y)
	Op *inverse;					// inverse operation
	char *left_dist_over[MAXCHAR];			// { " + ", NULL }
	char *left_dist_over_by[3][MAXCHAR];	// { { " + ", " * ", NULL }, { NULL, NULL, NULL} }
	LeftDistOverCharFunctionPointer left_dist_over_char_f;//
	char *right_dist_over[MAXCHAR];				// (a + b) * c = a * c + b * c
	char *right_dist_over_by[3][MAXCHAR];	// { { " + ", " * ", NULL }, { NULL, NULL, NULL} }
	RightDistOverCharFunctionPointer right_dist_over_char_f;//
	unsigned int is_dist : 1;
	LeftAssocCharFunctionPointer left_assoc;	// (a * b) * c
	RightAssocCharFunctionPointer right_assoc;	// a * (b * c)
	AssocCharFunctionPointer left2right_assoc;
	AssocCharFunctionPointer right2left_assoc;
	char *left2right_assoc_by[3][MAXCHAR];	// { { " ^ ", " * ", NULL }, { NULL, NULL, NULL} }
	RightAssocByCharFunctionPointer right_assoc_by;
	char *right2left_assoc_by[3][MAXCHAR];	// { { " ^ ", " * ", NULL }, { NULL, NULL, NULL} }
	LeftAssocByCharFunctionPointer left_assoc_by;
	unsigned int is_assoc : 1;
	CommBinaryCharFunctionPointer comm;	// f(a, b) = f(b, a)
	unsigned int is_comm : 1;
	char *right_unit;
	char *left_unit;
	char *unit;							// unit (identity element) captured in ASCII string
	Op *left;
	Op *right;
};

static Op *opAlloc(void)
{
	return (Op *) malloc(sizeof(Op));
}

Op *updateOp(Op *p, Op op);

Op *addOp(Op *p, Op op)
{
	int cond;

	if (p == NULL) {
		p = opAlloc();
		p->name = strdup(op.name);
		p->short_name = strdup(op.short_name);
		p->f = op.f;
		p->char_f = op.char_f;
		p->char_f_alt = op.char_f_alt;
		p->inverse = op.inverse;

		p->left = NULL;
		p->right = NULL;

		p = updateOp(p, op);
	} else if ((cond = strcmp(op.name, p->name)) == 0)
		p = updateOp(p, op);
	else if (cond < 0)
		p->left = addOp(p->left, op);
	else
		p->right = addOp(p->right, op);

	return p;
}

/* listOp: in-order print of tree p */
void listOp(Op *p)
{
	if (p != NULL) {
		listOp(p->left);
		printf("\"%s\"\n", p->name);
		listOp(p->right);
	}
}
void testlistOp(void)
{
	Op *root = NULL;

	//root = addOp(root, "f", "x + y");

	listOp(root);
}

/* getOp: returns a pointer to the Op found */
Op *getOp(Op *p, char *name)
{
	int cond;

	if (p == NULL) {
		return NULL;
	}
	else if ((cond = strcmp(name, p->name)) == 0) {
		return p;
	} else if (cond < 0) {
		return getOp(p->left, name);
	} else {
		return getOp(p->right, name);
	}
}

/* removeOp: frees a node and its branch below */
void _removeOp(Op *p)
{
	if (p != NULL) {
		_removeOp(p->left);
		_removeOp(p->right);
		p->left = NULL;
		p->right = NULL;
		free(p->name);
		free(p->short_name);
		free(p);
	}
}

void removeOp(Op **p)
{
	_removeOp(*p);
	*p = NULL;
}

Op *updateOp(Op *p, Op op)
{
	int i;
	for (i=0; op.left_dist_over[i] != NULL; i++)
		p->left_dist_over[i] = op.left_dist_over[i];
	p->left_dist_over[i] = NULL;
	for (i=0; op.right_dist_over[i] != NULL; i++)
		p->right_dist_over[i] = op.right_dist_over[i];
	p->right_dist_over[i] = NULL;
	p->is_dist = (p->left_dist_over[0] != NULL && p->right_dist_over[0] != NULL) ? 1 : 0;
	//left_dist_over_by
	int j;
	for (i=0; op.left_dist_over_by[i][0] != NULL; i++) {
		for (j=0; op.left_dist_over_by[i][j] != NULL; j++)
			p->left_dist_over_by[i][j] = op.left_dist_over_by[i][j];
		p->left_dist_over_by[i][j] = NULL;
	}
	p->left_dist_over_by[i][0] = NULL;
	p->left_dist_over_by[i][1] = NULL;
	p->left_dist_over_by[i][2] = NULL;
	p->left_dist_over_char_f = op.left_dist_over_char_f;
	//right_dist_over_by
	for (i=0; op.right_dist_over_by[i][0] != NULL; i++) {
		for (j=0; op.right_dist_over_by[i][j] != NULL; j++)
			p->right_dist_over_by[i][j] = op.right_dist_over_by[i][j];
		p->right_dist_over_by[i][j] = NULL;
	}
	p->right_dist_over_by[i][0] = NULL;
	p->right_dist_over_by[i][1] = NULL;
	p->right_dist_over_by[i][2] = NULL;
	p->right_dist_over_char_f = op.right_dist_over_char_f;

	p->left_assoc = op.left_assoc;
	p->right_assoc = op.right_assoc;
	p->left2right_assoc = op.left2right_assoc;
	p->right2left_assoc = op.right2left_assoc;
	p->is_assoc = (p->left2right_assoc != NULL && p->right2left_assoc != NULL) ? 1 : 0;
	//left2right_assoc_by
	for (i=0; op.left2right_assoc_by[i][0] != NULL; i++) {
		for (j=0; op.left2right_assoc_by[i][j] != NULL; j++)
			p->left2right_assoc_by[i][j] = op.left2right_assoc_by[i][j];
		p->left2right_assoc_by[i][j] = NULL;
	}
	p->left2right_assoc_by[i][0] = NULL;
	p->left2right_assoc_by[i][1] = NULL;
	p->left2right_assoc_by[i][2] = NULL;
	p->right_assoc_by = op.right_assoc_by;
	//right2left_assoc_by
	for (i=0; op.right2left_assoc_by[i][0] != NULL; i++) {
		for (j=0; op.right2left_assoc_by[i][j] != NULL; j++)
			p->right2left_assoc_by[i][j] = op.right2left_assoc_by[i][j];
		p->right2left_assoc_by[i][j] = NULL;
	}
	p->right2left_assoc_by[i][0] = NULL;
	p->right2left_assoc_by[i][1] = NULL;
	p->right2left_assoc_by[i][2] = NULL;
	p->left_assoc_by = op.left_assoc_by;

	p->comm = op.comm;
	p->is_comm = (p->comm != NULL) ? 1 : 0;
	p->right_unit = op.right_unit;
	p->left_unit = op.left_unit;
	p->unit = (p->right_unit != NULL && p->left_unit != NULL && strcmp(p->right_unit, p->left_unit) == 0) ? p->right_unit : NULL;

	return p;
}















/*
.name = "";
.short_name = "";
.f = NULL;
.char_f = NULL;
.char_f_alt = NULL;
.inverse = NULL;
.left_dist_over[0] = NULL;
.left_dist_over_by[0][0] = NULL;
.left_dist_over_by[0][1] = NULL;
.left_dist_over_by[0][2] = NULL;
.left_dist_over_char_f = NULL;
.right_dist_over[0] = NULL;
.right_dist_over_by[0][0] = NULL;
.right_dist_over_by[0][1] = NULL;
.right_dist_over_by[0][2] = NULL;
.right_dist_over_char_f = NULL;
.left_assoc = NULL;
.right_assoc = NULL;
.left2right_assoc = NULL;
.right2left_assoc = NULL;
.left2right_assoc_by[0][0] = NULL;
.left2right_assoc_by[0][1] = NULL;
.left2right_assoc_by[0][2] = NULL;
.right_assoc_by = NULL;
.right2left_assoc_by[0][0] = NULL;
.right2left_assoc_by[0][1] = NULL;
.right2left_assoc_by[0][2] = NULL;
.left_assoc_by = NULL;
.comm = NULL;
.right_unit = "";
.left_unit = "";
*/







/* character binary operation */
void binaryCharFunction(char w[], char *x, char *y, char *bop)
{
	if (x == NULL || y == NULL)
		return;
	if (strlen(x) * strlen(y) == 0)
		return;

	char x1[MAXCHAR] = "";
	char y1[MAXCHAR] = "";
	strcpy(x1, x);
	strcpy(y1, y);

	/* do not determine parentheses here
	while (is_outer_blocked_blk(x1, block_start, block_end, NULL))
		remove_outer_block_blk(x1, block_start, block_end);
	while (is_outer_blocked_blk(y1, block_start, block_end, NULL))
		remove_outer_block_blk(y1, block_start, block_end);

	parenthstr(x1);
	parenthstr(y1);
	*/
	strcpy(w, x1);
	strcat(w, bop);
	strcat(w, y1);
}
// x * y ==> y * x
void commBinaryCharFunction(char w[], char *x, char *y, char *op)
{
	sprintf(w, "%s%s%s", y, op, x);
}
// a * b * c ==> (a * b) * c
void leftAssocCharFunction(char w[], char *a, char *b, char *c, char *op)
{
	sprintf(w, "(%s%s%s)%s%s", a, op, b, op, c);
}
// a * b * c ==> a * (b * c)
void rightAssocCharFunction(char w[], char *a, char *b, char *c, char *op)
{
	sprintf(w, "%s%s(%s%s%s)", a, op, b, op, c);
}
// (a * b) * c ==> a * (b * c)
void left2rightAssocCharFunction(char w[], char *left, char *c, char *op)
{
	// checking proper blocks in left & removing outer blocks
	char dum_line[MAXCHAR] = "";
	strcpy(dum_line, left);
	int index = 0;
	if (!is_blocked_properly_blk(dum_line, block_start, block_end, &index))
		return;
	while (is_outer_blocked_blk(dum_line, block_start, block_end, &index)) {
		remove_outer_block(dum_line, block_start[index], block_end[index]);
	}

	// parse a and b
	char a[MAXCHAR] = "";
	char b[MAXCHAR] = "";
	Op *p = getOp(op_tree, op);
	parseExprLeft(a, dum_line, p, block_start, block_end);
	parseExprRight(b, dum_line, p, block_start, block_end);

	sprintf(w, "%s%s(%s%s%s)", a, op, b, op, c);
}
// a * (b * c) ==> (a * b) * c
void right2leftAssocCharFunction(char w[], char *a, char *right, char *op)
{
	// checking proper blocks in right & removing outer blocks
	char dum_line[MAXCHAR] = "";
	strcpy(dum_line, right);
	int index = 0;
	if (!is_blocked_properly_blk(dum_line, block_start, block_end, &index)) {
		return;
	}
	while (is_outer_blocked_blk(dum_line, block_start, block_end, &index)) {
		remove_outer_block(dum_line, block_start[index], block_end[index]);
	}

	// parse a and b
	char b[MAXCHAR] = "";
	char c[MAXCHAR] = "";
	Op *p = getOp(op_tree, op);
	parseExprLeft(b, dum_line, p, block_start, block_end);
	parseExprRight(c, dum_line, p, block_start, block_end);

	sprintf(w, "(%s%s%s)%s%s", a, op, b, op, c);
}
// (a op2 b) op c
void LeftAssocByCharFunction(char w[], char *a, char *b, char *c, char *op, char *op2)
{
	sprintf(w, "(%s%s%s)%s%s", a, op2, b, op, c);
}
// a op (b op2 c)
void RightAssocByCharFunction(char w[], char *a, char *b, char *c, char *op, char *op2)
{
	sprintf(w, "%s%s(%s%s%s)", a, op, b, op2, c);
}
// (x op a) op2 (x op b)
void LeftDistOverCharFunction(char w[], char *x, char *a, char *b, char *op, char *op2)
{
	sprintf(w, "(%s%s%s)%s(%s%s%s)", x, op, a, op2, x, op, b);
}
// (a op x) op2 (b op x)
void RightDistOverCharFunction(char w[], char *x, char *a, char *b, char *op, char *op2)
{
	sprintf(w, "(%s%s%s)%s(%s%s%s)", a, op, x, op2, b, op, x);
}








/***************************
 * addition and subtrction
 * *************************/
double add(double x, double y)
{
	return x + y;
}
void charAdd(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " + ");
}
void charAddComm(char w[], char *x, char *y)
{
	commBinaryCharFunction(w, x, y, " + ");
}
void charAddLeftAssoc(char w[], char *a, char *b, char *c)
{
	leftAssocCharFunction(w, a, b, c, " + ");
}
void charAddRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " + ");
}
void left2rightAssocAdd(char w[], char *left, char *c)
{
	left2rightAssocCharFunction(w, left, c, " + ");
}
void right2leftAssocAdd(char w[], char *a, char *right)
{
	right2leftAssocCharFunction(w, a, right, " + ");
}
// (a op2 b) op c
void leftAssocByAdd(char w[], char *a, char *b, char *c, char *op2)
{
	// this is a sample function
	LeftAssocByCharFunction(w, a, b, c, " + ", op2);
}
// a op (b op2 c)
void rightAssocByAdd(char w[], char *a, char *b, char *c, char *op2)
{
	// this is a sample function
	RightAssocByCharFunction(w, a, b, c, " + ", op2);
}
// (x op a) op2 (x op b)
void leftDistOverAdd(char w[], char *x, char *a, char *b, char *op2)
{
	// this is a sample function
	LeftDistOverCharFunction(w, x, a, b, " + ", op2);
}
// (a op x) op2 (b op x)
void rightDistOverCharFunction(char w[], char *x, char *a, char *b, char *op2)
{
	// this is a sample function
	RightDistOverCharFunction(w, x, a, b, " + ", op2);
}

double subtract(double x, double y)
{
	return x - y;
}
void charSubtract(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " - ");
}
void charSubtractAlt(char w[], char *x, char *y)
{
	char yp[MAXCHAR] = "";
	strcpy(yp, "-1 * ");
	strcat(yp, y);

	binaryCharFunction(w, x, yp, " + ");
}
void charSubtractLeftAssoc(char w[], char *a, char *b, char *c)
{
	leftAssocCharFunction(w, a, b, c, " - ");
}




/***************************
 * multiplication and division
 * *************************/
double multiply(double x, double y)
{
	return x * y;
}
void charMultiply(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " * ");
}
void charMultiplyComm(char w[], char *x, char *y)
{
	commBinaryCharFunction(w, x, y, " * ");
}
void charMultiplyLeftAssoc(char w[], char *a, char *b, char *c)
{
	leftAssocCharFunction(w, a, b, c, " * ");
}
void charMultiplyRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " * ");
}
void left2rightAssocMultiply(char w[], char *left, char *c)
{
	left2rightAssocCharFunction(w, left, c, " * ");
}
void right2leftAssocMultiply(char w[], char *a, char *right)
{
	right2leftAssocCharFunction(w, a, right, " * ");
}

double divide(double x, double y)
{
	return x / y;
}
void charDivide(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " / ");
}
void charDivideAlt(char w[], char *x, char *y)
{
	char yp[MAXCHAR] = "";
	strcpy(yp, y);
	strcat(yp, "^-1");

	binaryCharFunction(w, x, yp, " * ");
}
void charDivideLeftAssoc(char w[], char *a, char *b, char *c)
{
	leftAssocCharFunction(w, a, b, c, " / ");
}

/***************************
 * exponentication
 * *************************/
double exponen(double x, double y)
{
	return pow(x, y);
}
void charExponen(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, "^");
}
void charExponenRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, "^");
}
// (x op a) op2 (x op b)
void leftDistOverExponen(char w[], char *x, char *a, char *b, char *op2)
{
	LeftDistOverCharFunction(w, x, a, b, "^", op2);
}
// a op (b op2 c)
void rightAssocByExponen(char w[], char *a, char *b, char *c, char *op2)
{
	RightAssocByCharFunction(w, a, b, c, "^", op2);
}

/***************************
 * modulo arithmetic
 * *************************/
double mod(double x, double y)
{
	return ((double) (((int) x) % ((int) y)));
}
void charMod(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " % ");
}
void charModLeftAssoc(char w[], char *a, char *b, char *c)
{
	leftAssocCharFunction(w, a, b, c, " % ");
}

/***************************
 * ", " and " = "
 * *************************/
void charComma(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, ", ");
}
void charCommaComm(char w[], char *x, char *y)
{
	commBinaryCharFunction(w, x, y, ", ");
}
void charCommaRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, ", ");
}

void charLet(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " = ");
}
void charLetRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " = ");
}

/***************************
 * " ==  ", " < ", " <= ", " > ", " >= ", " != "
 * *************************/
void charEqual(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " == ");
}
void charEqualRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " == ");
}

void charLess(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " < ");
}
void charLessRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " < ");
}

void charLeq(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " <= ");
}
void charLeqRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " <= ");
}

void charGreater(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " > ");
}
void charGreaterRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " > ");
}

void charGeq(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " >= ");
}
void charGeqRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " >= ");
}

void charNeq(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " != ");
}
void charNeqRightAssoc(char w[], char *a, char *b, char *c)
{
	rightAssocCharFunction(w, a, b, c, " != ");
}










/* run this at the beginning of the main code to load up the operation tree */
Op *loadOps(Op *p)
{
	/***************************
	 * addition and subtrction
	 * *************************/
	Op addition;
	Op subtraction;

	addition.name = " + ";
	addition.short_name = "+";
	addition.f = add;
	addition.char_f = charAdd;
	addition.char_f_alt = NULL;
	addition.inverse = &subtraction;
	addition.left_dist_over[0] = NULL;
	addition.left_dist_over_by[0][0] = NULL;
	addition.left_dist_over_by[0][1] = NULL;
	addition.left_dist_over_by[0][2] = NULL;
	addition.left_dist_over_char_f = NULL;
	addition.right_dist_over[0] = NULL;
	addition.right_dist_over_by[0][0] = NULL;
	addition.right_dist_over_by[0][1] = NULL;
	addition.right_dist_over_by[0][2] = NULL;
	addition.right_dist_over_char_f = NULL;
	addition.left_assoc = charAddLeftAssoc;
	addition.right_assoc = charAddRightAssoc;
	addition.left2right_assoc = left2rightAssocAdd;
	addition.right2left_assoc = right2leftAssocAdd;
	addition.left2right_assoc_by[0][0] = NULL;
	addition.left2right_assoc_by[0][1] = NULL;
	addition.left2right_assoc_by[0][2] = NULL;
	addition.right_assoc_by = NULL;
	addition.right2left_assoc_by[0][0] = NULL;
	addition.right2left_assoc_by[0][1] = NULL;
	addition.right2left_assoc_by[0][2] = NULL;
	addition.left_assoc_by = NULL;
	addition.comm = charAddComm;
	addition.right_unit = "0";
	addition.left_unit = "0";

	subtraction.name = " - ";
	subtraction.short_name = "-";
	subtraction.f = subtract;
	subtraction.char_f = charSubtract;
	subtraction.char_f_alt = charSubtractAlt;
	subtraction.inverse = &addition;
	subtraction.left_dist_over[0] = NULL;
	subtraction.left_dist_over_by[0][0] = NULL;
	subtraction.left_dist_over_by[0][1] = NULL;
	subtraction.left_dist_over_by[0][2] = NULL;
	subtraction.left_dist_over_char_f = NULL;
	subtraction.right_dist_over[0] = NULL;
	subtraction.right_dist_over_by[0][0] = NULL;
	subtraction.right_dist_over_by[0][1] = NULL;
	subtraction.right_dist_over_by[0][2] = NULL;
	subtraction.right_dist_over_char_f = NULL;
	subtraction.left_assoc = charSubtractLeftAssoc;
	subtraction.right_assoc = NULL;
	subtraction.left2right_assoc = NULL;
	subtraction.right2left_assoc = NULL;
	subtraction.left2right_assoc_by[0][0] = NULL;
	subtraction.left2right_assoc_by[0][1] = NULL;
	subtraction.left2right_assoc_by[0][2] = NULL;
	subtraction.right_assoc_by = NULL;
	subtraction.right2left_assoc_by[0][0] = NULL;
	subtraction.right2left_assoc_by[0][1] = NULL;
	subtraction.right2left_assoc_by[0][2] = NULL;
	subtraction.left_assoc_by = NULL;
	subtraction.comm = NULL;
	subtraction.right_unit = "0";
	subtraction.left_unit = NULL;

	p = addOp(p, addition);
	p = addOp(p, subtraction);


	/***************************
	 * multiplication and division
	 * *************************/
	Op multiplication;
	Op division;

	multiplication.name = " * ";
	multiplication.short_name = "*";
	multiplication.f = multiply;
	multiplication.char_f = charMultiply;
	multiplication.char_f_alt = NULL;
	multiplication.inverse = &division;
	multiplication.left_dist_over[0] = " + ";
	multiplication.left_dist_over[1] = " - ";
	multiplication.left_dist_over[2] = NULL;
	multiplication.left_dist_over_by[0][0] = NULL;
	multiplication.left_dist_over_by[0][1] = NULL;
	multiplication.left_dist_over_by[0][2] = NULL;
	multiplication.left_dist_over_char_f = NULL;
	multiplication.right_dist_over[0] = " + ";
	multiplication.right_dist_over[1] = " - ";
	multiplication.right_dist_over[2] = NULL;
	multiplication.right_dist_over_by[0][0] = NULL;
	multiplication.right_dist_over_by[0][1] = NULL;
	multiplication.right_dist_over_by[0][2] = NULL;
	multiplication.right_dist_over_char_f = NULL;
	multiplication.left_assoc = charMultiplyLeftAssoc;
	multiplication.right_assoc = charMultiplyRightAssoc;
	multiplication.left2right_assoc = left2rightAssocMultiply;
	multiplication.right2left_assoc = right2leftAssocMultiply;
	multiplication.left2right_assoc_by[0][0] = NULL;
	multiplication.left2right_assoc_by[0][1] = NULL;
	multiplication.left2right_assoc_by[0][2] = NULL;
	multiplication.right_assoc_by = NULL;
	multiplication.right2left_assoc_by[0][0] = NULL;
	multiplication.right2left_assoc_by[0][1] = NULL;
	multiplication.right2left_assoc_by[0][2] = NULL;
	multiplication.left_assoc_by = NULL;
	multiplication.comm = charMultiplyComm;
	multiplication.right_unit = "1";
	multiplication.left_unit = "1";

	division.name = " / ";
	division.short_name = "/";
	division.f = divide;
	division.char_f = charDivide;
	division.char_f_alt = charDivideAlt;
	division.inverse = &multiplication;
	division.left_dist_over[0] = NULL;		// a / (b + c) != a / b + a / c
	division.left_dist_over_by[0][0] = NULL;
	division.left_dist_over_by[0][1] = NULL;
	division.left_dist_over_by[0][2] = NULL;
	division.left_dist_over_char_f = NULL;
	division.right_dist_over[0] = " + ";
	division.right_dist_over[1] = " - ";
	division.right_dist_over[2] = NULL;
	division.right_dist_over_by[0][0] = NULL;
	division.right_dist_over_by[0][1] = NULL;
	division.right_dist_over_by[0][2] = NULL;
	division.right_dist_over_char_f = NULL;
	division.left_assoc = charDivideLeftAssoc;
	division.right_assoc = NULL;
	division.left2right_assoc = NULL;
	division.right2left_assoc = NULL;
	division.left2right_assoc_by[0][0] = NULL;
	division.left2right_assoc_by[0][1] = NULL;
	division.left2right_assoc_by[0][2] = NULL;
	division.right_assoc_by = NULL;
	division.right2left_assoc_by[0][0] = NULL;
	division.right2left_assoc_by[0][1] = NULL;
	division.right2left_assoc_by[0][2] = NULL;
	division.left_assoc_by = NULL;
	division.comm = NULL;
	division.right_unit = "1";
	division.left_unit = NULL;

	p = addOp(p, multiplication);
	p = addOp(p, division);


	/***************************
	 * exponentiation
	 * *************************/
	Op exponentiation;

	exponentiation.name = "^";
	exponentiation.short_name = "^";
	exponentiation.f = exponen;
	exponentiation.char_f = charExponen;
	exponentiation.char_f_alt = NULL;
	exponentiation.inverse = NULL;
	exponentiation.left_dist_over[0] = NULL;
	exponentiation.left_dist_over_by[0][0] = " + ";
	exponentiation.left_dist_over_by[0][1] = " * ";
	exponentiation.left_dist_over_by[0][2] = NULL;
	exponentiation.left_dist_over_by[1][0] = NULL;
	exponentiation.left_dist_over_by[1][1] = NULL;
	exponentiation.left_dist_over_by[1][2] = NULL;
	exponentiation.left_dist_over_char_f = leftDistOverExponen;
	exponentiation.right_dist_over[0] = " * ";
	exponentiation.right_dist_over[1] = " / ";
	exponentiation.right_dist_over[2] = NULL;
	exponentiation.right_dist_over_by[0][0] = NULL;
	exponentiation.right_dist_over_by[0][1] = NULL;
	exponentiation.right_dist_over_by[0][2] = NULL;
	exponentiation.right_dist_over_char_f = NULL;
	exponentiation.left_assoc = NULL;
	exponentiation.right_assoc = charExponenRightAssoc;
	exponentiation.left2right_assoc = NULL;
	exponentiation.right2left_assoc = NULL;
	exponentiation.left2right_assoc_by[0][0] = "^";
	exponentiation.left2right_assoc_by[0][1] = " * ";
	exponentiation.left2right_assoc_by[0][2] = NULL;
	exponentiation.left2right_assoc_by[1][0] = NULL;
	exponentiation.left2right_assoc_by[1][1] = NULL;
	exponentiation.left2right_assoc_by[1][2] = NULL;
	exponentiation.right_assoc_by = rightAssocByExponen;
	exponentiation.right2left_assoc_by[0][0] = NULL;
	exponentiation.right2left_assoc_by[0][1] = NULL;
	exponentiation.right2left_assoc_by[0][2] = NULL;
	exponentiation.left_assoc_by = NULL;
	exponentiation.comm = NULL;
	exponentiation.right_unit = "1";
	exponentiation.left_unit = NULL;

	p = addOp(p, exponentiation);

	/***************************
	 * modulo arithmetic
	 * *************************/
	Op modulo;

	modulo.name = " % ";
	modulo.short_name = "%";
	modulo.f = mod;
	modulo.char_f = charMod;
	modulo.char_f_alt = NULL;
	modulo.inverse = NULL;
	modulo.left_dist_over[0] = NULL;
	modulo.left_dist_over_by[0][0] = NULL;
	modulo.left_dist_over_by[0][1] = NULL;
	modulo.left_dist_over_by[0][2] = NULL;
	modulo.left_dist_over_char_f = NULL;
	modulo.right_dist_over[0] = NULL;
	modulo.right_dist_over_by[0][0] = NULL;
	modulo.right_dist_over_by[0][1] = NULL;
	modulo.right_dist_over_by[0][2] = NULL;
	modulo.right_dist_over_char_f = NULL;
	modulo.left_assoc = charModLeftAssoc;
	modulo.right_assoc = NULL;
	modulo.left2right_assoc = NULL;
	modulo.right2left_assoc = NULL;
	modulo.left2right_assoc_by[0][0] = NULL;
	modulo.left2right_assoc_by[0][1] = NULL;
	modulo.left2right_assoc_by[0][2] = NULL;
	modulo.right_assoc_by = NULL;
	modulo.right2left_assoc_by[0][0] = NULL;
	modulo.right2left_assoc_by[0][1] = NULL;
	modulo.right2left_assoc_by[0][2] = NULL;
	modulo.left_assoc_by = NULL;
	modulo.comm = NULL;
	modulo.right_unit = NULL;
	modulo.left_unit = NULL;

	p = addOp(p, modulo);


	/***************************
	 * ", " and " = "
	 * *************************/
	Op comma;
	Op let;

	comma.name = ", ";
	comma.short_name = ",";
	comma.f = NULL;
	comma.char_f = charComma;
	comma.char_f_alt = NULL;
	comma.inverse = NULL;
	comma.left_dist_over[0] = NULL;
	comma.left_dist_over_by[0][0] = NULL;
	comma.left_dist_over_by[0][1] = NULL;
	comma.left_dist_over_by[0][2] = NULL;
	comma.left_dist_over_char_f = NULL;
	comma.right_dist_over[0] = NULL;
	comma.right_dist_over_by[0][0] = NULL;
	comma.right_dist_over_by[0][1] = NULL;
	comma.right_dist_over_by[0][2] = NULL;
	comma.right_dist_over_char_f = NULL;
	comma.left_assoc = NULL;
	comma.right_assoc = charCommaRightAssoc;
	comma.left2right_assoc = NULL;
	comma.right2left_assoc = NULL;
	comma.left2right_assoc_by[0][0] = NULL;
	comma.left2right_assoc_by[0][1] = NULL;
	comma.left2right_assoc_by[0][2] = NULL;
	comma.right_assoc_by = NULL;
	comma.right2left_assoc_by[0][0] = NULL;
	comma.right2left_assoc_by[0][1] = NULL;
	comma.right2left_assoc_by[0][2] = NULL;
	comma.left_assoc_by = NULL;
	comma.comm = charCommaComm;
	comma.right_unit = NULL;
	comma.left_unit = NULL;

	let.name = " = ";
	let.short_name = "=";
	let.f = NULL;
	let.char_f = charLet;
	let.char_f_alt = NULL;
	let.inverse = NULL;
	let.left_dist_over[0] = NULL;
	let.left_dist_over_by[0][0] = NULL;
	let.left_dist_over_by[0][1] = NULL;
	let.left_dist_over_by[0][2] = NULL;
	let.left_dist_over_char_f = NULL;
	let.right_dist_over[0] = NULL;
	let.right_dist_over_by[0][0] = NULL;
	let.right_dist_over_by[0][1] = NULL;
	let.right_dist_over_by[0][2] = NULL;
	let.right_dist_over_char_f = NULL;
	let.left_assoc = NULL;
	let.right_assoc = charEqualRightAssoc;
	let.left2right_assoc = NULL;
	let.right2left_assoc = NULL;
	let.left2right_assoc_by[0][0] = NULL;
	let.left2right_assoc_by[0][1] = NULL;
	let.left2right_assoc_by[0][2] = NULL;
	let.right_assoc_by = NULL;
	let.right2left_assoc_by[0][0] = NULL;
	let.right2left_assoc_by[0][1] = NULL;
	let.right2left_assoc_by[0][2] = NULL;
	let.left_assoc_by = NULL;
	let.comm = NULL;
	let.right_unit = NULL;
	let.left_unit = NULL;

	p = addOp(p, comma);
	p = addOp(p, let);


	/***************************
	 * " ==  ", " < ", " <= ", " > ", " >= ", " != "
	 * *************************/
	Op equal;
	Op less;
	Op leq;
	Op greater;
	Op geq;
	Op neq;

	equal.name = " == ";
	equal.short_name = "==";
	equal.f = NULL;
	equal.char_f = charEqual;
	equal.char_f_alt = NULL;
	equal.inverse = NULL;
	equal.left_dist_over[0] = NULL;
	equal.left_dist_over_by[0][0] = NULL;
	equal.left_dist_over_by[0][1] = NULL;
	equal.left_dist_over_by[0][2] = NULL;
	equal.left_dist_over_char_f = NULL;
	equal.right_dist_over[0] = NULL;
	equal.right_dist_over_by[0][0] = NULL;
	equal.right_dist_over_by[0][1] = NULL;
	equal.right_dist_over_by[0][2] = NULL;
	equal.right_dist_over_char_f = NULL;
	equal.left_assoc = NULL;
	equal.right_assoc = charEqualRightAssoc;
	equal.left2right_assoc = NULL;
	equal.right2left_assoc = NULL;
	equal.left2right_assoc_by[0][0] = NULL;
	equal.left2right_assoc_by[0][1] = NULL;
	equal.left2right_assoc_by[0][2] = NULL;
	equal.right_assoc_by = NULL;
	equal.right2left_assoc_by[0][0] = NULL;
	equal.right2left_assoc_by[0][1] = NULL;
	equal.right2left_assoc_by[0][2] = NULL;
	equal.left_assoc_by = NULL;
	equal.comm = NULL;
	equal.right_unit = NULL;
	equal.left_unit = NULL;

	less.name = " < ";
	less.short_name = "<";
	less.f = NULL;
	less.char_f = charLess;
	less.char_f_alt = NULL;
	less.inverse = NULL;
	less.left_dist_over[0] = NULL;
	less.left_dist_over_by[0][0] = NULL;
	less.left_dist_over_by[0][1] = NULL;
	less.left_dist_over_by[0][2] = NULL;
	less.left_dist_over_char_f = NULL;
	less.right_dist_over[0] = NULL;
	less.right_dist_over_by[0][0] = NULL;
	less.right_dist_over_by[0][1] = NULL;
	less.right_dist_over_by[0][2] = NULL;
	less.right_dist_over_char_f = NULL;
	less.left_assoc = NULL;
	less.right_assoc = charLessRightAssoc;
	less.left2right_assoc = NULL;
	less.right2left_assoc = NULL;
	less.left2right_assoc_by[0][0] = NULL;
	less.left2right_assoc_by[0][1] = NULL;
	less.left2right_assoc_by[0][2] = NULL;
	less.right_assoc_by = NULL;
	less.right2left_assoc_by[0][0] = NULL;
	less.right2left_assoc_by[0][1] = NULL;
	less.right2left_assoc_by[0][2] = NULL;
	less.left_assoc_by = NULL;
	less.comm = NULL;
	less.right_unit = NULL;
	less.left_unit = NULL;

	leq.name = " <= ";
	leq.short_name = "<=";
	leq.f = NULL;
	leq.char_f = charLeq;
	leq.char_f_alt = NULL;
	leq.inverse = NULL;
	leq.left_dist_over[0] = NULL;
	leq.left_dist_over_by[0][0] = NULL;
	leq.left_dist_over_by[0][1] = NULL;
	leq.left_dist_over_by[0][2] = NULL;
	leq.left_dist_over_char_f = NULL;
	leq.right_dist_over[0] = NULL;
	leq.right_dist_over_by[0][0] = NULL;
	leq.right_dist_over_by[0][1] = NULL;
	leq.right_dist_over_by[0][2] = NULL;
	leq.right_dist_over_char_f = NULL;
	leq.left_assoc = NULL;
	leq.right_assoc = charLeqRightAssoc;
	leq.left2right_assoc = NULL;
	leq.right2left_assoc = NULL;
	leq.left2right_assoc_by[0][0] = NULL;
	leq.left2right_assoc_by[0][1] = NULL;
	leq.left2right_assoc_by[0][2] = NULL;
	leq.right_assoc_by = NULL;
	leq.right2left_assoc_by[0][0] = NULL;
	leq.right2left_assoc_by[0][1] = NULL;
	leq.right2left_assoc_by[0][2] = NULL;
	leq.left_assoc_by = NULL;
	leq.comm = NULL;
	leq.right_unit = NULL;
	leq.left_unit = NULL;

	greater.name = " > ";
	greater.short_name = ">";
	greater.f = NULL;
	greater.char_f = charGreater;
	greater.char_f_alt = NULL;
	greater.inverse = NULL;
	greater.left_dist_over[0] = NULL;
	greater.left_dist_over_by[0][0] = NULL;
	greater.left_dist_over_by[0][1] = NULL;
	greater.left_dist_over_by[0][2] = NULL;
	greater.left_dist_over_char_f = NULL;
	greater.right_dist_over[0] = NULL;
	greater.right_dist_over_by[0][0] = NULL;
	greater.right_dist_over_by[0][1] = NULL;
	greater.right_dist_over_by[0][2] = NULL;
	greater.right_dist_over_char_f = NULL;
	greater.left_assoc = NULL;
	greater.right_assoc = charGreaterRightAssoc;
	greater.left2right_assoc = NULL;
	greater.right2left_assoc = NULL;
	greater.left2right_assoc_by[0][0] = NULL;
	greater.left2right_assoc_by[0][1] = NULL;
	greater.left2right_assoc_by[0][2] = NULL;
	greater.right_assoc_by = NULL;
	greater.right2left_assoc_by[0][0] = NULL;
	greater.right2left_assoc_by[0][1] = NULL;
	greater.right2left_assoc_by[0][2] = NULL;
	greater.left_assoc_by = NULL;
	greater.comm = NULL;
	greater.right_unit = NULL;
	greater.left_unit = NULL;

	geq.name = " >= ";
	geq.short_name = ">=";
	geq.f = NULL;
	geq.char_f = charGeq;
	geq.char_f_alt = NULL;
	geq.inverse = NULL;
	geq.left_dist_over[0] = NULL;
	geq.left_dist_over_by[0][0] = NULL;
	geq.left_dist_over_by[0][1] = NULL;
	geq.left_dist_over_by[0][2] = NULL;
	geq.left_dist_over_char_f = NULL;
	geq.right_dist_over[0] = NULL;
	geq.right_dist_over_by[0][0] = NULL;
	geq.right_dist_over_by[0][1] = NULL;
	geq.right_dist_over_by[0][2] = NULL;
	geq.right_dist_over_char_f = NULL;
	geq.left_assoc = NULL;
	geq.right_assoc = charGeqRightAssoc;
	geq.left2right_assoc = NULL;
	geq.right2left_assoc = NULL;
	geq.left2right_assoc_by[0][0] = NULL;
	geq.left2right_assoc_by[0][1] = NULL;
	geq.left2right_assoc_by[0][2] = NULL;
	geq.right_assoc_by = NULL;
	geq.right2left_assoc_by[0][0] = NULL;
	geq.right2left_assoc_by[0][1] = NULL;
	geq.right2left_assoc_by[0][2] = NULL;
	geq.left_assoc_by = NULL;
	geq.comm = NULL;
	geq.right_unit = NULL;
	geq.left_unit = NULL;

	neq.name = " != ";
	neq.short_name = "!=";
	neq.f = NULL;
	neq.char_f = charNeq;
	neq.char_f_alt = NULL;
	neq.inverse = NULL;
	neq.left_dist_over[0] = NULL;
	neq.left_dist_over_by[0][0] = NULL;
	neq.left_dist_over_by[0][1] = NULL;
	neq.left_dist_over_by[0][2] = NULL;
	neq.left_dist_over_char_f = NULL;
	neq.right_dist_over[0] = NULL;
	neq.right_dist_over_by[0][0] = NULL;
	neq.right_dist_over_by[0][1] = NULL;
	neq.right_dist_over_by[0][2] = NULL;
	neq.right_dist_over_char_f = NULL;
	neq.left_assoc = NULL;
	neq.right_assoc = charNeqRightAssoc;
	neq.left2right_assoc = NULL;
	neq.right2left_assoc = NULL;
	neq.left2right_assoc_by[0][0] = NULL;
	neq.left2right_assoc_by[0][1] = NULL;
	neq.left2right_assoc_by[0][2] = NULL;
	neq.right_assoc_by = NULL;
	neq.right2left_assoc_by[0][0] = NULL;
	neq.right2left_assoc_by[0][1] = NULL;
	neq.right2left_assoc_by[0][2] = NULL;
	neq.left_assoc_by = NULL;
	neq.comm = NULL;
	neq.right_unit = NULL;
	neq.left_unit = NULL;

	p = addOp(p, equal);
	p = addOp(p, less);
	p = addOp(p, leq);
	p = addOp(p, greater);
	p = addOp(p, geq);
	p = addOp(p, neq);




	
	return p;
}
void testloadOps(void)
{
	Op *p = NULL;
	
	p = loadOps(p);

	listOp(p);

	char w[MAXCHAR] = "";
	char *a = "a";
	char *b = "b";
	char *c = "c";
	Op *q = NULL;

	q = getOp(p, " + ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->left_assoc(w, a, b, c);
	printf("%s\n", w);
	q->right_assoc(w, a, b, c);
	printf("%s\n", w);
	q->left2right_assoc(w, "(a + b)", c);
	printf("%s\n", w);
	q->right2left_assoc(w, a, "(b + c)");
	printf("%s\n", w);

	q = getOp(p, " - ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->left_assoc(w, a, b, c);
	printf("%s\n", w);

	q = getOp(p, " * ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->left_assoc(w, a, b, c);
	printf("%s\n", w);
	q->right_assoc(w, a, b, c);
	printf("%s\n", w);
	q->left2right_assoc(w, "(a * b)", c);
	printf("%s\n", w);
	q->right2left_assoc(w, a, "(b * c)");
	printf("%s\n", w);

	q = getOp(p, " / ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->left_assoc(w, a, b, c);
	printf("%s\n", w);
}








#endif	/* _OPERATION_H */
