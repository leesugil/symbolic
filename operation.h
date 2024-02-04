#ifndef OPERATION_H
#define OPERATION_H

#include <string.h>
#include <math.h>
#include <complex.h>

#include "constants.h"
#include "expression.h"
#include "_operation.h"

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
	addition.char_right2left_assoc_f = left2rightAssocAdd;
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
	division.left_dist_over[0] = NULL;		// a / (b + c) != a / b + a / c
	division.right_dist_over[0] = " + ";
	division.right_dist_over[1] = " - ";
	division.right_dist_over[2] = NULL;
	division.char_left_assoc_f = NULL;
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
}

#endif	/* OPERATION_H */
