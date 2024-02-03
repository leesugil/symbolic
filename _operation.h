#ifndef _OPERATION_H
#define _OPERATION_H

// function signature
typedef double (*BinaryFunctionPointer)(double, double);
typedef void (*BinaryCharFunctionPointer)(char w[], char *x, char *y);
typedef void (*CommBinaryCharFunctionPointer)(char w[], char *x, char *y);
typedef void (*LeftAssocCharFunctionPointer)(char w[], char *a, char *b, char *c);
typedef void (*RightAssocCharFunctionPointer)(char w[], char *a, char *b, char *c);
typedef void (*AssocCharFunctionPointer)(char [], char *, char *);

typdef struct Op Op;

typedef struct {
	/* assume binary */
	char *name;							// like " + "
	char *short_name;					// like "+"
	BinaryFunctionPointer f;			// like add(x, y)
	BinaryCharFunctionPointer char_f;	// like charadd(w, x, y)
	Op *inverse;					// inverse operation
	char *left_dist_over[];				// { " + ", NULL }
	char *right_dist_over[];				// (a + b) * c = a * c + b * c
	unsigned int is_dist : 1;
	LeftAssocCharFunctionPointer char_left_assoc_f;	// (a * b) * c
	RightAssocCharFunctionPointer char_right_assoc_f;	// a * (b * c)
	AssocCharFunctionPointer char_left2right_assoc_f;
	AssocCharFunctionPointer char_right2Left_assoc_f;
	unsigned int is_assoc : 1;
	CommBinaryCharFunctionPointer char_comm_f;	// f(a, b) = f(b, a)
	unsigned int is_comm : 1;
	char *right_unit;
	char *left_unit;
	char *unit;							// unit (identity element) captured in ASCII string
	Op *left;
	Op *right;
} Op;

static Op *opAlloc(void)
{
	return (Op *) malloc(sizeof(Op));
}

Op *addOp(Op *p, Op op)
{
	int cond;

	if (p == NULL) {
		p = opAlloc();
		p->name = strdup(op.name);
		p->short_name = op.short_name;
		p->f = op.f;
		p->char_f = op.char_f;
		p->inverse = op.inverse;
		p->left_dist_over = op.left_dist_over;
		p->right_dist_over = op.right_dist_over;
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
	} else if ((cond = strcmp(name, op.name)) == 0)
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
		printf("\"%s\" = \"%s\"\n", p->name);
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
	else if ((cond = strcmp(name, p->name)) == 0)
		return p;
	else if (cond < 0) {
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


#endif	/* _OPERATION_H */
