#ifndef OPERATION_H
#define OPERATION_H

#include <string.h>
#include <math.h>
#include <complex.h>

#include "_operation.h"
#include "constants.h"

/* run this at the beginning of the main code to load up the operation tree */
Op *loadOps(Op *p)
{
	p = addOp(addition);
	p = addOp(subtraction);
	p = addOp(multiplication);
	p = addOp(division);
	
	return p;
}

/*
.name = "";
.short_name = "";
.f = NULL;
.char_f = NULL;
.inverse = NULL;
.left_dist_over = { NULL };
.right_dist_over = { NULL };
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

	while (is_outer_blocked(x1))
		remove_outer_block_blk(x1, block_start, block_end);
	while (is_outer_blocked(y1))
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
	if (!is_blocked_properly_blk(dum_line, block_start, block_end, &index)) {
		return;
	}
	while (is_outer_blocked_blk(dum_line, block_start, block_end, &index)) {
		remove_outer_block(dum_line, block_start[index], block_end[index]);
	}

	// parse a and b
	char a[MAXCHAR] = "";
	char b[MAXCHAR] = "";
	char *op_list[] = { op, NULL }
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
	char a[MAXCHAR] = "";
	char b[MAXCHAR] = "";
	char *op_list[] = { op, NULL }
	parseExprLeftBy(a, dum_line, op_list, block_start, block_end);
	parseExprRightBy(b, dum_line, op_list, block_start, block_end);

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
	left2rightAssocCharFunction(char w[], char *left, char *c, " + ")
}
void right2leftAssocAdd(char w[], char *a, char *right)
{
	right2leftAssocCharFunction(char w[], char *a, char *right, " + ")
}

double subtract(double x, double y)
{
	return x - y;
}
void charSubtract(char w[], char *x, char *y)
{
	binaryCharFunction(w, x, y, " - ");
}

Op addition;
Op subtraction;

addition.name = " + ";
addition.short_name = "+";
addition.f = add;
addition.char_f = charAdd;
addition.inverse = &subtraction;
addition.left_dist_over = { NULL };
addition.right_dist_over = { NULL };
addition.char_left_assoc_f = charAddLeftAssoc;
addition.char_right_assoc_f = charAddRightAssoc;
addition.char_left2right_assoc_f = left2rightAssocAdd;
addition.char_right2left_assoc_f = left2rightAssocAdd;
addition.char_comm_f = charAddComm;
addition.right_unit = "0";
addition.left_unit = "0";

subtraction.name = " - ";
subtraction.short_name = "-";
subtraction.f = subtract;
subtraction.char_f = charSubtract;
subtraction.inverse = &addition;
subtraction.left_dist_over = { NULL };
subtraction.right_dist_over = { NULL };
subtraction.char_left_assoc_f = NULL;
subtraction.char_right_assoc_f = NULL;
subtraction.char_left2right_assoc_f = NULL;
subtraction.char_right2left_assoc_f = NULL;
subtraction.char_comm_f = NULL;
subtraction.right_unit = "0";
subtraction.left_unit = "";


/***************************
 * multiplication and division
 * *************************/
double multiply(double x, double y)
{
	return x * y;
}
void charMultiply(char w[], char *x, char *y)
{
	charbinop(w, x, " * ", y);
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

double divide(double x, double y)
{
	return x / y;
}
void charDivide(char w[], char *x, char *y)
{
	charbinop(w, x, " / ", y);
}

Op multiplication;
Op division;

multiplicatiion.name = " * ";
multiplication.short_name = "*";
multiplication.f = multiply;
multiplication.char_f = charMultiply;
multiplication.inverse = &division;
multiplication.left_dist_over = { " + ", " - ", NULL };
multiplication.right_dist_over = { " + ", " - ", NULL };
multiplication.char_left_assoc_f = charMultiplyLeftAssoc;
multiplication.char_right_assoc_f = charMultiplyRightAssoc;
multiplication.char_left2right_assoc_f = NULL;
multiplication.char_right2left_assoc_f = NULL;
multiplication.char_comm_f = charMultiplyComm;
multiplication.right_unit = "1";
multiplication.left_unit = "1";

division.name = " / ";
division.short_name = "/";
division.f = divide;
division.char_f = charDivide;
division.inverse = &multiplication;
division.left_dist_over = { NULL };		// a / (b + c) != a / b + a / c
division.right_dist_over = { " + ", " - ", NULL };
division.char_left_assoc_f = NULL;
division.char_right_assoc_f = NULL;
division.char_left2right_assoc_f = NULL;
division.char_right2left_assoc_f = NULL;
division.char_comm_f = NULL;
division.right_unit = "1";
division.left_unit = "";


#endif	/* OPERATION_H */
