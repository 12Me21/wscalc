#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>

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
Num DLround(Num a);
Num DLfloor(Num a);
Num DLmod(Num a, Num b);

// GLOBALS
Num ans = 0;
int haveAns = 0;
jmp_buf env;

#define OPDEF(name, expr) Num op_##name(Num a, Num b) { return (expr); }
#define OPDEFL(name, code) Num op_##name(Num a, Num b) { code }
#define OPDEFS(name, code) Num op_##name(Str* str) { code }

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
OPDEF(mul,a*b);OPDEF(div,a/b);OPDEF(mod,DLmod(a,b));
//OPDEF(pow,pow(a,b));
OPDEF(shr,(IntNum)a>>(IntNum)b);OPDEF(shl,(IntNum)a<<(IntNum)b);
OPDEF(and,(IntNum)a&(IntNum)b);OPDEF(or,(IntNum)a|(IntNum)b);OPDEF(xor,(IntNum)a^(IntNum)b);
OpDef infix[] = {
	{"+",op_add},{"-",op_sub},
	{"*",op_mul},{"/",op_div},{"%",op_mod},
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

// literal prefixes
OPDEFS(hex,
	Num num = 0;
	for (;;) {
		char c = **str;
		if (c>='0' && c<='9') {
			num *= 16;
			num += c-'0';
			(*str)++;
		} else if (c>='a' && c<='f') {
			num *= 16;
			num += c-'a';
			(*str)++;
		} else if (c>='A' && c<='F') {
			num *= 16;
			num += c-'A';
			(*str)++;
		} else
			break;
	}
	return num;
);
OPDEFS(bin,
	Num num = 0;
	for (;;) {
		char c = **str;
		if (c=='0' || c=='1') {
			num *= 2;
			num += c-'0';
			(*str)++;
		} else
			break;
	}
	return num;
);
OpDef literal[] = {
	{"0x",op_hex},{"0b",op_bin},
	{"&h",op_hex},{"&H",op_hex},
	{"&b",op_bin},{"&B",op_bin},
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

Num readValue(Str* str, int depth) {
	Op op = search(str, literal);
	if (op)
		return op(str);
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
	op = search(str, variable);
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
		int err = 0;
		while (1) {
			c = getline(&line, &n, stdin);
			if (c<0) {
				err = err || errno;
				break;
			}
			if (line[c-1]=='\n')
				line[c-1] = '\0';
			err = err || doline(line, 0);
		}
		free(line);
		return err;
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
