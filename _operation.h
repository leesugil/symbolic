#ifndef _OPERATION_H
#define _OPERATION_H

struct operation {
	/* assume binary */
	char *name;
	unsigned int isdist : 1;		/* is distributive over +? */
	unsigned int isassoc : 1;		/* f(f(a, b), c) = f(a, f(b, c)) */
	unsigned int iscomm : 1;		/* f(a, b) = f(b, a) */
	char *id;						/* unit (identity element) captured in ASCII string */
};

typedef struct operation OP;

#endif	/* _OPERATION_H */
