#ifndef _OPERATION_H
#define _OPERATION_H

#include "qol/c/getword.h"
#include "expression.h"

extern char *block_start[];
extern char *block_end[];
extern void parseExprLeftBy(char [], char *, char **, char **, char **);
extern void parseExprRightBy(char [], char *, char **, char **, char **);

// function signature
typedef double (*BinaryFunctionPointer)(double, double);
typedef void (*BinaryCharFunctionPointer)(char w[], char *x, char *y);
typedef void (*CommBinaryCharFunctionPointer)(char w[], char *x, char *y);
typedef void (*LeftAssocCharFunctionPointer)(char w[], char *a, char *b, char *c);
typedef void (*RightAssocCharFunctionPointer)(char w[], char *a, char *b, char *c);
typedef void (*AssocCharFunctionPointer)(char [], char *, char *);

typedef struct Op Op;

struct Op {
	/* assume binary */
	char *name;							// like " + "
	char *short_name;					// like "+"
	BinaryFunctionPointer f;			// like add(x, y)
	BinaryCharFunctionPointer char_f;	// like charadd(w, x, y)
	Op *inverse;					// inverse operation
	char *left_dist_over[MAXCHAR];				// { " + ", NULL }
	char *right_dist_over[MAXCHAR];				// (a + b) * c = a * c + b * c
	unsigned int is_dist : 1;
	LeftAssocCharFunctionPointer char_left_assoc_f;	// (a * b) * c
	RightAssocCharFunctionPointer char_right_assoc_f;	// a * (b * c)
	AssocCharFunctionPointer char_left2right_assoc_f;
	AssocCharFunctionPointer char_right2left_assoc_f;
	unsigned int is_assoc : 1;
	CommBinaryCharFunctionPointer char_comm_f;	// f(a, b) = f(b, a)
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

Op *addOp(Op *p, Op op)
{
	int cond;

	if (p == NULL) {
		p = opAlloc();
		p->name = op.name;
		p->short_name = op.short_name;
		p->f = op.f;
		p->char_f = op.char_f;
		p->inverse = op.inverse;

		int i;
		for (i=0; op.left_dist_over[i] != NULL; i++)
			p->left_dist_over[i] = op.left_dist_over[i];
		p->left_dist_over[i] = NULL;
		for (i=0; op.right_dist_over[i] != NULL; i++)
			p->right_dist_over[i] = op.right_dist_over[i];
		p->right_dist_over[i] = NULL;

		p->is_dist = (p->left_dist_over[0] != NULL && p->right_dist_over[0] != NULL) ? 1 : 0;
		p->char_left_assoc_f = op.char_left_assoc_f;
		p->char_right_assoc_f = op.char_right_assoc_f;
		p->char_left2right_assoc_f = op.char_left2right_assoc_f;
		p->char_right2left_assoc_f = op.char_right2left_assoc_f;
		p->is_assoc = (p->char_left2right_assoc_f != NULL && p->char_right2left_assoc_f != NULL) ? 1 : 0;
		p->char_comm_f = op.char_comm_f;
		p->is_comm = (p->char_comm_f != NULL) ? 1 : 0;
		p->right_unit = op.right_unit;
		p->left_unit = op.left_unit;
		p->unit = (strlen(p->right_unit) > 0 && strcmp(p->right_unit, p->left_unit) == 0) ? p->right_unit : "";

		p->left = NULL;
		p->right = NULL;
	} else if ((cond = strcmp(op.name, p->name)) == 0)
		;
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
		free(p);
	}
}

void removeOp(Op **p)
{
	_removeOp(*p);
	*p = NULL;
}












