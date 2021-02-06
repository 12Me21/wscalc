#include "../calc.h"


// Examples:
#if 0
// Define an operator function:
DEF_OP(name, a+b+2);
//  is short for ↓
local Num op_name(Num a, Num b) {
	return a+b+2;
}
// (for prefix ops, don't use `b`)

// Define a more complex operator function:
DEF_OP_L(name,
	if (a<b)
		return -1;
	if (a>b)
		return 1;
	return 0;
);
//  is short for ↓
local Num op_name(Num a, Num b) {
	if (a<b)
		return -1;
	if (a>b)
		return 1;
	return 0;
}

// Define a literal prefix function:
DEF_LIT(name,
	if (**str)
		return (Num)*((*str)++);
	return DLnan;
);
//  is short for ↓
local Num lit_name(Str** str) {
	if (**str)
		return (Num)*((*str)++);
	return DLnan;
}

// Adding operators:
ADD_OP(list, symbol, function);
//  is short for ↓
addOp(&list, &(OpDef){symbol, function});
// list - prefix, infix, or literal
// symbol - a string (ex: "++")
// function - (ex: op_add2)

// Init:
// use the AUTORUN macro to run a block of code when the program starts:
AUTORUN {
	addOp(&infix, &(OpDef){"++", op_add2});
}

#endif


DEF_OP(neg, -a);
DEF_OP(not, ~(IntNum)a);
DEF_OP(add, a+b);
DEF_OP(sub, a-b);
DEF_OP(mul, a*b);
DEF_OP(div, a/b);
DEF_OP(mod, DLmod(a,b));

AUTORUN {
	ADD_OP(infix, "+", op_add);
	ADD_OP(infix, "-", op_add);
	ADD_OP(infix, "+", op_add);
	ADD_OP(infix, "-", op_sub);
	ADD_OP(infix, "*", op_mul);
	ADD_OP(infix, "/", op_div);
	ADD_OP(infix, "%", op_mod);

	ADD_OP(prefix, "-", op_neg);
	ADD_OP(prefix, "~", op_not);
}
