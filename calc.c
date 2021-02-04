#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define when(n) break; case n
#define otherwise break; default
#define err(args...) fprintf(stderr, args)

typedef _Decimal128 Num;
typedef long long IntNum;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op(*) -> Num
typedef struct {Str name; Op func;} OpDef;

int scanDL(Num* out, FILE* stream);
Num strtoDL(char* s, char** end);
void printDL(Num x);

// GLOBALS
Num ans = 0;
int haveAns = 0;
jmp_buf env;

#define OPDEF(name, expr) Num op_##name(Num a, Num b) { return (expr); }
#define OPDEFL(name, code) Num op_##name(Num a, Num b) { code }

// Prefix Operator Definitions
OPDEF(neg,-a);
OPDEF(not,~(IntNum)a);
OpDef prefix[] = {
	{"-",op_neg},
	{"~",op_not},
	{NULL, NULL},
};
// Infix Operator Definitions
OPDEF(add,a+b);OPDEF(sub,a-b);
OPDEF(mul,a*b);OPDEF(div,a/b);//OPDEF(mod,fmod(a,b));
//OPDEF(pow,pow(a,b));
OPDEF(shr,(IntNum)a>>(IntNum)b);OPDEF(shl,(IntNum)a<<(IntNum)b);
OPDEF(and,(IntNum)a&(IntNum)b);OPDEF(or,(IntNum)a|(IntNum)b);OPDEF(xor,(IntNum)a^(IntNum)b);
OpDef infix[] = {
	{"+",op_add},{"-",op_sub},
	{"*",op_mul},{"/",op_div},//{"%",op_mod},
	//{"^",op_pow},
	{">>",op_shr},{"<<",op_shl},
	{"&",op_and},{"|",op_or},{"~",op_xor},
	//	{"->",op_assign},
	{NULL, NULL},
};
// Variable Definitions
OPDEFL(input, {
		if (isatty(0))
			err("input number: ");
		_Decimal128 res;
		if (scanDL(&res, stdin)==1)
			return res;
		longjmp(env, 4);
	});
OPDEFL(ans, {
		if (haveAns)
			return ans;
		return op_input(a, b);
	});
// variables
OpDef variable[] = {
	{"a",op_ans},
	{"i",op_input},
	{NULL, NULL},
};

Op search(Str* str, OpDef* ops) {
	for (; ops->name ; ops++) {
		int len = strlen(ops->name);
		if (strncmp(*str, ops->name, len)==0) {
			*str += len;
			return ops->func;
		}
	}
	return NULL;
}

Num readExpr(Str*, int);

// Read hex number. similar to `strtod`
Num strtoh(Str str, Str* end) {
	int len;
	unsigned int num;
	if (sscanf(str, "%x%n", &num, &len)==1) {
		*end = str+len;
		return num;
	}
	*end = NULL;
	return 0;
}

Num readValue(Str* str, int depth) {
	// hex number
	if ((*str)[0]=='0' && (*str)[1]=='x') {
		Str end;
		Num num = strtoh(*str, &end);
		if (end) {
			*str = end;
			return num;
		}
	}
	// decimal
	if (((*str)[0]>='0' && (*str)[0]<='9') || (*str)[0]=='.') {
		Str end;
		Num num = strtoDL(*str, &end);
		if (end) {
			*str = end;
			return num;
		}
	}
	// Start group
	if ((*str)[0]==' ') { 
		(*str)++;
		return readExpr(str, (depth || 1)+1);
	}
	// Variable
	Op op = search(str, variable);
	if (op)
		return op(0, 0);
	// Prefix Operator
	op = search(str, prefix);
	if (op)
		return op(readValue(str, depth || 1), 0);
	// Implicit value (top level only)
	// "+1" is treated as "a+1", etc.
	// this only works if the expression is not otherwise valid.
	// so, "-1" is not treated as "a-1", etc.
	if (!depth)
		return op_ans(0, 0);
	// Error
	longjmp(env, 1);
}

Num readAfter(Str* str, int depth, Num acc) {
	// End group
	if ((*str)[0]==' ') { 
		(*str)++;
		if (depth>1)
			return acc;
	}
	if ((*str)[0]=='\0')
		return acc;
	// Infix Operator
	Op op = search(str, infix);
	if (op) {
		Num v = readValue(str, depth || 1);
		return readAfter(str, depth || 1, op(acc, v));
	}
	// Error
	longjmp(env, 2);
}

Num readExpr(Str* str, int depth) {
	Num acc = readValue(str, depth);
	return readAfter(str, depth || 1, acc);
}

int doline(Str line, int interactive) {
	if (!line)
		return 3;
	if (interactive)
		add_history(line);
	//IF (line[0]=='\0')
	//	continue;
	Str expr;
	int result = setjmp(env);
	if (!result) {
		expr = line;
		Num res = readExpr(&expr, 0); //can longjump
		ans = res;
		haveAns = 1;
		if (interactive)
			printf("> ");
		printDL(res);
		putchar('\n');
		return 0;
	} else { //returned via longjump
		//if (interactive) {
			switch (result) {
			when(1):				
				err("! Error: expected value (number, a) or prefix operator (");
				OpDef* op;
				for (op=prefix; op->name; op++) {
					if (op!=prefix)
						err(", ");
					err("%s", op->name);
				}
				err(")");
			when(2):
				err("! Error: expected operator (");
				for (op=infix; op->name; op++) {
					if (op!=infix)
						err(", ");
					err("%s", op->name);
				}
				err(")");
			when(4):
				err("! Error: expected number from stdin");
			otherwise:
				err("! Unknown error: %d", result);
			}
			err("\n");
			err("! %.*s⟨here⟩%s\n", (int)(expr-line), line, expr);
		}
		return result;
		//}
}

int main(int argc, Str* argv) {
	if (argc == 2)
		return doline(argv[1], 0);
	if (!isatty(0)) {
		Str line = NULL;
		size_t n = 0;
		ssize_t c = 0;
		c = getline(&line, &n, stdin);
		if (line[c-1]=='\n')
			line[c-1] = '\0';
		return doline(line, 0);
	}
	using_history();
	while (1) {
		Str line = readline("<< ");
		if (!line)
			break;
		doline(line, 1);
		free(line);
	}
	return 0;
}

//todo: allow parentheses. these are stronger than space precedence.
//(....) will always be a group (unless it contains more parentheses) regardless of the amount of whitespace.