/*
.name = "";
.short_name = "";
.f = NULL;
.char_f = NULL;
.inverse = NULL;
.left_dist_over[0] = NULL;
.right_dist_over[0] = NULL;
.char_left_assoc_f = NULL;
.char_right_assoc_f = NULL;
.char_left2right_assoc_f = NULL;
.char_right2left_assoc_f = NULL;
.char_comm_f = NULL;
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

	while (is_outer_blocked_blk(x1, block_start, block_end, NULL))
		remove_outer_block_blk(x1, block_start, block_end);
	while (is_outer_blocked_blk(y1, block_start, block_end, NULL))
		remove_outer_block_blk(y1, block_start, block_end);

	parenthstr(x1);
	parenthstr(y1);
	w[0] = '\0';
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
	char *op_list[] = { op, NULL };
	parseExprLeftBy(a, dum_line, op_list, block_start, block_end);
	parseExprRightBy(b, dum_line, op_list, block_start, block_end);

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
	char *op_list[] = { op, NULL };
	parseExprLeftBy(b, dum_line, op_list, block_start, block_end);
	parseExprRightBy(c, dum_line, op_list, block_start, block_end);

	sprintf(w, "(%s%s%s)%s%s", a, op, b, op, c);
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

double subtract(double x, double y)
{
	return x - y;
}
void charSubtract(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " - ");
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
void charDivideLeftAssoc(char w[], char *a, char *b, char *c)
{
	leftAssocCharFunction(w, a, b, c, " / ");
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
	addition.inverse = &subtraction;
	addition.left_dist_over[0] = NULL;
	addition.right_dist_over[0] = NULL;
	addition.char_left_assoc_f = charAddLeftAssoc;
	addition.char_right_assoc_f = charAddRightAssoc;
	addition.char_left2right_assoc_f = left2rightAssocAdd;
	addition.char_right2left_assoc_f = right2leftAssocAdd;
	addition.char_comm_f = charAddComm;
	addition.right_unit = "0";
	addition.left_unit = "0";

	subtraction.name = " - ";
	subtraction.short_name = "-";
	subtraction.f = subtract;
	subtraction.char_f = charSubtract;
	subtraction.inverse = &addition;
	subtraction.left_dist_over[0] = NULL;
	subtraction.right_dist_over[0] = NULL;
	subtraction.char_left_assoc_f = charSubtractLeftAssoc;
	subtraction.char_right_assoc_f = NULL;
	subtraction.char_left2right_assoc_f = NULL;
	subtraction.char_right2left_assoc_f = NULL;
	subtraction.char_comm_f = NULL;
	subtraction.right_unit = "0";
	subtraction.left_unit = "";


	/***************************
	 * multiplication and division
	 * *************************/
	Op multiplication;
	Op division;

	multiplication.name = " * ";
	multiplication.short_name = "*";
	multiplication.f = multiply;
	multiplication.char_f = charMultiply;
	multiplication.inverse = &division;
	multiplication.left_dist_over[0] = " + ";
	multiplication.left_dist_over[1] = " - ";
	multiplication.left_dist_over[2] = NULL;
	multiplication.right_dist_over[0] = " + ";
	multiplication.right_dist_over[1] = " - ";
	multiplication.right_dist_over[2] = NULL;
	multiplication.char_left_assoc_f = charMultiplyLeftAssoc;
	multiplication.char_right_assoc_f = charMultiplyRightAssoc;
	multiplication.char_left2right_assoc_f = left2rightAssocMultiply;
	multiplication.char_right2left_assoc_f = right2leftAssocMultiply;
	multiplication.char_comm_f = charMultiplyComm;
	multiplication.right_unit = "1";
	multiplication.left_unit = "1";

	division.name = " / ";
	division.short_name = "/";
	division.f = divide;
	division.char_f = charDivide;
	division.inverse = &multiplication;
	division.left_dist_over[0] = NULL;		// a / (b + c) != a / b + a / c
	division.right_dist_over[0] = " + ";
	division.right_dist_over[1] = " - ";
	division.right_dist_over[2] = NULL;
	division.char_left_assoc_f = charDivideLeftAssoc;
	division.char_right_assoc_f = NULL;
	division.char_left2right_assoc_f = NULL;
	division.char_right2left_assoc_f = NULL;
	division.char_comm_f = NULL;
	division.right_unit = "1";
	division.left_unit = "";

	p = addOp(p, addition);
	p = addOp(p, subtraction);
	p = addOp(p, multiplication);
	p = addOp(p, division);
	
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
	q->char_left_assoc_f(w, a, b, c);
	printf("%s\n", w);
	q->char_right_assoc_f(w, a, b, c);
	printf("%s\n", w);
	q->char_left2right_assoc_f(w, "(a + b)", c);
	printf("%s\n", w);
	q->char_right2left_assoc_f(w, a, "(b + c)");
	printf("%s\n", w);

	q = getOp(p, " - ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->char_left_assoc_f(w, a, b, c);
	printf("%s\n", w);

	q = getOp(p, " * ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->char_left_assoc_f(w, a, b, c);
	printf("%s\n", w);
	q->char_right_assoc_f(w, a, b, c);
	printf("%s\n", w);
	q->char_left2right_assoc_f(w, "(a * b)", c);
	printf("%s\n", w);
	q->char_right2left_assoc_f(w, a, "(b * c)");
	printf("%s\n", w);

	q = getOp(p, " / ");
	q->char_f(w, a, b);
	printf("%s\n", w);
	q->char_left_assoc_f(w, a, b, c);
	printf("%s\n", w);
}








#endif	/* _OPERATION_H */
