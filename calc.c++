#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef double Num;
typedef const char* Str;
typedef Num (*Op)(Num, Num); //typedef Op
typedef struct {Str name; Op func;} OpDef;

#define OPDEF(name, expr) Num op_##name(Num a, Num b) { return (expr); }

OPDEF(add, a+b);
OPDEF(sub, a-b);
OPDEF(mul, a*b);
OPDEF(div, a/b);
OPDEF(mod, fmod(a,b));
OPDEF(pow, pow(a,b));

OPDEF(neg, -a);

OpDef prefix[] = {
	{"-", op_neg},
	{NULL, NULL},
};

OpDef infix[] = {
	{"+", op_add},
	{"-", op_sub},
	{"*", op_mul},
	{"/", op_div},
	{"%", op_mod},
	{"^", op_pow},
	{NULL, NULL},
};

int match(Str at, Str word) {
	for (; word++,at++ ; *word)
		if (*word!=*at)
			return 0;
	return 1;
}

Num readExpr(Str&, int);

Op search(Str& str, OpDef* ops) {
	for (; ops->name ; ops++) {
		int len = strlen(ops->name);
		if (strncmp(str, ops->name, len)==0) {
			str += len;
			return ops->func;
		}
	}
	return NULL;
}
Num readValue(Str& str, int depth) {
	if (str[0]>='0' && str[0]<='9' || str[0]=='.') {
		Str end;
		Num num = strtod(str, (char**)&end);
		if (end) {
			str = end;
			return num;
		}
	}
	if (str[0]==' ') {
		str++;
		return readExpr(str, depth+1);
	}
	Op op = search(str, prefix);
	if (op)
		return op(readValue(str, depth), 0);
}

Num readAfter(Str& str, int depth, Num acc) {
	if (str[0]==' ') {
		str++;
		if (depth>0)
			return acc;
	}
	if (str[0]=='\0')
		return acc;
	Op op = search(str, infix);
	if (op) {
		Num v = readValue(str, depth);
		return readAfter(str, depth, op(acc, v));
	}
}

Num readExpr(Str& str, int depth) {
	Num acc = readValue(str, depth);
	return readAfter(str, depth, acc);
}

int main(int argc, Str* argv) {
	Str expr = argv[1];
	Num res = readExpr(expr, 0);
	printf("%f", res);
}
