#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef double Num;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op
typedef struct {Str name; Op func;} OpDef;

#define RET(n) { __auto_type temp = (n) ; *pstr = str; return(temp); }

Num negate(Num a) {
	return -a;
}
Num subtract(Num a, Num b) {
	return a-b;
}
OpDef prefix[] = {
	{"-", negate},
	{NULL, NULL},
};

OpDef infix[] = {
	{"-", subtract},
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
	printf("readvalue: %s\n", str);
	if (str[0]>='0' && str[0]<='9' || str[0]=='.') {
		Str end;
		Num num = strtod(str, &end);
		printf("strtod: %f\n", num);
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
		printf("afterop: %s\n", str);
		RET(op(readValue(&str, depth)));
	}
	puts("ERROR");
}

Num readAfter(Str* pstr, int depth, Num acc) {
	Str str = *pstr;
	printf("readafter: %s\n", str);
	if (str[0]==' ') {
		str = str+2;
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
	puts("ERROR");
}

Num readExpr(Str* pstr, int depth) {
	Str str = *pstr;
	printf("readexpr: %s\n", str);
	Num acc = readValue(&str, depth);
	return readAfter(&str, depth, acc);
}

void main(int argc, Str* argv) {
	Str expr = argv[1];
	Num res = readExpr(&expr, 0);
	printf("%f", res);
}
