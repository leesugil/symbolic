#ifndef OPERATION_H
#define OPERATION_H

#include <string.h>
#include <math.h>
#include <complex.h>

#include "constants.h"
#include "expression.h"
#include "_operation.h"

extern Op *op_tree;

/* run this at the beginning of the main code to load up the operation tree */
Op *loadOps(Op *p);

#endif	/* OPERATION_H */
