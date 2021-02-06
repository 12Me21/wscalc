#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdint.h>

#include "decimal.h"
#include "m68kd.h"

#define when(n) break; case n
#define otherwise break; default
#define err(args...) fprintf(stderr, args)

typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef long long IntNum;
typedef char* Str;
typedef Num (*Op)(/*any args lol*/); //typedef Op(*) -> Num
typedef struct {Str name; Op func;} OpDef;

// GLOBALS
Num ans = 0;
int haveAns = 0;
jmp_buf env;

#define OPDEF(name, expr...) Num op_##name(Num a, Num b) { return (expr); }
#define OPDEFL(name, code...) Num op_##name(Num a, Num b) { code }
#define OPDEFS(name, expr...) Num op_##name(Str* str) { return expr; }
#define OPDEFSL(name, code...) Num op_##name(Str* str) { code }

Num dis68k(Num a, Num ignore) {
	U64 data = a;
	U64 highestN(U64 data, int bits) {
		return data>>(64-bits);
	}
	while (data && !highestN(data, 16))
		data<<=16;
	//printf("data: [%016lX]\n", data);
	U16 nextWord() {
		U16 ret = highestN(data, 16);
		data<<=16;
		//printf("word: %04X\n", ret);
		return ret;
	}
	U32 nextLong() {
		U32 ret = highestN(data, 32);
		data<<=32;
		//printf("long: %08X\n", ret);
		return ret;
	}
	do {
		Str x = M68KDisasm(nextWord, nextLong);
		printf("%s\n", x);
	} while (data);
	return a;
}

// Prefix Operator Definitions
OPDEF(neg,-a);
OPDEF(not,~(IntNum)a);
OPDEF(bytes,dis68k(a,b));
OpDef prefix[] = {
	{"-",op_neg},
	{"~",op_not},
	{"=",op_bytes},
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

// variables + literal prefixes
OPDEFS(bin, DLread(str, 2));
OPDEFS(oct, DLread(str, 8));
OPDEFS(hex, DLread(str, 16));
OPDEFSL(chr,
	if (**str)
		return (Num)*((*str)++);
	else
		return DLnan;
);
OPDEFS(nan, DLnan);
OPDEFS(inf, DLinf);
OPDEFSL(input,
	if (isatty(0))
		err("input number: ");
	Num res;
	if (scanDL(&res, stdin)==1)
		return res;
	longjmp(env, 4);
);
OPDEFS(ans,	haveAns ? ans : op_input(str));

// todo: sort by length
OpDef literal[] = {
	{"0x",op_hex},{"&h",op_hex},{"&H",op_hex},{"x",op_hex},
	{"0b",op_bin},{"&b",op_bin},{"&B",op_bin},{"b",op_bin},
	{"0o",op_oct},{"&o",op_oct},{"&O",op_oct},{"o",op_oct},
	{"'",op_chr},{"c",op_chr},
	{"NaN",op_nan},{"nan",op_nan},
	{"inf",op_inf},{"infinity",op_inf},{"Inf",op_inf},{"Infinity",op_inf},
	{"a",op_ans},
	{"i",op_ans},
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
	// Start group
	if ((*str)[0]==' ') { 
		(*str)++;
		return readExpr(str, (depth || 1)+1);
	}
	// literal/vars
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
	// Prefix Operator
	op = search(str, prefix);
	if (op)
		return op(readValue(str, depth || 1), 0);
	// Implicit value (top level only)
	// "+1" is treated as "a+1", etc.
	// this only works if the expression is not otherwise valid.
	// so, "-1" is not treated as "a-1", etc.
	if (!depth)
		return op_ans(str);
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
		DLprint(res, 10);
		putchar('\n');
		return 0;
	} else { //returned via longjump
		//if (interactive) {
			switch (result) {
			when(1):				
				err("! Error: expected value (number");
				OpDef* op;
				for (op=literal; op->name; op++) {
					err(", %s", op->name);
				}
				err(") or prefix operator (");
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
	//	loadReadline();
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
