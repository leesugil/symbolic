#ifndef OPERATION_H
#define OPERATION_H

#include "_operation.h"
#include "constants.h"

EXPR *sadd(EXPR *a, EXPR *b)
{
	/* resume work from here */
}

OP add;
add.name = "addition";
add.df = sadd;
add.isdist = F;
add.isassoc = T;
add.iscomm = T;
add.id = "0";
add.inv = NULL;

OP subt;
subt.name = "subtraction";
subt.df = NULL;
subt.isdist = F;
subt.isassoc = F;
subt.iscomm = F;
subt.id = NULL;
subt.inv = NULL;

OP mult;
mult.name = "multiplicatin";
mult.df = NULL;
mult.isdist = T;
mult.isassoc = T;
mult.iscomm = T;
mult.id = "1";
mult.inv = NULL;

OP div;
div.name = "division";
div.df = NULL;
div.isdist = T;
div.isassoc = F;
div.iscomm = F;
div.id = NULL;
div.inv = NULL;

#endif	/* OPERATION_H */
