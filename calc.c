#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef double Num;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op
typedef struct {Str name; Op func;} OpDef;

#define RET(n) { __auto_type temp = (n) ; *pstr = str; return(temp); }
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

Num readExpr(Str*, int);

Op search(Str* pstr, OpDef* ops) {
	Str str = *pstr;
	for (; ops->name ; ops++) {
		int len = strlen(ops->name);
		if (strncmp(str, ops->name, len)==0) {
			str += len;
			RET(ops->func);
		}
	}
	return NULL;
}
Num readValue(Str* pstr, int depth) {
	Str str = *pstr;
	if (str[0]>='0' && str[0]<='9' || str[0]=='.') {
		Str end;
		Num num = strtod(str, &end);
		if (end) {
			str = end;
			RET(num);
		}
	}
	if (str[0]==' ') {
		str++;
		RET(readExpr(&str, depth+1));
	}
	Op op = search(&str, prefix);
	if (op) {
		RET(op(readValue(&str, depth), 0));
	}
}

Num readAfter(Str* pstr, int depth, Num acc) {
	Str str = *pstr;
	if (str[0]==' ') {
		str++;
		if (depth>0)
			RET(acc);
	}
	if (str[0]=='\0')
		RET(acc);
	Op op = search(&str, infix);
	if (op) {
		Num v = readValue(&str, depth);
		RET(readAfter(&str, depth, op(acc, v)));
	}
}

Num readExpr(Str* pstr, int depth) {
	Str str = *pstr;
	Num acc = readValue(&str, depth);
	RET(readAfter(&str, depth, acc));
}

void main(int argc, Str* argv) {
	Str expr = argv[1];
	Num res = readExpr(&expr, 0);
	printf("%f", res);
}
